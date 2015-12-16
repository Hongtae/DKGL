#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCGeometryBuffer.h"
#include "DCObject.h"

struct DCIndexBuffer
{
	DCGeometryBuffer base;
	DKIndexBuffer* buffer;
};

static PyObject* DCIndexBufferNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCIndexBuffer* self = (DCIndexBuffer*)DCResourceTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->buffer = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCIndexBufferInit(DCIndexBuffer *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKIndexBuffer> ib = NULL;
	if (self->buffer == NULL)
	{
		Py_buffer source;
		int indexType;
		Py_ssize_t count;
		int primitiveType;
		int location;
		int usage;

		char* kwlist[] = { "source", "indexType", "count", "primitiveType", "location", "usage", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "y*iniii", kwlist,
			&source, &indexType, &count, &primitiveType, &location, &usage))
			return NULL;

		DCBufferRelease _tmp[] = { &source };	// release buffer when return

		if (source.len <= 0)
		{
			PyErr_SetString(PyExc_ValueError, "first argument is invalid. (empty buffer)");
			return -1;
		}
		if (indexType != (int)DKIndexBuffer::Type::UInt8 &&
			indexType != (int)DKIndexBuffer::Type::UInt16 &&
			indexType != (int)DKIndexBuffer::Type::UInt32)
		{
			PyErr_SetString(PyExc_ValueError, "second argument is invalid. (invalid type)");
			return -1;
		}
		if (count <= 0)
		{
			PyErr_SetString(PyExc_ValueError, "third argument must be greater than zero.");
			return -1;
		}
		if (primitiveType <= DKPrimitive::TypeUnknown ||
			primitiveType > DKPrimitive::TypeTriangleStripAdjacency)	// DKPrimitive::Type
		{
			PyErr_SetString(PyExc_ValueError, "fourth argument is invalid. (wrong primitive)");
			return -1;
		}
		if (location < DKGeometryBuffer::MemoryLocationStatic ||
			location > DKGeometryBuffer::MemoryLocationStream)		// DKGeometryBuffer::MemoryLocation
		{
			PyErr_SetString(PyExc_ValueError, "fifth argument is invalid. (wrong location)");
			return -1;
		}
		if (usage < DKGeometryBuffer::BufferUsageDraw ||
			usage > DKGeometryBuffer::BufferUsageCopy)	// DKGeometryBuffer::BufferUsage
		{
			PyErr_SetString(PyExc_ValueError, "sixth argument is invalid. (wrong usage)");
			return -1;
		}

		size_t indexSize = 0;
		if (indexType == 0)
			indexSize = sizeof(unsigned char);
		else if (indexType == 1)
			indexSize = sizeof(unsigned short);
		else if (indexType == 2)
			indexSize = sizeof(unsigned int);
		else { DKASSERT_DEBUG(false); }

		size_t numIndices = source.len / indexSize;
		if (count > numIndices)
		{
			PyErr_SetString(PyExc_ValueError, "thrid argument is out of range");
			return -1;
		}

		Py_BEGIN_ALLOW_THREADS
		ib = DKIndexBuffer::Create((const void*)source.buf,
			(DKIndexBuffer::Type)indexType,
			(size_t)count,
			(DKPrimitive::Type)primitiveType,
			(DKGeometryBuffer::MemoryLocation)location,
			(DKGeometryBuffer::BufferUsage)usage);
		Py_END_ALLOW_THREADS

		if (ib == NULL)
		{
			PyErr_SetString(PyExc_RuntimeError, "failed to create index buffer");
			return -1;
		}
		self->buffer = ib;
		DCObjectSetAddress(self->buffer, (PyObject*)self);
	}

	DKASSERT_DEBUG(self->buffer);
	self->base.buffer = self->buffer;
	return DCGeometryBufferTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCIndexBufferDealloc(DCIndexBuffer* self)
{
	self->buffer = NULL;
	DCGeometryBufferTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCIndexBufferGetIndices(DCIndexBuffer* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);

	Py_ssize_t begin = 0;
	Py_ssize_t count = -1;

	if (!PyArg_ParseTuple(args, "|nn", &begin, &count))
		return NULL;

	DKArray<unsigned int> indices;
	bool b = false;
	Py_BEGIN_ALLOW_THREADS
	b = self->buffer->CopyIndices(indices);
	Py_END_ALLOW_THREADS
	if (b)
	{
		size_t numIndices = indices.Count();
		if (begin >= numIndices)
		{
			PyErr_SetString(PyExc_ValueError, "first argument is out of range");
			return NULL;
		}

		if (count < 0 || begin+count > numIndices)
			count = numIndices - begin;

		DKASSERT_DEBUG(begin + count <= indices.Count());

		PyObject* tuple = PyTuple_New(count);
		for (size_t i = 0; i < count; ++i)
		{
			unsigned int idx = indices.Value(begin + i);
			PyTuple_SET_ITEM(tuple, i, PyLong_FromLong(idx));
		}
		return tuple;
	}
	PyErr_SetString(PyExc_NotImplementedError, "failed to get data");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "getIndices", (PyCFunction)&DCIndexBufferGetIndices, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCIndexBufferPrimitiveType(DCIndexBuffer* self, void*)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	return PyLong_FromLong((long)self->buffer->PrimitiveType());
}

static PyObject* DCIndexBufferNumberOfIndices(DCIndexBuffer* self, void*)
{
	size_t num = 0;
	if (self->buffer)
		num = self->buffer->NumberOfIndices();
	return PyLong_FromSize_t(num);
}

static PyObject* DCIndexBufferIndexType(DCIndexBuffer* self, void*)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	return PyLong_FromLong((long)self->buffer->IndexType());
}

static PyGetSetDef getsets[] = {
	{ "primitiveType", (getter)&DCIndexBufferPrimitiveType, 0, 0, 0 },
	{ "numberOfIndices", (getter)&DCIndexBufferNumberOfIndices, 0, 0, 0 },
	{ "indexType", (getter)&DCIndexBufferIndexType, 0, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".IndexBuffer",			/* tp_name */
	sizeof(DCIndexBuffer),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCIndexBufferDealloc,			/* tp_dealloc */
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
	0,											/* tp_as_buffer */
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
	getsets,									/* tp_getset */
	DCGeometryBufferTypeObject(),				/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCIndexBufferInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCIndexBufferNew,								/* tp_new */
};

PyTypeObject* DCIndexBufferTypeObject(void)
{
	return &objectType;
}

PyObject* DCIndexBufferFromObject(DKIndexBuffer* buffer)
{
	if (buffer)
	{
		DCIndexBuffer* self = (DCIndexBuffer*)DCObjectFromAddress(buffer);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCIndexBuffer*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->buffer = buffer;
				DCObjectSetAddress(self->buffer, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKIndexBuffer* DCIndexBufferToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCIndexBuffer*)obj)->buffer;
	}
	return NULL;
}
