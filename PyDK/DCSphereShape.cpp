#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConvexShape.h"
#include "DCObject.h"

struct DCSphereShape
{
	DCConvexShape base;
	DKSphereShape* shape;
};

static PyObject* DCSphereShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCSphereShape* self = (DCSphereShape*)DCConvexShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCSphereShapeInit(DCSphereShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKSphereShape> shape = NULL;
	if (self->shape == NULL)
	{
		float radius;
		if (!PyArg_ParseTuple(args, "f", &radius))
			return -1;

		shape = DKOBJECT_NEW DKSphereShape(radius);
		self->shape = shape;
	}

	self->base.shape = self->shape;
	return DCConvexShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCSphereShapeDealloc(DCSphereShape* self)
{
	self->shape = NULL;
	DCConvexShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCSphereShapeRadius(DCSphereShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->Radius());
}

static PyObject* DCSphereShapeScaledRadius(DCSphereShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->ScaledRadius());
}

static PyMethodDef methods[] = {
	{ "radius", (PyCFunction)&DCSphereShapeRadius, METH_NOARGS },
	{ "scaledRadius", (PyCFunction)&DCSphereShapeScaledRadius, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".SphereShape",			/* tp_name */
	sizeof(DCSphereShape),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCSphereShapeDealloc,			/* tp_dealloc */
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
	0,											/* tp_as_shape */
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
	DCConvexShapeTypeObject(),					/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCSphereShapeInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCSphereShapeNew,							/* tp_new */
};

PyTypeObject* DCSphereShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCSphereShapeFromObject(DKSphereShape* shape)
{
	if (shape)
	{
		DCSphereShape* self = (DCSphereShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCSphereShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->shape = shape;
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKSphereShape* DCSphereShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCSphereShape*)obj)->shape;
	}
	return NULL;
}
