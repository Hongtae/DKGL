#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCCollisionObject.h"
#include "DCObject.h"

struct DCSoftBody
{
	DCCollisionObject base;
	DKSoftBody* body;
};

static PyObject* DCSoftBodyNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCSoftBody* self = (DCSoftBody*)DCCollisionObjectTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->body = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCSoftBodyInit(DCSoftBody *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKSoftBody> body = NULL;
	if (self->body == NULL)
	{
		PyErr_SetString(PyExc_NotImplementedError, "Not implemented yet!");
		return -1;
	}

	self->base.object = self->body;
	return DCCollisionObjectTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCSoftBodyDealloc(DCSoftBody* self)
{
	self->body = NULL;
	DCCollisionObjectTypeObject()->tp_dealloc((PyObject*)self);
}

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".SoftBody",				/* tp_name */
	sizeof(DCSoftBody),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCSoftBodyDealloc,				/* tp_dealloc */
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
	0,											/* tp_as_body */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
	0,											/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	0,											/* tp_methods */
	0,											/* tp_members */
	0,											/* tp_getset */
	DCCollisionObjectTypeObject(),				/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCSoftBodyInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCSoftBodyNew,								/* tp_new */
};

PyTypeObject* DCSoftBodyTypeObject(void)
{
	return &objectType;
}

PyObject* DCSoftBodyFromObject(DKSoftBody* body)
{
	if (body)
	{
		DCSoftBody* self = (DCSoftBody*)DCObjectFromAddress(body);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCSoftBody*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->body = body;
				DCObjectSetAddress(self->body, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKSoftBody* DCSoftBodyToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCSoftBody*)obj)->body;
	}
	return NULL;
}
