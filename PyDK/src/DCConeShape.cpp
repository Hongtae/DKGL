#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConvexShape.h"
#include "DCObject.h"

struct DCConeShape
{
	DCConvexShape base;
	DKConeShape* shape;
};

static PyObject* DCConeShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCConeShape* self = (DCConeShape*)DCConvexShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCConeShapeInit(DCConeShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKConeShape> shape = NULL;
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

		shape = DKOBJECT_NEW DKConeShape(radius, height, (DKCollisionShape::UpAxis)upaxis);
		self->shape = shape;
	}

	self->base.shape = self->shape;
	return DCConvexShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCConeShapeDealloc(DCConeShape* self)
{
	self->shape = NULL;
	DCConvexShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCConeShapeRadius(DCConeShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->Radius());
}

static PyObject* DCConeShapeHeight(DCConeShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->Height());
}

static PyObject* DCConeShapeScaledRadius(DCConeShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->ScaledRadius());
}

static PyObject* DCConeShapeScaledHeight(DCConeShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->ScaledHeight());
}

static PyObject* DCConeShapeBaseAxis(DCConeShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromLong((long)self->shape->BaseAxis());
}

static PyMethodDef methods[] = {
	{ "radius", (PyCFunction)&DCConeShapeRadius, METH_NOARGS },
	{ "height", (PyCFunction)&DCConeShapeHeight, METH_NOARGS },
	{ "scaledRadius", (PyCFunction)&DCConeShapeScaledRadius, METH_NOARGS },
	{ "scaledHeight", (PyCFunction)&DCConeShapeScaledHeight, METH_NOARGS },
	{ "baseAxis", (PyCFunction)&DCConeShapeBaseAxis, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ConeShape",			/* tp_name */
	sizeof(DCConeShape),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCConeShapeDealloc,			/* tp_dealloc */
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
	(initproc)&DCConeShapeInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCConeShapeNew,							/* tp_new */
};

PyTypeObject* DCConeShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCConeShapeFromObject(DKConeShape* shape)
{
	if (shape)
	{
		DCConeShape* self = (DCConeShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCConeShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKConeShape* DCConeShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCConeShape*)obj)->shape;
	}
	return NULL;
}
