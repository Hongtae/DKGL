#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCPolyhedralConvexShape.h"
#include "DCObject.h"

struct DCConvexHullShape
{
	DCPolyhedralConvexShape base;
	DKConvexHullShape* shape;
};

static PyObject* DCConvexHullShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCConvexHullShape* self = (DCConvexHullShape*)DCPolyhedralConvexShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCConvexHullShapeInit(DCConvexHullShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKConvexHullShape> shape = NULL;
	if (self->shape == NULL)
	{
		Py_buffer buffer;
		memset(&buffer, 0, sizeof(Py_buffer));
		if (!PyArg_ParseTuple(args, "|y*", &buffer))
			return -1;

		if (buffer.len > 0)
		{
			DKVector3* verts = reinterpret_cast<DKVector3*>(buffer.buf);
			size_t numVerts = buffer.len / sizeof(DKVector3);
			shape = DKOBJECT_NEW DKConvexHullShape(verts, numVerts);
		}
		else
		{
			shape = DKOBJECT_NEW DKConvexHullShape(0,0);
		}

		if (buffer.obj)
		{
			PyBuffer_Release(&buffer);
		}

		self->shape = shape;
	}

	self->base.shape = self->shape;
	return DCPolyhedralConvexShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCConvexHullShapeDealloc(DCConvexHullShape* self)
{
	self->shape = NULL;
	DCPolyhedralConvexShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCConvexHullShapeAddPoint(DCConvexHullShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 pos;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &pos))
		return NULL;

	self->shape->AddPoint(pos);
	Py_RETURN_NONE;
}

static PyObject* DCConvexHullShapeNumberOfPoints(DCConvexHullShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromSize_t(self->shape->NumberOfPoints());
}

static PyObject* DCConvexHullShapePointAtIndex(DCConvexHullShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	if (index >= 0 && index < self->shape->NumberOfPoints())
	{
		DKVector3 v = self->shape->PointAtIndex(index);
		return DCVector3FromObject(&v);
	}
	PyErr_SetString(PyExc_IndexError, "index is out of range.");
	return NULL;
}

static PyObject* DCConvexHullShapeScaledPointAtIndex(DCConvexHullShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	if (index >= 0 && index < self->shape->NumberOfPoints())
	{
		DKVector3 v = self->shape->ScaledPointAtIndex(index);
		return DCVector3FromObject(&v);
	}
	PyErr_SetString(PyExc_IndexError, "index is out of range.");
	return NULL;
}

static PyObject* DCConvexHullShapeCreateHull(PyObject*, PyObject* args)
{
	Py_buffer buffer;
	if (!PyArg_ParseTuple(args, "y*", &buffer))
		return NULL;

	const DKTriangle* tri = reinterpret_cast<const DKTriangle*>(buffer.buf);
	size_t numTri = buffer.len / sizeof(DKTriangle);

	DKObject<DKConvexHullShape> shape = NULL;
	Py_BEGIN_ALLOW_THREADS
	shape = DKConvexHullShape::CreateHull(tri, numTri);
	Py_END_ALLOW_THREADS

	PyBuffer_Release(&buffer);

	if (shape)
		return DCConvexHullShapeFromObject(shape);

	PyErr_SetString(PyExc_ValueError, "Invalid triangles.");
	return NULL;
}

static PyObject* DCConvexHullShapeDecomposeTriangleMesh(PyObject*, PyObject* args)
{
	Py_buffer vertexBuffer, indexBuffer;
	Py_ssize_t minClusters = 2;
	Py_ssize_t maxVertsPerCH = 100;
	double maxConcavity = 100.0;
	int addExtraDistPoints = 0;
	int addNeighboursDistPoints = 0;
	int addFacesPoints = 0;

	if (!PyArg_ParseTuple(args, "y*y*|nndppp", &vertexBuffer, &indexBuffer,
		&minClusters, &maxVertsPerCH, &maxConcavity,
		&addExtraDistPoints, &addNeighboursDistPoints, &addFacesPoints))
		return NULL;

	const DKVector3* vertices = reinterpret_cast<const DKVector3*>(vertexBuffer.buf);
	size_t numVerts = vertexBuffer.len / sizeof(DKVector3);
	const long* indices = reinterpret_cast<const long*>(indexBuffer.buf);
	size_t numIndices = indexBuffer.len / sizeof(long);

	DKConvexHullShape::ConvexHullArray result;

	Py_BEGIN_ALLOW_THREADS
		result = DKConvexHullShape::DecomposeTriangleMesh(vertices, numVerts, indices, numIndices,
		minClusters, maxVertsPerCH, maxConcavity,
		addExtraDistPoints != 0,
		addNeighboursDistPoints != 0,
		addFacesPoints != 0);

	Py_END_ALLOW_THREADS

	PyBuffer_Release(&vertexBuffer);
	PyBuffer_Release(&indexBuffer);

	PyObject* tuple = PyTuple_New(result.Count());
	for (size_t i = 0; i < result.Count(); ++i)
	{
		DKConvexHullShape* shape = result.Value(i).shape;
		DKNSTransform& offset = result.Value(i).offset;

		DKASSERT_DEBUG(shape);

		PyObject* obj = DCConvexHullShapeFromObject(shape);
		DKASSERT_DEBUG(DCConvexHullShapeToObject(obj) == shape);

		PyObject* t = PyTuple_New(2);
		PyTuple_SET_ITEM(t, 0, obj);
		PyTuple_SET_ITEM(t, 1, DCNSTransformFromObject(&offset));

		PyTuple_SET_ITEM(tuple, i, t);
	}
	return tuple;
}

static PyMethodDef methods[] = {
	{ "addPoint", (PyCFunction)&DCConvexHullShapeAddPoint, METH_VARARGS },
	{ "numberOfPoints", (PyCFunction)&DCConvexHullShapeNumberOfPoints, METH_NOARGS },
	{ "pointAtIndex", (PyCFunction)&DCConvexHullShapePointAtIndex, METH_VARARGS },
	{ "scaledPointAtIndex", (PyCFunction)&DCConvexHullShapeScaledPointAtIndex, METH_VARARGS },
	{ "createHull", (PyCFunction)&DCConvexHullShapeCreateHull, METH_VARARGS | METH_STATIC },
	{ "decomposeTriangleMesh", (PyCFunction)&DCConvexHullShapeDecomposeTriangleMesh, METH_VARARGS | METH_STATIC },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ConvexHullShape",		/* tp_name */
	sizeof(DCConvexHullShape),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCConvexHullShapeDealloc,		/* tp_dealloc */
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
	(initproc)&DCConvexHullShapeInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCConvexHullShapeNew,						/* tp_new */
};

PyTypeObject* DCConvexHullShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCConvexHullShapeFromObject(DKConvexHullShape* shape)
{
	if (shape)
	{
		DCConvexHullShape* self = (DCConvexHullShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCConvexHullShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKConvexHullShape* DCConvexHullShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCConvexHullShape*)obj)->shape;
	}
	return NULL;
}
