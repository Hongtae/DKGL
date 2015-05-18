#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCPolyhedralConvexShape.h"
#include "DCObject.h"

struct DCBoxShape
{
	DCPolyhedralConvexShape base;
	DKBoxShape* shape;
};

static PyObject* DCBoxShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCBoxShape* self = (DCBoxShape*)DCPolyhedralConvexShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCBoxShapeInit(DCBoxShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKBoxShape> shape = NULL;
	if (self->shape == NULL)
	{
		float u, v, w;
		if (!PyArg_ParseTuple(args, "fff", &u, &v, &w))
			return -1;

		shape = DKOBJECT_NEW DKBoxShape(u, v, w);
		self->shape = shape;
	}

	self->base.shape = self->shape;
	return DCPolyhedralConvexShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCBoxShapeDealloc(DCBoxShape* self)
{
	self->shape = NULL;
	DCPolyhedralConvexShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCBoxShapeHalfExtents(DCBoxShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->HalfExtents();
	return DCVector3FromObject(&v);
}

static PyObject* DCBoxShapeHalfExtentsWithMargin(DCBoxShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->HalfExtentsWithMargin();
	return DCVector3FromObject(&v);
}

static PyObject* DCBoxShapeScaledHalfExtents(DCBoxShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->ScaledHalfExtents();
	return DCVector3FromObject(&v);
}

static PyObject* DCBoxShapeScaledHalfExtentsWithMargin(DCBoxShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->ScaledHalfExtentsWithMargin();
	return DCVector3FromObject(&v);
}

static PyMethodDef methods[] = {
	{ "halfExtents", (PyCFunction)&DCBoxShapeHalfExtents, METH_NOARGS },
	{ "halfExtentsWithMargin", (PyCFunction)&DCBoxShapeHalfExtentsWithMargin, METH_NOARGS },
	{ "scaledHalfExtents", (PyCFunction)&DCBoxShapeScaledHalfExtents, METH_NOARGS },
	{ "scaledHalfExtentsWithMargin", (PyCFunction)&DCBoxShapeScaledHalfExtentsWithMargin, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".BoxShape",				/* tp_name */
	sizeof(DCBoxShape),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCBoxShapeDealloc,				/* tp_dealloc */
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
	DCPolyhedralConvexShapeTypeObject(),		/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCBoxShapeInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCBoxShapeNew,								/* tp_new */
};

PyTypeObject* DCBoxShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCBoxShapeFromObject(DKBoxShape* shape)
{
	if (shape)
	{
		DCBoxShape* self = (DCBoxShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCBoxShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKBoxShape* DCBoxShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCBoxShape*)obj)->shape;
	}
	return NULL;
}
