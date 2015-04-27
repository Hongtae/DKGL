#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConvexShape.h"
#include "DCObject.h"

struct DCCapsuleShape
{
	DCConvexShape base;
	DKCapsuleShape* shape;
};

static PyObject* DCCapsuleShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCCapsuleShape* self = (DCCapsuleShape*)DCConvexShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCCapsuleShapeInit(DCCapsuleShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKCapsuleShape> shape = NULL;
	if (self->shape == NULL)
	{
		float radius, height;
		int upaxis = (int)DKCollisionShape::UpAxis::Top;
		if (!PyArg_ParseTuple(args, "ff|i", &radius, &height, &upaxis))
			return -1;

		if (upaxis != (int)DKCollisionShape::UpAxis::Left &&
			upaxis != (int)DKCollisionShape::UpAxis::Top &&
			upaxis != (int)DKCollisionShape::UpAxis::Forward)
		{
			PyErr_SetString(PyExc_ValueError, "third parameter must be Axis constant value.");
			return -1;
		}

		shape = DKOBJECT_NEW DKCapsuleShape(radius, height, (DKCollisionShape::UpAxis)upaxis);
		self->shape = shape;
	}

	self->base.shape = self->shape;
	return DCConvexShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCCapsuleShapeDealloc(DCCapsuleShape* self)
{
	self->shape = NULL;
	DCConvexShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCCapsuleShapeRadius(DCCapsuleShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->Radius());
}

static PyObject* DCCapsuleShapeHalfHeight(DCCapsuleShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->HalfHeight());
}

static PyObject* DCCapsuleShapeScaledRadius(DCCapsuleShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->ScaledRadius());
}

static PyObject* DCCapsuleShapeScaledHalfHeight(DCCapsuleShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->ScaledHalfHeight());
}

static PyObject* DCCapsuleShapeBaseAxis(DCCapsuleShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromLong((long)self->shape->BaseAxis());
}

static PyMethodDef methods[] = {
	{ "radius", (PyCFunction)&DCCapsuleShapeRadius, METH_NOARGS },
	{ "halfHeight", (PyCFunction)&DCCapsuleShapeHalfHeight, METH_NOARGS },
	{ "scaledRadius", (PyCFunction)&DCCapsuleShapeScaledRadius, METH_NOARGS },
	{ "scaledHalfHeight", (PyCFunction)&DCCapsuleShapeScaledHalfHeight, METH_NOARGS },
	{ "baseAxis", (PyCFunction)&DCCapsuleShapeBaseAxis, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".CapsuleShape",			/* tp_name */
	sizeof(DCCapsuleShape),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCCapsuleShapeDealloc,			/* tp_dealloc */
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
	(initproc)&DCCapsuleShapeInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCCapsuleShapeNew,							/* tp_new */
};

PyTypeObject* DCCapsuleShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCCapsuleShapeFromObject(DKCapsuleShape* shape)
{
	if (shape)
	{
		DCCapsuleShape* self = (DCCapsuleShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCCapsuleShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKCapsuleShape* DCCapsuleShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCCapsuleShape*)obj)->shape;
	}
	return NULL;
}
