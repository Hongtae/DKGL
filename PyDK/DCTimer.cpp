#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKGL;
using namespace DKGL;

struct DCTimer
{
	PyObject_HEAD
	DKTimer timer;
};

static PyObject* DCTimerNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCTimer* self = (DCTimer*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->timer) DKTimer();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCTimerInit(DCTimer *self, PyObject *args, PyObject *kwds)
{
	return 0;
}

static void DCTimerDealloc(DCTimer* self)
{
	self->timer.~DKTimer();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCTimerReset(DCTimer* self, PyObject*)
{
	return PyFloat_FromDouble(self->timer.Reset());
}

static PyObject* DCTimerElapsed(DCTimer* self, PyObject*)
{
	return PyFloat_FromDouble(self->timer.Elapsed());
}

static PyObject* DCTimerTick(PyObject*, PyObject*)
{
	return PyLong_FromLongLong(DKTimer::SystemTick());
}

static PyObject* DCTimerFrequency(PyObject*, PyObject*)
{
	return PyLong_FromLongLong(DKTimer::SystemTickFrequency());
}

static PyMethodDef methods[] = {
	{ "reset", (PyCFunction)&DCTimerReset, METH_NOARGS },
	{ "elapsed", (PyCFunction)&DCTimerElapsed, METH_NOARGS },
	{ "tick", (PyCFunction)&DCTimerTick, METH_NOARGS | METH_STATIC },
	{ "frequency", (PyCFunction)&DCTimerFrequency, METH_NOARGS | METH_STATIC },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Timer",					/* tp_name */
	sizeof(DCTimer),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCTimerDealloc,				/* tp_dealloc */
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
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCTimerInit,						/* tp_init */
	0,											/* tp_alloc */
	&DCTimerNew,								/* tp_new */
};

PyTypeObject* DCTimerTypeObject(void)
{
	return &objectType;
}

PyObject* DCTimerFromObject(DKTimer* timer)
{
	PyObject* args = PyTuple_New(0);
	PyObject* kwds = PyDict_New();

	DCTimer* self = (DCTimer*)DCObjectCreateDefaultClass(&objectType, args, kwds);
	if (self)
	{
		self->timer = *timer;
		Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
	}

	Py_XDECREF(args);
	Py_XDECREF(kwds);

	return (PyObject*)self;
}

DKTimer* DCTimerToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCTimer*)obj)->timer;
	}
	return NULL;
}
