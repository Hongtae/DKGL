#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;


struct DCRunLoopOperationResult
{
	PyObject_HEAD
	DKObject<DKRunLoopOperationResult> result;
};

static PyObject* DCRunLoopOperationResultNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCRunLoopOperationResult* self = (DCRunLoopOperationResult*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->result) DKObject<DKRunLoopOperationResult>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCRunLoopOperationResultInit(DCRunLoopOperationResult *self, PyObject *args, PyObject *kwds)
{
	return 0;
}

static void DCRunLoopOperationResultDealloc(DCRunLoopOperationResult* self)
{
	self->result = NULL;
	self->result.~DKObject<DKRunLoopOperationResult>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

template <bool (DKRunLoopOperationResult::*fn)(void) const>
static PyObject* CallBoolFunc(DCRunLoopOperationResult* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->result, NULL);
	return PyBool_FromLong((self->result->*fn)());
}

static PyMethodDef methods[] =
{
	{ "result", (PyCFunction)&CallBoolFunc<&DKRunLoopOperationResult::Result>, METH_NOARGS },
	{ "revoke", (PyCFunction)&CallBoolFunc<&DKRunLoopOperationResult::Revoke>, METH_NOARGS },
	{ "isDone", (PyCFunction)&CallBoolFunc<&DKRunLoopOperationResult::IsDone>, METH_NOARGS },
	{ "isRevoked", (PyCFunction)&CallBoolFunc<&DKRunLoopOperationResult::IsRevoked>, METH_NOARGS },
	{ "isPending", (PyCFunction)&CallBoolFunc<&DKRunLoopOperationResult::IsPending>, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".RunLoopOperationResult",			/* tp_name */
	sizeof(DCRunLoopOperationResult),					/* tp_basicsize */
	0,													/* tp_itemsize */
	(destructor)&DCRunLoopOperationResultDealloc,		/* tp_dealloc */
	0,													/* tp_print */
	0,													/* tp_getattr */
	0,													/* tp_setattr */
	0,													/* tp_reserved */
	0,													/* tp_repr */
	0,													/* tp_as_number */
	0,													/* tp_as_sequence */
	0,													/* tp_as_mapping */
	0,													/* tp_hash  */
	0,													/* tp_call */
	0,													/* tp_str */
	0,													/* tp_getattro */
	0,													/* tp_setattro */
	0,													/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,			/* tp_flags */
	0,													/* tp_doc */
	0,													/* tp_traverse */
	0,													/* tp_clear */
	0,													/* tp_richcompare */
	0,													/* tp_weaklistoffset */
	0,													/* tp_iter */
	0,													/* tp_iternext */
	methods,											/* tp_methods */
	0,													/* tp_members */
	0,													/* tp_getset */
	0,													/* tp_base */
	0,													/* tp_dict */
	0,													/* tp_descr_get */
	0,													/* tp_descr_set */
	0,													/* tp_dictoffset */
	(initproc)&DCRunLoopOperationResultInit,			/* tp_init */
	0,													/* tp_alloc */
	&DCRunLoopOperationResultNew,						/* tp_new */
};

PyTypeObject* DCRunLoopOperationResultTypeObject(void)
{
	return &objectType;
}

PyObject* DCRunLoopOperationResultFromObject(DKRunLoopOperationResult* result)
{
	if (result)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();

		DCRunLoopOperationResult* self = (DCRunLoopOperationResult*)DCObjectCreateDefaultClass(&objectType, args, kwds);
		if (self)
		{
			self->result = result;
			Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
		}
		Py_XDECREF(args);
		Py_XDECREF(kwds);
		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKRunLoopOperationResult* DCRunLoopOperationResultToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCRunLoopOperationResult*)obj)->result;
	}
	return NULL;
}
