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
		int rebuildIndex = 1;
		float welding = 0.0f;
		char* kwlist[] = { "vertexBuffer", "indexBuffer", "use16bitIndex", "rebuildIndex", "weldingThreshold", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "y*y*|ppf", kwlist,
			&vertexBuffer, &indexBuffer, &use16bit, &rebuildIndex, &welding))
			return -1;

		DKVector3* vertices = reinterpret_cast<DKVector3*>(vertexBuffer.buf);
		size_t numVerts = vertexBuffer.len / sizeof(DKVector3);

		if (use16bit)
		{
			unsigned int* indices = reinterpret_cast<unsigned int*>(indexBuffer.buf);
			size_t numIndices = indexBuffer.len / sizeof(unsigned int);
			if (numVerts > 0 && numIndices > 0)
			{
				shape = DKOBJECT_NEW DKStaticTriangleMeshShape(vertices, numVerts, indices, numIndices, DKAABox(), rebuildIndex != 0, welding);
			}
		}
		else
		{
			unsigned int* indices = reinterpret_cast<unsigned int*>(indexBuffer.buf);
			size_t numIndices = indexBuffer.len / sizeof(unsigned int);
			if (numVerts > 0 && numIndices > 0)
			{
				shape = DKOBJECT_NEW DKStaticTriangleMeshShape(vertices, numVerts, indices, numIndices, DKAABox(), rebuildIndex != 0, welding);
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

static PyObject* DCStaticTriangleMeshShapeRefitBVH(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 aabbMin, aabbMax;
	if (!PyArg_ParseTuple(args, "O&O&", &DCVector3Converter, &aabbMin, &DCVector3Converter, &aabbMax))
		return NULL;

	self->shape->RefitBVH(DKAABox(aabbMin, aabbMax));
	Py_RETURN_NONE;
}

static PyObject* DCStaticTriangleMeshShapePartialRefitBVH(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKVector3 aabbMin, aabbMax;
	if (!PyArg_ParseTuple(args, "O&O&", &DCVector3Converter, &aabbMin, &DCVector3Converter, &aabbMax))
		return NULL;

	self->shape->PartialRefitBVH(DKAABox(aabbMin, aabbMax));
	Py_RETURN_NONE;
}

static PyObject* DCStaticTriangleMeshShapeMeshAABB(DCStaticTriangleMeshShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	DKAABox aabb = self->shape->MeshAABB();
	return Py_BuildValue("NN", DCVector3FromObject(&aabb.positionMin), DCVector3FromObject(&aabb.positionMax));
}

static PyObject* DCStaticTriangleMeshShapeNumberOfTriangles(DCStaticTriangleMeshShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromSize_t(self->shape->NumberOfTriangles());
}

static PyObject* DCStaticTriangleMeshShapeGetTriangleVertexIndices(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int triangle;
	if (!PyArg_ParseTuple(args, "i", &triangle))
		return NULL;

	unsigned int index[3];
	if (self->shape->GetTriangleVertexIndices(triangle, index))
	{
		return Py_BuildValue("III", index[0], index[1], index[2]);
	}
	PyErr_SetString(PyExc_IndexError, "triangle index is out of range.");
	return NULL;
}

static PyObject* DCStaticTriangleMeshShapeGetTriangleFace(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	DKTriangle triangle;
	if (self->shape->GetTriangleFace(index, triangle))
	{
		return Py_BuildValue("NNN",
			DCVector3FromObject(&triangle.position1),
			DCVector3FromObject(&triangle.position2),
			DCVector3FromObject(&triangle.position3));
	}
	PyErr_SetString(PyExc_IndexError, "triangle index is out of range.");
	return NULL;
}

static PyObject* DCStaticTriangleMeshShapeSetTriangleFace(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	DKTriangle triangle;
	if (!PyArg_ParseTuple(args, "iO&O&O&", &index,
		&DCVector3Converter, &triangle.position1,
		&DCVector3Converter, &triangle.position2,
		&DCVector3Converter, &triangle.position3))
		return NULL;

	if (self->shape->SetTriangleFace(index, triangle))
	{
		Py_RETURN_NONE;
	}
	PyErr_SetString(PyExc_IndexError, "triangle index is out of range.");
	return NULL;
}

static PyObject* DCStaticTriangleMeshShapeGetVertex(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	size_t numVerts;
	DKVector3* verts = self->shape->VertexBuffer(&numVerts);
	if (index >= 0 && index < numVerts)
	{
		return DCVector3FromObject(&verts[index]);
	}
	PyErr_SetString(PyExc_IndexError, "vertex index is out of range.");
	return NULL;
}

static PyObject* DCStaticTriangleMeshShapeSetVertex(DCStaticTriangleMeshShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	int index;
	DKVector3 vertex;
	if (!PyArg_ParseTuple(args, "iO&", &index, &DCVector3Converter, &vertex))
		return NULL;

	size_t numVerts;
	DKVector3* verts = self->shape->VertexBuffer(&numVerts);
	if (index >= 0 && index < numVerts)
	{
		verts[index] = vertex;
		Py_RETURN_NONE;
	}
	PyErr_SetString(PyExc_IndexError, "vertex index is out of range.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "refitBVH", (PyCFunction)&DCStaticTriangleMeshShapeRefitBVH, METH_VARARGS },
	{ "partialRefitBVH", (PyCFunction)&DCStaticTriangleMeshShapePartialRefitBVH, METH_VARARGS },
	{ "meshAABB", (PyCFunction)&DCStaticTriangleMeshShapeMeshAABB, METH_NOARGS },
	{ "numberOfTriangles", (PyCFunction)&DCStaticTriangleMeshShapeNumberOfTriangles, METH_NOARGS },
	{ "getTriangleVertexIndices", (PyCFunction)&DCStaticTriangleMeshShapeGetTriangleVertexIndices, METH_VARARGS },
	{ "getTriangleFace", (PyCFunction)&DCStaticTriangleMeshShapeGetTriangleFace, METH_VARARGS },
	{ "setTriangleFace", (PyCFunction)&DCStaticTriangleMeshShapeSetTriangleFace, METH_VARARGS },
	{ "getVertex", (PyCFunction)&DCStaticTriangleMeshShapeGetVertex, METH_VARARGS },
	{ "setVertex", (PyCFunction)&DCStaticTriangleMeshShapeSetVertex, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
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
	0,												/* tp_getset */
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
