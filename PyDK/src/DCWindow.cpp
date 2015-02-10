#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

struct DCWindow
{
	PyObject_HEAD
	DKObject<DKWindow> window;
};

static PyObject* DCWindowNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCWindow* self = (DCWindow*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->window) DKObject<DKWindow>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCWindowInit(DCWindow *self, PyObject *args, PyObject *kwds)
{
	if (self->window)
		return 0;

	char* title = "";
	DKSize size;
	DKPoint origin = DKWindow::undefinedOrigin;
	int style = DKWindow::StyleGeneralWindow;

	char* kwlist[] = { "title", "size", "origin", "style", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO&|O&i", kwlist,
									 &title,
									 &DCSizeConverter, &size,
									 &DCPointConverter, &origin,
									 &style))
	{
		return -1;
	}

	Py_BEGIN_ALLOW_THREADS

	DKWindow::WindowCallback callback;
	callback.contentMinSize = DKFunction([self](DKWindow*)->DKSize {
		DKASSERT_DEBUG(Py_IsInitialized());
		DKASSERT_DEBUG(self);

		DKSize s(0,0);
		DCObjectCallPyCallableGIL([&]() {
			PyObject* ret = PyObject_CallMethod((PyObject*)self, "getContentMinSize", NULL);
			if (!PyErr_Occurred() && ret)
			{
				if (!DCSizeConverter(ret, &s))
					PyErr_Clear();
			}
			Py_XDECREF(ret);
		});
		return s;
	});
	callback.contentMaxSize = DKFunction([self](DKWindow*)->DKSize {
		DKASSERT_DEBUG(Py_IsInitialized());
		DKASSERT_DEBUG(self);

		DKSize s(0, 0);
		DCObjectCallPyCallableGIL([&]() {
			PyObject* ret = PyObject_CallMethod((PyObject*)self, "getContentMaxSize", NULL);
			if (!PyErr_Occurred() && ret)
			{
				if (!DCSizeConverter(ret, &s))
					PyErr_Clear();
			}
			Py_XDECREF(ret);
		});
		return s;
	});
	callback.filesDropped = DKFunction([self](DKWindow*, const DKPoint& pt, const DKWindow::WindowCallback::StringArray& files) {
		DKASSERT_DEBUG(Py_IsInitialized());
		DKASSERT_DEBUG(self);

		DCObjectCallPyCallableGIL([&]() {

			DKPoint dropPos = pt;
			PyObject* tuple = PyTuple_New(files.Count());
			for (Py_ssize_t i = 0; i < files.Count(); ++i)
			{
				PyTuple_SET_ITEM(tuple, i, PyUnicode_FromWideChar(files.Value(i), -1));
			}

			PyObject* ret = PyObject_CallMethod((PyObject*)self, "onFilesDropped", "NN",
												DCPointFromObject(&dropPos), tuple);

			Py_XDECREF(ret);
		});
	});
	callback.closeRequest = DKFunction([self](DKWindow*)->bool {
		DKASSERT_DEBUG(Py_IsInitialized());
		DKASSERT_DEBUG(self);

		bool destroy = true;
		DCObjectCallPyCallableGIL([&]() {
			PyObject* ret = PyObject_CallMethod((PyObject*)self, "shouldClose", NULL);
			if (ret)
			{
				int result = PyObject_IsTrue(ret);
				destroy = result > 0;
			}
			Py_XDECREF(ret);
		});
		return destroy;
	});

	self->window = DKWindow::Create(title, size, origin, style, callback);

	Py_END_ALLOW_THREADS

	if (self->window == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "window creation failed.");
		return -1;
	}
	
	auto windowEvent = [self](DKWindow::EventWindow e, DKSize s, DKPoint o)
	{
		DKASSERT_DEBUG(Py_IsInitialized());
		DKASSERT_DEBUG(self);
		
		DCObjectCallPyCallableGIL([&]() {
			PyObject* ret = PyObject_CallMethod((PyObject*)self,
												"onWindowEvent", "iNN",
												(int)e,
												DCSizeFromObject(&s),
												DCPointFromObject(&o));
			Py_XDECREF(ret);
		});
	};
	auto keyboardEvent = [self](DKWindow::EventKeyboard e, int dev, DKVirtualKey vk, DKString str)
	{
		DKASSERT_DEBUG(Py_IsInitialized());
		DKASSERT_DEBUG(self);
		
		DCObjectCallPyCallableGIL([&]() {
			PyObject* ret = PyObject_CallMethod((PyObject*)self,
												"onKeyboardEvent", "iiiN",
												(int)e, dev, (int)vk, PyUnicode_FromWideChar(str, -1));
			Py_XDECREF(ret);
		});
	};
	auto mouseEvent = [self](DKWindow::EventMouse e, int dev, int btn, DKPoint pt, DKVector2 delta)
	{
		DKASSERT_DEBUG(Py_IsInitialized());
		DKASSERT_DEBUG(self);
		
		DCObjectCallPyCallableGIL([&]() {
			PyObject* ret = PyObject_CallMethod((PyObject*)self,
												"onMouseEvent", "iiiNN",
												(int)e, dev, btn, DCPointFromObject(&pt), DCVector2FromObject(&delta));
			Py_XDECREF(ret);
		});
	};
	
	self->window->AddObserver(self,
							  DKFunction(windowEvent),
							  DKFunction(keyboardEvent),
							  DKFunction(mouseEvent),
							  NULL /* runloop */
							  );
	
	DCObjectSetAddress(self->window, (PyObject*)self);	
	return 0;
}

static void DCWindowDealloc(DCWindow* self)
{
	if (self->window)
	{
		self->window->RemoveObserver(self);
		DCObjectSetAddress(self->window, NULL);
		self->window = NULL;
	}

	self->window.~DKObject<DKWindow>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCWindowActivate(DCWindow* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->window, NULL);
	self->window->Activate();
	Py_RETURN_NONE;
}

static PyObject* DCWindowMinimize(DCWindow* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->window, NULL);
	self->window->Minimize();
	Py_RETURN_NONE;
}

static PyObject* DCWindowResize(DCWindow* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->window, NULL);
	DKSize size;
	DKPoint pt;
	if (!PyArg_ParseTuple(args, "O&O&", &DCSizeConverter, &size, &DCPointConverter, &pt))
	{
		return NULL;
	}

	self->window->Resize(size, pt);
	Py_RETURN_NONE;
}

static PyObject* DCWindowClose(DCWindow* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->window, NULL);
	Py_BEGIN_ALLOW_THREADS
	self->window->Close();
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject* DCWindowIsTextInputEnabled(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	char* kwlist[] = { "deviceId", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &deviceId))
		return NULL;

	return PyBool_FromLong( self->window->IsTextInputEnabled(deviceId) );
}

static PyObject* DCWindowSetTextInputEnabled(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	int enabled;
	char* kwlist[] = { "deviceId", "enabled", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ip", kwlist, &deviceId, &enabled))
		return NULL;

	self->window->SetTextInputEnabled(deviceId, enabled != 0);
	Py_RETURN_NONE;
}

static PyObject* DCWindowIsKeyDown(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	int vkey;
	char* kwlist[] = { "deviceId", "key", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &deviceId, &vkey))
		return NULL;

	if (vkey < 0 || vkey > DKVK_MAXVALUE)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is invalid.");
		return NULL;
	}

	if (self->window->KeyState(deviceId, (DKVirtualKey)vkey))
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCWindowShowMouse(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	int show;
	char* kwlist[] = { "deviceId", "show", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ip", kwlist, &deviceId, &show))
		return NULL;

	self->window->ShowMouse(deviceId, show > 0);
	Py_RETURN_NONE;
}

static PyObject* DCWindowHoldMouse(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	int hold;
	char* kwlist[] = { "deviceId", "hold", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ip", kwlist, &deviceId, &hold))
		return NULL;

	self->window->HoldMouse(deviceId, hold > 0);
	Py_RETURN_NONE;
}

static PyObject* DCWindowIsMouseVisible(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	char* kwlist[] = { "deviceId", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &deviceId))
		return NULL;

	return PyBool_FromLong(self->window->IsMouseVisible(deviceId));
}

static PyObject* DCWindowIsMouseHeld(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	char* kwlist[] = { "deviceId", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &deviceId))
		return NULL;

	return PyBool_FromLong(self->window->IsMouseHeld(deviceId));
}

static PyObject* DCWindowMousePosition(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	char* kwlist[] = { "deviceId", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &deviceId))
		return NULL;

	DKPoint pt = self->window->MousePosition(deviceId);
	return DCPointFromObject(&pt);
}

static PyObject* DCWindowSetMousePosition(DCWindow* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	int deviceId;
	DKPoint pt;
	char* kwlist[] = { "deviceId", "position", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "iO&", kwlist, &deviceId, &DCPointConverter, &pt))
		return NULL;

	self->window->SetMousePosition(deviceId, pt);
	Py_RETURN_NONE;
}

static PyObject* DCWindowGetContentMinSize(DCWindow* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->window, NULL);
	DKSize s(0,0);
	return DCSizeFromObject(&s);
}

static PyObject* DCWindowGetContentMaxSize(DCWindow* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->window, NULL);
	DKSize s(-1, -1);
	return DCSizeFromObject(&s);
}

static PyMethodDef methods[] = {
	{ "activate", (PyCFunction)&DCWindowActivate, METH_NOARGS },
	{ "minimize", (PyCFunction)&DCWindowMinimize, METH_NOARGS },
	{ "resize", (PyCFunction)&DCWindowResize, METH_VARARGS },
	{ "close", (PyCFunction)&DCWindowClose, METH_NOARGS },

	{ "isTextInputEnabled", (PyCFunction)&DCWindowIsTextInputEnabled, METH_VARARGS | METH_KEYWORDS },
	{ "setTextInputEnabled", (PyCFunction)&DCWindowSetTextInputEnabled, METH_VARARGS | METH_KEYWORDS },
	{ "isKeyDown", (PyCFunction)&DCWindowIsKeyDown, METH_VARARGS | METH_KEYWORDS },

	{ "showMouse", (PyCFunction)&DCWindowShowMouse, METH_VARARGS | METH_KEYWORDS },
	{ "holdMouse", (PyCFunction)&DCWindowHoldMouse, METH_VARARGS | METH_KEYWORDS },
	{ "isMouseVisible", (PyCFunction)&DCWindowIsMouseVisible, METH_VARARGS | METH_KEYWORDS },
	{ "isMouseHeld", (PyCFunction)&DCWindowIsMouseHeld, METH_VARARGS | METH_KEYWORDS },
	{ "mousePosition", (PyCFunction)&DCWindowMousePosition, METH_VARARGS | METH_KEYWORDS },
	{ "setMousePosition", (PyCFunction)&DCWindowSetMousePosition, METH_VARARGS | METH_KEYWORDS },

	{ "onWindowEvent", &DCObjectMethodNone, METH_VARARGS },
	{ "onKeyboardEvent", &DCObjectMethodNone, METH_VARARGS },
	{ "onMouseEvent", &DCObjectMethodNone, METH_VARARGS },
	{ "onFilesDropped", &DCObjectMethodNone, METH_VARARGS },

	{ "getContentMinSize", (PyCFunction)&DCWindowGetContentMinSize, METH_NOARGS },
	{ "getContentMaxSize", (PyCFunction)&DCWindowGetContentMaxSize, METH_NOARGS },
	{ "shouldClose", &DCObjectMethodTrue, METH_NOARGS },

	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCWindowHidden(DCWindow* self, void*)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	bool visible = self->window->IsVisible();
	if (visible)
		Py_RETURN_FALSE;
	Py_RETURN_TRUE;
}

static int DCWindowSetHidden(DCWindow* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->window, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (self->window == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid Object");
		return -1;
	}

	int r = PyObject_IsTrue(value);
	if (r >= 0)
	{
		bool hidden = r > 0;
		if (hidden)
			self->window->Hide();
		else
			self->window->Show();
		return 0;
	}
	PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
	return -1;
}

static PyObject* DCWindowOrigin(DCWindow* self, void*)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	DKPoint p = self->window->Origin();
	return Py_BuildValue("ff", p.x, p.y);
}

static int DCWindowSetOrigin(DCWindow* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->window, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKPoint pt(0,0);
	if (!DCPointConverter(value, &pt))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be two floats or Point object.");
		return -1;
	}
	self->window->SetOrigin(pt);
	return 0;
}

static PyObject* DCWindowContentSize(DCWindow* self, void*)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	DKSize s = self->window->ContentSize();
	return Py_BuildValue("ff", s.width, s.height);
}

static int DCWindowSetContentSize(DCWindow* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->window, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKSize size(0, 0);
	if (!DCSizeConverter(value, &size))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be two floats or Size object");
		return -1;
	}
	if (size.width < 1 || size.height < 1)
	{
		PyErr_SetString(PyExc_ValueError, "attribute must be greater than zero.");
		return -1;
	}

	self->window->Resize(size);
	return 0;
}

static PyObject* DCWindowContentScaleFactor(DCWindow* self, void*)
{
	DCOBJECT_VALIDATE(self->window, NULL);

	return PyFloat_FromDouble(self->window->ContentScaleFactor());
}

static PyObject* DCWindowTitle(DCWindow* self, void*)
{
	DCOBJECT_VALIDATE(self->window, NULL);
	
	DKString title = self->window->Title();
	return PyUnicode_FromWideChar((const wchar_t*)title, -1);
}

static int DCWindowSetTitle(DCWindow* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->window, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (!PyUnicode_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "attribute value must be a string");
		return -1;
	}

	char* title = PyUnicode_AsUTF8(value);
	self->window->SetTitle(DKString(title));
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "hidden", (getter)&DCWindowHidden, (setter)&DCWindowSetHidden, 0, NULL },
	{ "origin", (getter)&DCWindowOrigin, (setter)&DCWindowSetOrigin, 0, NULL },
	{ "contentSize", (getter)&DCWindowContentSize, (setter)&DCWindowSetContentSize, 0, NULL },
	{ "contentScaleFactor", (getter)&DCWindowContentScaleFactor, (setter)NULL, 0, NULL },
	{ "title", (getter)&DCWindowTitle, (setter)&DCWindowSetTitle, 0, NULL },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Window",					/* tp_name */
	sizeof(DCWindow),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCWindowDealloc,				/* tp_dealloc */
	0,											/* tp_print */
	0,											/* tp_getattr */
	0,											/* tp_setattr */
	0,											/* tp_reserved */
	0,											/* tp_repr */
	0,											/* tp_as_number */
	0,											/* tp_as_sequence */
	0,											/* tp_as_mapping */
	0,											/* tp_hash  */
	0,											/* tp_call */
	0,											/* tp_str */
	0,											/* tp_getattro */
	0,											/* tp_setattro */
	0,											/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
	0,											/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	getsets,									/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCWindowInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCWindowNew,								/* tp_new */
};

PyTypeObject* DCWindowTypeObject(void)
{
	return &objectType;
}

PyObject* DCWindowFromObject(DKWindow* window)
{
	if (window)
	{
		DCWindow* self = (DCWindow*)DCObjectFromAddress(window);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCWindow*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->window = window;
				DCObjectSetAddress(self->window, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKWindow* DCWindowToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCWindow*)obj)->window;
	}
	return NULL;
}
