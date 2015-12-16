#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConcaveShape.h"
#include "DCObject.h"

struct DCStaticTriangleMeshShape
{
	DCConcaveShape base;
	DKStaticTriangleMeshShape* shape;
};

static PyObject* DCStaticTriangleMeshShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCStaticTriangleMeshShape* self = (DCStaticTriangleMeshShape*)DCConcaveShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCStaticTriangleMeshShapeInit(DCStaticTriangleMeshShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKStaticTriangleMeshShape> shape = NULL;
	if (self->shape == NULL)
	{
		Py_buffer vertexBuffer, indexBuffer;
		int use16bit = 0;
		char* kwlist[] = { "vertexBuffer", "indexBuffer", "use16bitIndex", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "y*y*p", kwlist,
			&vertexBuffer, &indexBuffer, &use16bit))
			return -1;

		DKVector3* vertices = reinterpret_cast<DKVector3*>(vertexBuffer.buf);
		size_t numVerts = vertexBuffer.len / sizeof(DKVector3);

		if (use16bit)
		{
			unsigned int* indices = reinterpret_cast<unsigned int*>(indexBuffer.buf);
			size_t numIndices = indexBuffer.len / sizeof(unsigned int);
			if (numVerts > 0 && numIndices > 0)
			{
				shape = DKOBJECT_NEW DKStaticTriangleMeshShape(vertices, numVerts, indices, numIndices, DKAabb());
			}
		}
		else
		{
			unsigned int* indices = reinterpret_cast<unsigned int*>(indexBuffer.buf);
			size_t numIndices = indexBuffer.len / sizeof(unsigned int);
			if (numVerts > 0 && numIndices > 0)
			{
				shape = DKOBJECT_NEW DKStaticTriangleMeshShape(vertices, numVerts, indices, numIndices, DKAabb());
			}
		}
		PyBuffer_Release(&vertexBuffer);
		PyBuffer_Release(&indexBuffer);

		if (shape == NULL)
		{
			PyErr_SetString(PyExc_ValueError, "invalid argument.");
			return -1;
		}
		self->shape = shape;
	}

	self->base.shape = self->shape;
	return DCConcaveShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCStaticTriangleMeshShapeDealloc(DCStaticTriangleMeshShape* self)
{
	self->shape = NULL;
	DCConcaveShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCStaticTriangleMeshShapeMeshAABB(DCStaticTriangleMeshShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKAabb aabb = self->shape->Aabb();
	return Py_BuildValue("NN", DCVector3FromObject(&aabb.positionMin), DCVector3FromObject(&aabb.positionMax));
}

static PyObject* DCStaticTriangleMeshShapeVertexAtIndex(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	size_t numVerts = self->shape->NumberOfVertices();
	if (index >= 0 && index < numVerts)
	{
		DKVector3 vertex = self->shape->VertexAtIndex(index);
		return DCVector3FromObject(&vertex);
	}
	PyErr_SetString(PyExc_IndexError, "vertex index is out of range.");
	return NULL;
}

static PyObject* DCStaticTriangleMeshShapeTriangleAtIndex(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	size_t numTriangles = self->shape->NumberOfTriangles();
	if (index >= 0 && index < numTriangles)
	{
		DKTriangle triangle = self->shape->TriangleAtIndex(index);
		return Py_BuildValue("NNN",
							 DCVector3FromObject(&triangle.position1),
							 DCVector3FromObject(&triangle.position2),
							 DCVector3FromObject(&triangle.position3));
	}
	PyErr_SetString(PyExc_IndexError, "triangle index is out of range.");
	return NULL;
}


static PyMethodDef methods[] = {
	{ "meshAABB", (PyCFunction)&DCStaticTriangleMeshShapeMeshAABB, METH_NOARGS },
	{ "vertexAtIndex", (PyCFunction)&DCStaticTriangleMeshShapeVertexAtIndex, METH_VARARGS },
	{ "triangleAtIndex", (PyCFunction)&DCStaticTriangleMeshShapeTriangleAtIndex, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCStaticTriangleMeshShapeNumberOfVertices(DCStaticTriangleMeshShape* self, void*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromSize_t(self->shape->NumberOfVertices());
}

static PyObject* DCStaticTriangleMeshShapeNumberOfIndices(DCStaticTriangleMeshShape* self, void*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromSize_t(self->shape->NumberOfIndices());
}

static PyObject* DCStaticTriangleMeshShapeIndexSize(DCStaticTriangleMeshShape* self, void*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromSize_t(self->shape->IndexSize());
}

static PyObject* DCStaticTriangleMeshShapeNumberOfTriangles(DCStaticTriangleMeshShape* self, void*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromSize_t(self->shape->NumberOfTriangles());
}

static PyGetSetDef getsets[] = {
	{ "numberOfVertices", (getter)&DCStaticTriangleMeshShapeNumberOfVertices, NULL, 0, 0 },
	{ "numberOfIndices", (getter)&DCStaticTriangleMeshShapeNumberOfIndices, NULL, 0, 0 },
	{ "indexSize", (getter)&DCStaticTriangleMeshShapeIndexSize, NULL, 0, 0 },
	{ "numberOfTriangles", (getter)&DCStaticTriangleMeshShapeNumberOfTriangles, NULL, 0, 0 },
	{ NULL }  /* Sentinel */
};


static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".StaticTriangleMeshShape",	/* tp_name */
	sizeof(DCStaticTriangleMeshShape),				/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCStaticTriangleMeshShapeDealloc,	/* tp_dealloc */
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
	getsets,										/* tp_getset */
	DCConcaveShapeTypeObject(),						/* tp_base */
	0,												/* tp_dict */
	0,												/* tp_descr_get */
	0,												/* tp_descr_set */
	0,												/* tp_dictoffset */
	(initproc)&DCStaticTriangleMeshShapeInit,		/* tp_init */
	0,												/* tp_alloc */
	&DCStaticTriangleMeshShapeNew,					/* tp_new */
};

PyTypeObject* DCStaticTriangleMeshShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCStaticTriangleMeshShapeFromObject(DKStaticTriangleMeshShape* shape)
{
	if (shape)
	{
		DCStaticTriangleMeshShape* self = (DCStaticTriangleMeshShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCStaticTriangleMeshShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKStaticTriangleMeshShape* DCStaticTriangleMeshShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCStaticTriangleMeshShape*)obj)->shape;
	}
	return NULL;
}
