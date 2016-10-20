#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKGL;
using namespace DKGL;

struct DCScreen
{
	PyObject_HEAD	
	DKObject<DKScreen> screen;
	PyObject* window;
	PyObject* frame;
};

class LocalOperation;
struct PendingOperation
{
	LocalOperation* op;
	DKObject<DKRunLoopOperationResult> result;
};
typedef DKArray<PendingOperation> PendingOperationArray;
typedef DKMap<DCScreen*, PendingOperationArray> PendingOperationByScreenMap;
static PendingOperationByScreenMap pendingOperations;
static DKSpinLock operationLock;

class LocalOperation : public DKOperation
{
public:
	DCScreen* screen;
	PyObject* callable;
	PyObject* tuple;
	PyObject* dict;
	LocalOperation(DCScreen* s, PyObject* func, PyObject* args, PyObject* kwargs)
		: screen(s), callable(func), tuple(args), dict(kwargs)
	{
		DKASSERT_DEBUG(screen);
		Py_INCREF(callable);
		Py_INCREF(tuple);
		Py_XINCREF(dict);
	}
	~LocalOperation(void)
	{
		if (Py_IsInitialized())
		{
			PyGILState_STATE st = PyGILState_Ensure();
			Py_CLEAR(callable);
			Py_CLEAR(tuple);
			Py_CLEAR(dict);
			PyGILState_Release(st);
		}
		DKASSERT_DEBUG(screen);
		DKCriticalSection<DKSpinLock> guard(operationLock);
		auto p = pendingOperations.Find(this->screen);
		if (p)
		{
			PendingOperationArray& ops = p->value;
			for (size_t i = 0; i < ops.Count(); ++i)
			{
				PendingOperation& po = ops.Value(i);
				if (po.op == this)
				{
					ops.Remove(i);
					break;
				}
			}
			if (ops.IsEmpty())
			{
				pendingOperations.Remove(this->screen);
			}
		}
	}

	void Perform(void) const override
	{
		if (Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([this](){
				PyObject* tmp = PyObject_Call(callable, tuple, dict);
				Py_XDECREF(tmp);
			});
		}
		else
		{
			DKLog("Warning: Python is not initialized. (Operation ignored)");
		}
	}
};

// PyThreadState that created from PyGILState_Ensure(), will be deleted if count is 0.
// To prevent re-creating PyThreadState repeatedly, pre-create object in OnInitialize().
class DCLocalScreen : public DKScreen
{
	PyGILState_STATE gs;
	PyThreadState* ts = NULL;
public:
	void Initialize(void) override
	{
		this->gs = PyGILState_Ensure();
		this->ts = PyEval_SaveThread();
		DKScreen::Initialize();
	}
	void Finalize(void) override
	{
		DKScreen::Finalize();
		PyEval_RestoreThread(this->ts);
		PyGILState_Release(this->gs);
	}
};

static PyObject* DCScreenNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCScreen* self = (DCScreen*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->screen) DKObject<DKScreen>();
		self->frame = NULL;
		self->window = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCScreenInit(DCScreen *self, PyObject *args, PyObject *kwds)
{
	if (self->screen)
		return 0;

	PyObject* windowObj = NULL;
	PyObject* frameObj = NULL;

	char* kwlist[] = { "window", "frame", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist, &windowObj, &frameObj))
		return -1;

	DKWindow* window = DCWindowToObject(windowObj);
	if (window == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "first argument must be Window object.");
		return -1;
	}
	if (!window->IsValid())
	{
		PyErr_SetString(PyExc_TypeError, "first argument is invalid Window object.");
		return -1;
	}
	DKFrame* frame = DCFrameToObject(frameObj);
	if (frame == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "second argument must be Frame object.");
		return -1;
	}
	
	Py_INCREF(windowObj);
	Py_INCREF(frameObj);
	self->window = windowObj;
	self->frame = frameObj;
	self->screen = DKOBJECT_NEW DCLocalScreen();
	DCObjectSetAddress(self->screen, (PyObject*)self);

	PyThreadState* tst = PyEval_SaveThread();

	bool result = self->screen->Run(window, frame);

	PyEval_RestoreThread(tst);

	if (result)
		return 0;

	Py_DECREF(windowObj);
	Py_DECREF(frameObj);
	DCObjectSetAddress(self->screen, NULL);
	self->screen = NULL;
	self->window = NULL;
	self->frame = NULL;

	PyErr_SetString(PyExc_RuntimeError, "Internal Error!");
	return -1;
}

static int DCScreenClear(DCScreen* self)
{
	if (self->screen)
	{
		Py_CLEAR(self->frame);
		Py_CLEAR(self->window);

		Py_BEGIN_ALLOW_THREADS
		self->screen->Terminate(true);
		Py_END_ALLOW_THREADS

		DCObjectSetAddress(self->screen, NULL);
		self->screen = NULL;

		DKCriticalSection<DKSpinLock> guard(operationLock);
		auto p = pendingOperations.Find(self);
		if (p)
		{
			for (PendingOperation& po : p->value)
			{
				if (po.result)
					po.result->Revoke();
			}
			pendingOperations.Remove(self);
		}
	}
	return 0;
}

static int DCScreenTraverse(DCScreen *self, visitproc visit, void *arg)
{
	Py_VISIT(self->frame);
	Py_VISIT(self->window);
	DKCriticalSection<DKSpinLock> guard(operationLock);
	auto p = pendingOperations.Find(self);
	if (p)
	{
		for (PendingOperation& po : p->value)
		{
			Py_VISIT(po.op->callable);
			Py_VISIT(po.op->tuple);
			Py_VISIT(po.op->dict);
		}
	}		
	return 0;
}

static void DCScreenDealloc(DCScreen* self)
{
	DCScreenClear(self);
	self->screen.~DKObject<DKScreen>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCScreenKeyFrame(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	int deviceId;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;
	
	DKFrame* frame = self->screen->KeyFrame(deviceId);
	if (frame)
	{
		PyObject* obj = DCObjectFromAddress(frame);
		if (obj)
		{
			Py_INCREF(obj);
			return obj;
		}
		DKLog("Warning: DCScreenKeyFrame has a frame. but it is not PyObject.");
	}
	Py_RETURN_NONE;
}

static PyObject* DCScreenSetKeyFrame(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	int deviceId;
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "iO", &deviceId, &obj))
		return NULL;
	DKFrame* frame = NULL;
	if (obj != Py_None)
	{
		frame = DCFrameToObject(obj);
		if (frame == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "second argument must be Frame object or None.");
			return NULL;
		}
	}
	if (!self->screen->SetKeyFrame(deviceId, frame))
	{
		PyErr_SetString(PyExc_ValueError, "frame cannot be KeyFrame.");
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject* DCScreenFocusFrame(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	int deviceId;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;
	
	DKFrame* frame = self->screen->FocusFrame(deviceId);
	if (frame)
	{
		PyObject* obj = DCObjectFromAddress(frame);
		if (obj)
		{
			Py_INCREF(obj);
			return obj;
		}
		DKLog("Warning: DCScreenFocusFrame has a frame. but it is not PyObject.");
	}
	Py_RETURN_NONE;
}

static PyObject* DCScreenSetFocusFrame(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	int deviceId;
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "iO", &deviceId, &obj))
		return NULL;
	DKFrame* frame = NULL;
	if (obj != Py_None)
	{
		frame = DCFrameToObject(obj);
		if (frame == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "second argument must be Frame object or None.");
			return NULL;
		}
	}
	if (!self->screen->SetFocusFrame(deviceId, frame))
	{
		PyErr_SetString(PyExc_ValueError, "frame cannot be FocusFrame.");
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject* DCScreenRemoveKeyFrameForAnyDevices(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	PyObject* obj;
	int notify = 1;
	if (!PyArg_ParseTuple(args, "Op", &obj, &notify))
		return NULL;
	DKFrame* frame = DCFrameToObject(obj);
	if (frame == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Frame object.");
		return NULL;
	}
	self->screen->RemoveKeyFrameForAnyDevices(frame, notify != 0);
	Py_RETURN_NONE;
}

static PyObject* DCScreenRemoveFocusFrameForAnyDevices(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	PyObject* obj;
	int notify = 1;
	if (!PyArg_ParseTuple(args, "Op", &obj, &notify))
		return NULL;
	DKFrame* frame = DCFrameToObject(obj);
	if (frame == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Frame object.");
		return NULL;
	}
	self->screen->RemoveFocusFrameForAnyDevices(frame, notify != 0);
	Py_RETURN_NONE;
}

static PyObject* DCScreenRemoveAllKeyFramesForAnyDevices(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	int notify = 1;
	if (!PyArg_ParseTuple(args, "p", &notify))
		return NULL;
	self->screen->RemoveAllKeyFramesForAnyDevices(notify != 0);
	Py_RETURN_NONE;
}

static PyObject* DCScreenRemoveAllFocusFramesForAnyDevices(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	int notify = 1;
	if (!PyArg_ParseTuple(args, "p", &notify))
		return NULL;
	self->screen->RemoveAllFocusFramesForAnyDevices(notify != 0);
	Py_RETURN_NONE;
}

static PyObject* DCScreenRender(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	int invalidate = 0;
	if (!PyArg_ParseTuple(args, "|p", &invalidate))
		return NULL;

	self->screen->Render(invalidate > 0);
	Py_RETURN_NONE;
}

static PyObject* DCScreenConvertWindowToScreen(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;
	DKPoint* pt = DCPointToObject(obj);
	if (pt)
	{
		DKPoint pt2 = self->screen->WindowToScreen(*pt);
		return DCPointFromObject(&pt2);
	}
	DKSize* sz = DCSizeToObject(obj);
	if (sz)
	{
		DKSize sz2 = self->screen->WindowToScreen(*sz);
		return DCSizeFromObject(&sz2);
	}
	DKRect* rc = DCRectToObject(obj);
	if (rc)
	{
		DKRect rc2 = self->screen->WindowToScreen(*rc);
		return DCRectFromObject(&rc2);
	}
	PyErr_SetString(PyExc_TypeError, "argument must be Point or Siez or Rect object.");
	return NULL;
}

static PyObject* DCScreenConvertScreenToWindow(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;
	DKPoint* pt = DCPointToObject(obj);
	if (pt)
	{
		DKPoint pt2 = self->screen->ScreenToWindow(*pt);
		return DCPointFromObject(&pt2);
	}
	DKSize* sz = DCSizeToObject(obj);
	if (sz)
	{
		DKSize sz2 = self->screen->ScreenToWindow(*sz);
		return DCSizeFromObject(&sz2);
	}
	DKRect* rc = DCRectToObject(obj);
	if (rc)
	{
		DKRect rc2 = self->screen->ScreenToWindow(*rc);
		return DCRectFromObject(&rc2);
	}
	PyErr_SetString(PyExc_TypeError, "argument must be Point or Siez or Rect object.");
	return NULL;
}

static PyObject* DCScreenPostOperation(DCScreen* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->screen, NULL);

	PyObject* callable = NULL;
	PyObject* tuple = NULL;
	PyObject* dict = NULL;
	double delay = 0.0;
	int waitUntilDone = 0;

	char* kwlist[] = { "callable", "args", "kwargs", "delay", "waitUntilDone", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|Odp", kwlist,
		&callable, &tuple, &dict, &delay, &waitUntilDone))
		return NULL;

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "first argument must be callable");
		return NULL;
	}
	if (!PyTuple_Check(tuple)) {
		PyErr_SetString(PyExc_TypeError, "second argument must be a tuple");
		return NULL;
	}
	if (dict && !PyDict_Check(dict)) {
		PyErr_SetString(PyExc_TypeError, "optional third argument must be a dictionary");
		return NULL;
	}

	if (self->screen->IsRunning())
	{
		if (waitUntilDone && self->screen->IsWrokingThread())
		{
			// call immediately.
			DCObjectCallPyCallable([=](){
				PyObject* tmp = PyObject_Call(callable, tuple, dict);
				Py_XDECREF(tmp);
			});
			Py_RETURN_NONE;
		}
		else
		{
			DKObject<LocalOperation> op = DKOBJECT_NEW LocalOperation(self, callable, tuple, dict);
			DKObject<DKRunLoopOperationResult> p = self->screen->PostOperation(op, delay);
			if (p)
			{
				if (waitUntilDone)
				{
					bool result = false;
					Py_BEGIN_ALLOW_THREADS
						result = p->Result(); // wait until done.
					Py_END_ALLOW_THREADS

					if (!result)		// error?
					{
						DKLog("Warning: DCScreenPostOperation failed!\n");
					}
					Py_RETURN_NONE;
				}
				PendingOperation po = { op, p };
				DKCriticalSection<DKSpinLock> guard(operationLock);
				pendingOperations.Value(self).Add(po);
				return DCRunLoopOperationResultFromObject(p);
			}
			PyErr_SetString(PyExc_RuntimeError, "PostOperation failed.");
			return NULL;
		}
	}
	PyErr_SetString(PyExc_RuntimeError, "Runloop is not running");
	return NULL;
}

static PyObject* DCScreenScheduleOperation(DCScreen* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->screen, NULL);

	PyObject* callable = NULL;
	PyObject* tuple = NULL;
	PyObject* dict = NULL;
	double interval = 0.0;

	char* kwlist[] = { "callable", "args", "kwargs", "interval", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|Od", kwlist,
		&callable, &tuple, &dict, &interval))
		return NULL;

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "first argument must be callable");
		return NULL;
	}
	if (!PyTuple_Check(tuple)) {
		PyErr_SetString(PyExc_TypeError, "second argument must be a tuple");
		return NULL;
	}
	if (dict && !PyDict_Check(dict)) {
		PyErr_SetString(PyExc_TypeError, "optional third argument must be a dictionary");
		return NULL;
	}

	if (self->screen->IsRunning())
	{
		DKObject<LocalOperation> op = DKOBJECT_NEW LocalOperation(self, callable, tuple, dict);
		DKObject<DKRunLoopTimer> runloopTimer = DKRunLoopTimer::Create(op, interval, self->screen);
		if (runloopTimer)
		{
			PendingOperation po = { op, NULL };
			DKCriticalSection<DKSpinLock> guard(operationLock);
			pendingOperations.Value(self).Add(po);

			return DCRunLoopTimerFromObject(runloopTimer);
		}
		PyErr_SetString(PyExc_RuntimeError, "RunLoopTimer creation failed.");
		return NULL;
	}
	PyErr_SetString(PyExc_RuntimeError, "Runloop is not running");
	return NULL;
}

static PyObject* DCScreenTerminate(DCScreen* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->screen, NULL);

	int wait = 0;

	if (!PyArg_ParseTuple(args, "|p", &wait))
		return NULL;

	if (wait)
	{
		Py_BEGIN_ALLOW_THREADS
			self->screen->Terminate(true);
		Py_END_ALLOW_THREADS
	}
	else
		self->screen->Terminate(false);
	Py_RETURN_NONE;
}

static PyObject* DCScreenIsWorkingThread(DCScreen* self, void*)
{
	DCOBJECT_VALIDATE(self->screen, NULL);

	if (self->screen->IsWrokingThread())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "keyFrame", (PyCFunction)&DCScreenKeyFrame, METH_VARARGS },
	{ "setKeyFrame", (PyCFunction)&DCScreenSetKeyFrame, METH_VARARGS },
	{ "focusFrame", (PyCFunction)&DCScreenFocusFrame, METH_VARARGS },
	{ "setFocusFrame", (PyCFunction)&DCScreenSetFocusFrame, METH_VARARGS },
	{ "removeKeyFrameForAnyDevices", (PyCFunction)&DCScreenRemoveKeyFrameForAnyDevices, METH_VARARGS },
	{ "removeFocusFrameForAnyDevices", (PyCFunction)&DCScreenRemoveFocusFrameForAnyDevices, METH_VARARGS },
	{ "removeAllKeyFramesForAnyDevices", (PyCFunction)&DCScreenRemoveAllKeyFramesForAnyDevices, METH_VARARGS },
	{ "removeAllFocusFramesForAnyDevices", (PyCFunction)&DCScreenRemoveAllFocusFramesForAnyDevices, METH_VARARGS },
	{ "render", (PyCFunction)&DCScreenRender, METH_VARARGS },
	{ "convertWindowToScreen", (PyCFunction)&DCScreenConvertWindowToScreen, METH_VARARGS },
	{ "convertScreenToWindow", (PyCFunction)&DCScreenConvertScreenToWindow, METH_VARARGS },
	{ "postOperation", (PyCFunction)&DCScreenPostOperation, METH_VARARGS | METH_KEYWORDS },
	{ "scheduleOperation", (PyCFunction)&DCScreenScheduleOperation, METH_VARARGS | METH_KEYWORDS },
	{ "terminate", (PyCFunction)&DCScreenTerminate, METH_VARARGS },
	{ "isWorkingThread", (PyCFunction)&DCScreenIsWorkingThread, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCScreenResolution(DCScreen* self, void*)
{
	DCOBJECT_VALIDATE(self->screen, NULL);

	DKSize s = self->screen->ScreenResolution();
	return Py_BuildValue("ff", s.width, s.height);
}

static PyObject* DCScreenWindow(DCScreen* self, void*)
{
	if (self->window)
	{
		Py_INCREF(self->window);
		return self->window;
	}
	Py_RETURN_NONE;
}

static PyObject* DCScreenFrame(DCScreen* self, void*)
{
	if (self->frame)
	{
		Py_INCREF(self->frame);
		return self->frame;
	}
	Py_RETURN_NONE;
}

static PyObject* DCScreenRunning(DCScreen* self, void*)
{
	DCOBJECT_VALIDATE(self->screen, NULL);

	if (self->screen->IsRunning())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCScreenSuspended(DCScreen* self, void*)
{
	DCOBJECT_VALIDATE(self->screen, NULL);

	if (self->screen->IsSuspended()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static int DCScreenSetSuspended(DCScreen* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->screen, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int b = PyObject_IsTrue(value);
	if (b < 0)
	{
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}
	if (b)
	{
		self->screen->Suspend();
		DKLog("Screen(%p) suspended.\n", (void*)self->screen);
	}
	else
	{
		self->screen->Resume();
		DKLog("Screen(%p) resumed.\n", (void*)self->screen);
	}
	return 0;
}

static PyObject* DCScreenActiveFrameLatency(DCScreen* self, void*)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	return PyFloat_FromDouble(self->screen->ActiveFrameLatency());
}

static int DCScreenSetActiveFrameLatency(DCScreen* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->screen, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double d = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be float number.");
		return -1;
	}

	self->screen->SetActiveFrameLatency(d);
	return 0;
}

static PyObject* DCScreenInactiveFrameLatency(DCScreen* self, void*)
{
	DCOBJECT_VALIDATE(self->screen, NULL);
	return PyFloat_FromDouble(self->screen->InactiveFrameLatency());
}

static int DCScreenSetInactiveFrameLatency(DCScreen* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->screen, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double d = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be float number.");
		return -1;
	}

	self->screen->SetInactiveFrameLatency(d);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "resolution", (getter)&DCScreenResolution, 0, 0, 0 },
	{ "window", (getter)&DCScreenWindow, 0, "window", 0 },
	{ "frame", (getter)&DCScreenFrame, 0, "root frame", 0 },
	{ "running", (getter)&DCScreenRunning, 0, 0, 0 },
	{ "suspended", (getter)&DCScreenSuspended, (setter)&DCScreenSetSuspended, 0, 0 },
	{ "activeFrameLatency", (getter)&DCScreenActiveFrameLatency, (setter)&DCScreenSetActiveFrameLatency, 0, 0 },
	{ "inactiveFrameLatency", (getter)&DCScreenInactiveFrameLatency, (setter)&DCScreenSetInactiveFrameLatency, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Screen",			/* tp_name */
	sizeof(DCScreen),					/* tp_basicsize */
	0,									/* tp_itemsize */
	(destructor)&DCScreenDealloc,		/* tp_dealloc */
	0,									/* tp_print */
	0,									/* tp_getattr */
	0,									/* tp_setattr */
	0,									/* tp_reserved */
	0,									/* tp_repr */
	0,									/* tp_as_number */
	0,									/* tp_as_sequence */
	0,									/* tp_as_mapping */
	0,									/* tp_hash  */
	0,									/* tp_call */
	0,									/* tp_str */
	0,									/* tp_getattro */
	0,									/* tp_setattro */
	0,									/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,					/* tp_flags */
	0,									/* tp_doc */
	(traverseproc)&DCScreenTraverse,	/* tp_traverse */
	(inquiry)&DCScreenClear,			/* tp_clear */
	0,									/* tp_richcompare */
	0,									/* tp_weaklistoffset */
	0,									/* tp_iter */
	0,									/* tp_iternext */
	methods,							/* tp_methods */
	0,									/* tp_members */
	getsets,							/* tp_getset */
	0,									/* tp_base */
	0,									/* tp_dict */
	0,									/* tp_descr_get */
	0,									/* tp_descr_set */
	0,									/* tp_dictoffset */
	(initproc)&DCScreenInit,			/* tp_init */
	0,									/* tp_alloc */
	&DCScreenNew,						/* tp_new */
};

PyTypeObject* DCScreenTypeObject(void)
{
	return &objectType;
}

PyObject* DCScreenFromObject(DKScreen* screen)
{
	if (screen)
	{
		DCScreen* self = (DCScreen*)DCObjectFromAddress(screen);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCScreen*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->screen = screen;
				DCObjectSetAddress(self->screen, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKScreen* DCScreenToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCScreen*)obj)->screen;
	}
	return NULL;
}
