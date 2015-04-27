#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConvexShape.h"
#include "DCObject.h"

static PyObject* DCConvexShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCConvexShape* self = (DCConvexShape*)DCCollisionShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCConvexShapeInit(DCConvexShape *self, PyObject *args, PyObject *kwds)
{
	if (self->shape == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid object");
		return -1;
	}

	self->base.shape = self->shape;
	return DCCollisionShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCConvexShapeDealloc(DCConvexShape* self)
{
	self->shape = NULL;
	DCCollisionShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCConvexShapeProject(DCConvexShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);

	DKNSTransform trans;
	DKVector3 dir;
	if (!PyArg_ParseTuple(args, "O&O&",
		&DCNSTransformConverter, &trans,
		&DCVector3Converter, &dir))
		return NULL;

	float min, max;
	self->shape->Project(trans, dir, min, max);
	return Py_BuildValue("ff", min, max);
}

static PyObject* DCConvexShapeIntersect(DCConvexShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);

	DKNSTransform transA, transB;
	PyObject* shapeBObj;
	if (!PyArg_ParseTuple(args, "OO&O&", &shapeBObj,
		&DCNSTransformConverter, &transA,
		&DCNSTransformConverter, &transB))
		return NULL;

	DKConvexShape* shapeB = DCConvexShapeToObject(shapeBObj);
	if (shapeB == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "first argument must be ConvexShape object.");
		return NULL;
	}
	if (DKConvexShape::Intersect(self->shape, transA, shapeB, transB))
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCConvexShapeRayTest(DCConvexShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKNSTransform trans;
	DKVector3 begin, end;
	if (!PyArg_ParseTuple(args, "O&O&O&",
		&DCNSTransformConverter, &trans,
		&DCVector3Converter, &begin,
		&DCVector3Converter, &end))
		return NULL;

	DKVector3 pt;
	if (self->shape->RayTest(trans, DKLine(begin, end), &pt))
	{
		return DCVector3FromObject(&pt);
	}
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "project", (PyCFunction)&DCConvexShapeProject, METH_VARARGS },
	{ "intersect", (PyCFunction)&DCConvexShapeIntersect, METH_VARARGS },
	{ "rayTest", (PyCFunction)&DCConvexShapeRayTest, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ConvexShape",			/* tp_name */
	sizeof(DCConvexShape),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCConvexShapeDealloc,			/* tp_dealloc */
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
	(initproc)&DCConvexShapeInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCConvexShapeNew,							/* tp_new */
};

PyTypeObject* DCConvexShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCConvexShapeFromObject(DKConvexShape* shape)
{
	if (shape)
	{
		DCConvexShape* self = (DCConvexShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(CapsuleShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(ConeShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(CylinderShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(MultiSphereShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(PolyhedralConvexShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(SphereShape, shape);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCConvexShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKConvexShape* DCConvexShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCConvexShape*)obj)->shape;
	}
	return NULL;
}
