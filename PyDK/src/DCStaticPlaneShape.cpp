#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConcaveShape.h"
#include "DCObject.h"

struct DCStaticPlaneShape
{
	DCConcaveShape base;
	DKStaticPlaneShape* shape;
};

static PyObject* DCStaticPlaneShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCStaticPlaneShape* self = (DCStaticPlaneShape*)DCConcaveShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCStaticPlaneShapeInit(DCStaticPlaneShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKStaticPlaneShape> shape = NULL;
	if (self->shape == NULL)
	{
		DKVector3 planeNormal;
		float planeConstant;
		if (!PyArg_ParseTuple(args, "O&f", &DCVector3Converter, &planeNormal, &planeConstant))
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "argument must be Vector3, float.");
			return -1;
		}

		shape = DKOBJECT_NEW DKStaticPlaneShape(planeNormal, planeConstant);
		self->shape = shape;
	}

	self->base.shape = self->shape;
	return DCConcaveShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCStaticPlaneShapeDealloc(DCStaticPlaneShape* self)
{
	self->shape = NULL;
	DCConcaveShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCStaticPlaneShapePlaneNormal(DCStaticPlaneShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 v = self->shape->PlaneNormal();
	return DCVector3FromObject(&v);
}

static PyObject* DCStaticPlaneShapePlaneConstant(DCStaticPlaneShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyFloat_FromDouble(self->shape->PlaneConstant());
}

static PyMethodDef methods[] = {
	{ "planeNormal", (PyCFunction)&DCStaticPlaneShapePlaneNormal, METH_NOARGS },
	{ "planeConstant", (PyCFunction)&DCStaticPlaneShapePlaneConstant, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".StaticPlaneShape",		/* tp_name */
	sizeof(DCStaticPlaneShape),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCStaticPlaneShapeDealloc,		/* tp_dealloc */
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
	DCConcaveShapeTypeObject(),					/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCStaticPlaneShapeInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCStaticPlaneShapeNew,						/* tp_new */
};

PyTypeObject* DCStaticPlaneShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCStaticPlaneShapeFromObject(DKStaticPlaneShape* shape)
{
	if (shape)
	{
		DCStaticPlaneShape* self = (DCStaticPlaneShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCStaticPlaneShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKStaticPlaneShape* DCStaticPlaneShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCStaticPlaneShape*)obj)->shape;
	}
	return NULL;
}
