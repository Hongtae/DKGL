#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKGL;
using namespace DKGL;

struct DCRunLoopTimer
{
	PyObject_HEAD
	DKObject<DKRunLoopTimer> runLoopTimer;
};

static PyObject* DCRunLoopTimerNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCRunLoopTimer* self = (DCRunLoopTimer*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->runLoopTimer) DKObject<DKRunLoopTimer>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCRunLoopTimerInit(DCRunLoopTimer *self, PyObject *args, PyObject *kwds)
{
	return 0;
}

static void DCRunLoopTimerDealloc(DCRunLoopTimer* self)
{
	self->runLoopTimer = NULL;
	self->runLoopTimer.~DKObject<DKRunLoopTimer>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCRunLoopTimerInvalidate(DCRunLoopTimer* self, PyObject*)
{
	if (self->runLoopTimer)
		self->runLoopTimer->Invalidate();
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "invalidate", (PyCFunction)&DCRunLoopTimerInvalidate, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCRunLoopTimerRunning(DCRunLoopTimer* self, void*)
{
	if (self->runLoopTimer && self->runLoopTimer->IsRunning())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCRunLoopTimerCount(DCRunLoopTimer* self, void*)
{
	size_t count = 0;
	if (self->runLoopTimer)
		count = self->runLoopTimer->Count();
	return PyLong_FromSize_t(count);
}

static PyObject* DCRunLoopTimerInterval(DCRunLoopTimer* self, void*)
{
	double d = 0.0;
	if (self->runLoopTimer)
		d = self->runLoopTimer->Interval();
	return PyFloat_FromDouble(d);
}

static PyGetSetDef getsets[] = {
	{ "running", (getter)&DCRunLoopTimerRunning, 0, 0, 0 },
	{ "count", (getter)&DCRunLoopTimerCount, 0, 0, 0 },
	{ "interval", (getter)&DCRunLoopTimerInterval, 0, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".RunLoopTimer",			/* tp_name */
	sizeof(DCRunLoopTimer),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCRunLoopTimerDealloc,			/* tp_dealloc */
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
	(initproc)&DCRunLoopTimerInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCRunLoopTimerNew,							/* tp_new */
};

PyTypeObject* DCRunLoopTimerTypeObject(void)
{
	return &objectType;
}

PyObject* DCRunLoopTimerFromObject(DKRunLoopTimer* runLoopTimer)
{
	if (runLoopTimer)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();

		DCRunLoopTimer* self = (DCRunLoopTimer*)DCObjectCreateDefaultClass(&objectType, args, kwds);
		if (self)
		{
			self->runLoopTimer = runLoopTimer;
			Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
		}
		Py_XDECREF(args);
		Py_XDECREF(kwds);
		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKRunLoopTimer* DCRunLoopTimerToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCRunLoopTimer*)obj)->runLoopTimer;
	}
	return NULL;
}
