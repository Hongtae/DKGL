#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConvexShape.h"
#include "DCObject.h"

struct DCCylinderShape
{
	DCConvexShape base;
	DKCylinderShape* shape;
};

static PyObject* DCCylinderShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCCylinderShape* self = (DCCylinderShape*)DCConvexShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCCylinderShapeInit(DCCylinderShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKCylinderShape> shape = NULL;
	if (self->shape == NULL)
	{
		float u, v, w;
		int upaxis = (int)DKCollisionShape::UpAxis::Top;
		if (!PyArg_ParseTuple(args, "fff|i", &u, &v, &w, &upaxis))
			return -1;

		if (upaxis != (int)DKCollisionShape::UpAxis::Left &&
			upaxis != (int)DKCollisionShape::UpAxis::Top &&
			upaxis != (int)DKCollisionShape::UpAxis::Forward)
		{
			PyErr_SetString(PyExc_ValueError, "fourth parameter must be Axis constant value.");
			return -1;
		}

		shape = DKOBJECT_NEW DKCylinderShape(u, v, w, (DKCollisionShape::UpAxis)upaxis);
		self->shape = shape;
	}

	self->base.shape = self->shape;
	return DCConvexShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCCylinderShapeDealloc(DCCylinderShape* self)
{
	self->shape = NULL;
	DCConvexShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCCylinderShapeHalfExtents(DCCylinderShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->HalfExtents();
	return DCVector3FromObject(&v);
}

static PyObject* DCCylinderShapeHalfExtentsWithMargin(DCCylinderShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->HalfExtentsWithMargin();
	return DCVector3FromObject(&v);
}

static PyObject* DCCylinderShapeScaledHalfExtents(DCCylinderShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->ScaledHalfExtents();
	return DCVector3FromObject(&v);
}

static PyObject* DCCylinderShapeScaledHalfExtentsWithMargin(DCCylinderShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->ScaledHalfExtentsWithMargin();
	return DCVector3FromObject(&v);
}

static PyObject* DCCylinderShapeBaseAxis(DCCylinderShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromLong((long)self->shape->BaseAxis());
}

static PyMethodDef methods[] = {
	{ "halfExtents", (PyCFunction)&DCCylinderShapeHalfExtents, METH_NOARGS },
	{ "halfExtentsWithMargin", (PyCFunction)&DCCylinderShapeHalfExtentsWithMargin, METH_NOARGS },
	{ "scaledHalfExtents", (PyCFunction)&DCCylinderShapeScaledHalfExtents, METH_NOARGS },
	{ "scaledHalfExtentsWithMargin", (PyCFunction)&DCCylinderShapeScaledHalfExtentsWithMargin, METH_NOARGS },
	{ "baseAxis", (PyCFunction)&DCCylinderShapeBaseAxis, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".CylinderShape",			/* tp_name */
	sizeof(DCCylinderShape),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCCylinderShapeDealloc,		/* tp_dealloc */
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
	(initproc)&DCCylinderShapeInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCCylinderShapeNew,						/* tp_new */
};

PyTypeObject* DCCylinderShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCCylinderShapeFromObject(DKCylinderShape* shape)
{
	if (shape)
	{
		DCCylinderShape* self = (DCCylinderShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCCylinderShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKCylinderShape* DCCylinderShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCCylinderShape*)obj)->shape;
	}
	return NULL;
}
