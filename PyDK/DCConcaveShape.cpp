#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConcaveShape.h"
#include "DCObject.h"

static PyObject* DCConcaveShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCConcaveShape* self = (DCConcaveShape*)DCCollisionShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCConcaveShapeInit(DCConcaveShape *self, PyObject *args, PyObject *kwds)
{
	if (self->shape == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid object");
		return -1;
	}
	self->base.shape = self->shape;
	return DCCollisionShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCConcaveShapeDealloc(DCConcaveShape* self)
{
	self->shape = NULL;
	DCCollisionShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCConcaveShapeEnumerateTrianglesInsideAABB(DCConcaveShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);

	DKVector3 aabbMin, aabbMax;
	PyObject* func;
	if (!PyArg_ParseTuple(args, "OO&O&", &func,
		&DCVector3Converter, &aabbMin,
		&DCVector3Converter, &aabbMax))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be (callable, aabbMin:Vector3, aabbMax:Vector3)");
		return NULL;
	}

	if (!PyCallable_Check(func)) {
		PyErr_SetString(PyExc_TypeError, "first argument must be callable");
		return NULL;
	}

	auto enumerator = [func](const DKConcaveShape::Triangle& tri)
	{
		if (!PyErr_Occurred())
		{
			PyObject* tuple = PyTuple_New(3);
			PyTuple_SET_ITEM(tuple, 0, DCVector3FromObject((DKVector3*)&tri.pos[0]));
			PyTuple_SET_ITEM(tuple, 1, DCVector3FromObject((DKVector3*)&tri.pos[1]));
			PyTuple_SET_ITEM(tuple, 2, DCVector3FromObject((DKVector3*)&tri.pos[2]));

			PyObject* args = PyTuple_New(3);
			PyTuple_SET_ITEM(args, 0, tuple);
			PyTuple_SET_ITEM(args, 1, PyLong_FromLong(tri.partId));
			PyTuple_SET_ITEM(args, 2, PyLong_FromLong(tri.triangleIndex));
			PyObject* tmp = PyObject_Call(func, args, NULL);
			Py_DECREF(args);
			Py_XDECREF(tmp);
		}
	};
	self->shape->EnumerateTrianglesInsideAABB(DKFunction(enumerator), aabbMin, aabbMax);
	if (PyErr_Occurred())
		return NULL;

	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "enumerateTrianglesInsideAABB", (PyCFunction)&DCConcaveShapeEnumerateTrianglesInsideAABB, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ConcaveShape",			/* tp_name */
	sizeof(DCConcaveShape),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCConcaveShapeDealloc,			/* tp_dealloc */
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
	DCCollisionShapeTypeObject(),				/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCConcaveShapeInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCConcaveShapeNew,							/* tp_new */
};

PyTypeObject* DCConcaveShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCConcaveShapeFromObject(DKConcaveShape* shape)
{
	if (shape)
	{
		DCConcaveShape* self = (DCConcaveShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(StaticPlaneShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(StaticTriangleMeshShape, shape);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCConcaveShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKConcaveShape* DCConcaveShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCConcaveShape*)obj)->shape;
	}
	return NULL;
}
