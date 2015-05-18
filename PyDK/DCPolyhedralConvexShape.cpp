#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCPolyhedralConvexShape.h"
#include "DCObject.h"

static PyObject* DCPolyhedralConvexShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCPolyhedralConvexShape* self = (DCPolyhedralConvexShape*)DCConvexShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCPolyhedralConvexShapeInit(DCPolyhedralConvexShape *self, PyObject *args, PyObject *kwds)
{
	if (self->shape == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid object");
		return -1;
	}

	self->base.shape = self->shape;
	return DCConvexShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCPolyhedralConvexShapeDealloc(DCPolyhedralConvexShape* self)
{
	self->shape = NULL;
	DCConvexShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCPolyhedralConvexShapeNumberOfVertices(DCPolyhedralConvexShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromLong(self->shape->NumberOfVertices());
}

static PyObject* DCPolyhedralConvexShapeNumberOfEdges(DCPolyhedralConvexShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromLong(self->shape->NumberOfEdges());
}

static PyObject* DCPolyhedralConvexShapeNumberOfPlanes(DCPolyhedralConvexShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromLong(self->shape->NumberOfPlanes());
}

static PyObject* DCPolyhedralConvexShapeEdgeAtIndex(DCPolyhedralConvexShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	if (index >= 0 && index < self->shape->NumberOfEdges())
	{
		DKLine edge = self->shape->EdgeAtIndex(index);
		return Py_BuildValue("NN",
			DCVector3FromObject(&edge.begin),
			DCVector3FromObject(&edge.end));
	}
	PyErr_SetString(PyExc_ValueError, "index is out of range.");
	return NULL;
}

static PyObject* DCPolyhedralConvexShapeVertexAtIndex(DCPolyhedralConvexShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	if (index >= 0 && index < self->shape->NumberOfVertices())
	{
		DKVector3 v = self->shape->VertexAtIndex(index);
		return DCVector3FromObject(&v);
	}
	PyErr_SetString(PyExc_ValueError, "index is out of range.");
	return NULL;
}

static PyObject* DCPolyhedralConvexShapePlaneAtIndex(DCPolyhedralConvexShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	if (index >= 0 && index < self->shape->NumberOfPlanes())
	{
		DKPlane plane = self->shape->PlaneAtIndex(index);
		return Py_BuildValue("ffff", plane.a, plane.b, plane.c, plane.d);
	}
	PyErr_SetString(PyExc_ValueError, "index is out of range.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "numberOfVertices", (PyCFunction)&DCPolyhedralConvexShapeNumberOfVertices, METH_NOARGS },
	{ "numberOfEdges", (PyCFunction)&DCPolyhedralConvexShapeNumberOfEdges, METH_NOARGS },
	{ "numberOfPlanes", (PyCFunction)&DCPolyhedralConvexShapeNumberOfPlanes, METH_NOARGS },
	{ "edgeAtIndex", (PyCFunction)&DCPolyhedralConvexShapeEdgeAtIndex, METH_VARARGS },
	{ "vertexAtIndex", (PyCFunction)&DCPolyhedralConvexShapeVertexAtIndex, METH_VARARGS },
	{ "planeAtIndex", (PyCFunction)&DCPolyhedralConvexShapePlaneAtIndex, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".PolyhedralConvexShape",		/* tp_name */
	sizeof(DCPolyhedralConvexShape),				/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCPolyhedralConvexShapeDealloc,	/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	0,												/* tp_repr */
	0,												/* tp_as_number */
	0,												/* tp_as_sequence */
	0,												/* tp_as_mapping */
	0,												/* tp_hash  */
	0,												/* tp_call */
	0,												/* tp_str */
	0,												/* tp_getattro */
	0,												/* tp_setattro */
	0,												/* tp_as_shape */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		/* tp_flags */
	0,												/* tp_doc */
	0,												/* tp_traverse */
	0,												/* tp_clear */
	0,												/* tp_richcompare */
	0,												/* tp_weaklistoffset */
	0,												/* tp_iter */
	0,												/* tp_iternext */
	methods,										/* tp_methods */
	0,												/* tp_members */
	0,												/* tp_getset */
	DCConvexShapeTypeObject(),						/* tp_base */
	0,												/* tp_dict */
	0,												/* tp_descr_get */
	0,												/* tp_descr_set */
	0,												/* tp_dictoffset */
	(initproc)&DCPolyhedralConvexShapeInit,			/* tp_init */
	0,												/* tp_alloc */
	&DCPolyhedralConvexShapeNew,					/* tp_new */
};

PyTypeObject* DCPolyhedralConvexShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCPolyhedralConvexShapeFromObject(DKPolyhedralConvexShape* shape)
{
	if (shape)
	{
		DCPolyhedralConvexShape* self = (DCPolyhedralConvexShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(BoxShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(ConvexHullShape, shape);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCPolyhedralConvexShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKPolyhedralConvexShape* DCPolyhedralConvexShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCPolyhedralConvexShape*)obj)->shape;
	}
	return NULL;
}
