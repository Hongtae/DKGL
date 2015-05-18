#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCGeometryBuffer.h"
#include "DCObject.h"

struct DCVertexBuffer
{
	DCGeometryBuffer base;
	DKVertexBuffer* buffer;
};

static PyObject* DCVertexBufferNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCVertexBuffer* self = (DCVertexBuffer*)DCResourceTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->buffer = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCVertexBufferInit(DCVertexBuffer *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKVertexBuffer> vb = NULL;
	if (self->buffer == NULL)
	{
		PyObject* decls;
		Py_ssize_t size;
		int location;
		int usage;

		char* kwlist[] = { "decls", "size", "location", "usage", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "Onii", kwlist,
			&decls, &size, &location, &usage))
			return -1;

		if (!PySequence_Check(decls))
		{
			PyErr_SetString(PyExc_TypeError, "first argument must be sequence object.");
			return -1;
		}
		if (size <= 0)
		{
			PyErr_SetString(PyExc_ValueError, "second argument must be greater than zero.");
			return -1;
		}
		// DKGeometryBuffer::MemoryLocation
		if (location < DKGeometryBuffer::MemoryLocationStatic ||
			location > DKGeometryBuffer::MemoryLocationStream)
		{
			PyErr_SetString(PyExc_ValueError, "thrid argument is invalid. (wrong location)");
			return -1;
		}
		// DKGeometryBuffer::BufferUsage
		if (usage < DKGeometryBuffer::BufferUsageDraw ||
			usage > DKGeometryBuffer::BufferUsageCopy)
		{
			PyErr_SetString(PyExc_ValueError, "fourth argument is invalid. (wrong usage)");
			return -1;
		}
		DKArray<DKVertexBuffer::Decl> declArray;

		PyObject* decl_args = PyTuple_New(0);
		DCObjectRelease decl_args_(decl_args);
		char* decl_kwlist[] = { "id", "name", "type", "normalize", "offset", NULL };

		Py_ssize_t len = PySequence_Size(decls);
		for (int i = 0; i < len; ++i)
		{
			PyObject* dict = PySequence_GetItem(decls, i);
			if (dict == NULL)
			{
				PyErr_SetString(PyExc_TypeError, "first argument must be sequence object.");
				return -1;
			}
			DCObjectRelease dict_(dict);

			if (!PyDict_Check(dict))
			{
				PyErr_SetString(PyExc_TypeError, "decl element must be dictionary object.");
				return -1;
			}

			int stream;
			const char* name;
			int type;
			int normalize;
			Py_ssize_t offset;

			if (!PyArg_ParseTupleAndKeywords(decl_args, dict, "isipn", decl_kwlist,
				&stream, &name, &type, &normalize, &offset))
			{
				PyErr_Clear();
				PyErr_SetString(PyExc_TypeError, "invalid decl data.");
				return -1;
			}

			if (stream <= DKVertexStream::StreamUnknown || stream >= DKVertexStream::StreamMax)
			{
				PyErr_SetString(PyExc_ValueError, "decl['id'] is out of range");
				return -1;
			}
			if (type <= DKVertexStream::TypeUnknown || type >= DKVertexStream::TypeMax)
			{
				PyErr_SetString(PyExc_ValueError, "decl['type'] is out of range");
				return -1;
			}
			if (offset < 0)
			{
				PyErr_SetString(PyExc_ValueError, "decl['offset'] must be greater than zero.");
				return -1;
			}

			DKVertexBuffer::Decl decl;
			decl.id = (DKVertexStream::Stream)stream;
			decl.name = name;
			decl.type = (DKVertexStream::Type)type;
			decl.normalize = normalize != 0;
			decl.offset = offset;

			declArray.Add(decl);
		}

		if (declArray.Count() == 0)
		{
			PyErr_SetString(PyExc_ValueError, "invalid vertex declarations");
			return -1;
		}

		Py_BEGIN_ALLOW_THREADS
		vb = DKVertexBuffer::Create(declArray, declArray.Count(), NULL, size, 0,
			(DKGeometryBuffer::MemoryLocation)location,
			(DKGeometryBuffer::BufferUsage)usage);
		Py_END_ALLOW_THREADS

		if (vb == NULL)
		{
			PyErr_SetString(PyExc_RuntimeError, "failed to create vertex-buffer");
			return -1;
		}
		self->buffer = vb;
		DCObjectSetAddress(self->buffer, (PyObject*)self);
	}

	DKASSERT_DEBUG(self->buffer);
	self->base.buffer = self->buffer;
	return DCGeometryBufferTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCVertexBufferDealloc(DCVertexBuffer* self)
{
	self->buffer = NULL;
	DCGeometryBufferTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCVertexDeclToDict(const DKVertexBuffer::Decl* decl)
{
	DKASSERT_DEBUG(decl);

	PyObject* declId = PyLong_FromLong((long)decl->id);
	PyObject* declName = PyUnicode_FromWideChar(decl->name, -1);
	PyObject* declType = PyLong_FromLong((long)decl->type);
	PyObject* declNorm = PyBool_FromLong(decl->normalize);
	PyObject* declOffset = PyLong_FromSize_t(decl->offset);

	PyObject* dict = PyDict_New();
	PyDict_SetItemString(dict, "id", declId);
	PyDict_SetItemString(dict, "name", declName);
	PyDict_SetItemString(dict, "type", declType);
	PyDict_SetItemString(dict, "normalize", declNorm);
	PyDict_SetItemString(dict, "offset", declOffset);

	Py_DECREF(declId);
	Py_DECREF(declName);
	Py_DECREF(declType);
	Py_DECREF(declNorm);
	Py_DECREF(declOffset);

	return dict;
}

static PyObject* DCVertexBufferDeclarationAtIndex(DCVertexBuffer* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	long index;
	if (!PyArg_ParseTuple(args, "l", &index))
		return NULL;

	size_t numDecls = self->buffer->NumberOfDeclarations();
	if (index < 0 || index >= numDecls)
	{
		PyErr_SetString(PyExc_IndexError, "index is out of range");
		return NULL;
	}
	const DKVertexBuffer::Decl* decl = self->buffer->DeclarationAtIndex(index);
	if (decl == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "failed to get vertex declaration");
		return NULL;
	}
	return DCVertexDeclToDict(decl);
}

static PyObject* DCVertexBufferDeclaration(DCVertexBuffer* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	long stream;
	const char* name = NULL;

	if (!PyArg_ParseTuple(args, "l|s", &stream, name))
		return NULL;

	if (stream <= (long)DKVertexStream::StreamUnknown || stream >= (long)DKVertexStream::StreamMax)
	{
		PyErr_SetString(PyExc_ValueError, "stream is out of range");
		return NULL;
	}
	const DKVertexBuffer::Decl* decl = NULL;
	if (name)
		decl = self->buffer->Declaration((DKVertexStream::Stream)stream, DKString(name));
	else
		decl = self->buffer->Declaration((DKVertexStream::Stream)stream);

	if (decl == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "failed to get vertex declaration");
		return NULL;
	}
	return DCVertexDeclToDict(decl);
}

static PyObject* DCVertexBufferGetStreamData(DCVertexBuffer* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	int stream;
	const char* name;
	if (!PyArg_ParseTuple(args, "is", &stream, &name))
		return NULL;

	if (stream <= 0 || stream >= DKVertexStream::StreamMax)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range");
		return NULL;
	}

	DKString str(name);
	if (self->buffer->Declaration((DKVertexStream::Stream)stream, str))
	{
		DKObject<DKBuffer> data = NULL;

		Py_BEGIN_ALLOW_THREADS
		data = self->buffer->CopyStream((DKVertexStream::Stream)stream, str);
		Py_END_ALLOW_THREADS
		if (data)
		{
			return DCDataFromObject((DKData*)data);
		}
		PyErr_SetString(PyExc_NotImplementedError, "failed to get data.");
		return NULL;
	}
	PyErr_SetString(PyExc_RuntimeError, "Stream not found");
	return NULL;
}

static PyObject* DCVertexBufferUpdateContent(DCVertexBuffer* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	PyObject* source;
	Py_ssize_t count;
	int location, usage;
	char* kwlist[] = { "source", "count", "location", "usage", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "Onii", kwlist, &source, &count, &location, &usage))
		return NULL;

	if (location < DKGeometryBuffer::MemoryLocationStatic ||
		location > DKGeometryBuffer::MemoryLocationStream)		// DKGeometryBuffer::MemoryLocation
	{
		PyErr_SetString(PyExc_ValueError, "thrid argument is invalid. (wrong location)");
		return NULL;
	}
	if (usage < DKGeometryBuffer::BufferUsageDraw ||
		usage > DKGeometryBuffer::BufferUsageCopy)	// DKGeometryBuffer::BufferUsage
	{
		PyErr_SetString(PyExc_ValueError, "fourth argument is invalid. (wrong usage)");
		return NULL;
	}

	bool result = false;

	if (source == Py_None)
	{
		if (count != 0)
		{
			PyErr_SetString(PyExc_ValueError, "second argument must be zero if first argument is None");
			return NULL;
		}
		Py_BEGIN_ALLOW_THREADS
		result = self->buffer->UpdateContent(NULL, 0,
			(DKVertexBuffer::MemoryLocation)location, (DKVertexBuffer::BufferUsage)usage);
		Py_END_ALLOW_THREADS
	}
	else if (PyObject_CheckBuffer(source))
	{
		Py_buffer buffer;
		if (PyObject_GetBuffer(source, &buffer, PyBUF_SIMPLE) == 0)
		{
			DCBufferRelease _tmp[] = { &buffer };

			size_t vertexSize = self->buffer->VertexSize();
			size_t numVerts = buffer.len / vertexSize;
			if (count > numVerts || count < 0)
			{
				PyErr_SetString(PyExc_ValueError, "second argument is out of range");
				return NULL;
			}
			Py_BEGIN_ALLOW_THREADS
			result = self->buffer->UpdateContent(buffer.buf, count,
				(DKVertexBuffer::MemoryLocation)location, (DKVertexBuffer::BufferUsage)usage);
			Py_END_ALLOW_THREADS
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "first argument must be buffer object or None.");
		return NULL;
	}

	if (!result)
	{
		PyErr_SetString(PyExc_RuntimeError, "updateContent failed");
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject* DCVertexBufferIsValid(DCVertexBuffer* self, PyObject*)
{
	if (self->buffer && self->buffer->IsValid())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "declarationAtIndex", (PyCFunction)&DCVertexBufferDeclarationAtIndex, METH_VARARGS },
	{ "declaration", (PyCFunction)&DCVertexBufferDeclaration, METH_VARARGS },
	{ "getStreamData", (PyCFunction)&DCVertexBufferGetStreamData, METH_VARARGS },
	{ "updateContent", (PyCFunction)&DCVertexBufferUpdateContent, METH_VARARGS | METH_KEYWORDS },
	{ "isValid", (PyCFunction)&DCVertexBufferIsValid, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCVertexBufferNumberOfDeclarations(DCVertexBuffer* self, void*)
{
	size_t s = 0;
	if (self->buffer)
		s = self->buffer->NumberOfDeclarations();
	return PyLong_FromSize_t(s);
}

static PyObject* DCVertexBufferNumberOfVertices(DCVertexBuffer* self, void*)
{
	size_t s = 0;
	if (self->buffer)
		s = self->buffer->NumberOfVertices();
	return PyLong_FromSize_t(s);
}

static PyObject* DCVertexBufferVertexSize(DCVertexBuffer* self, void*)
{
	size_t s = 0;
	if (self->buffer)
		s = self->buffer->VertexSize();
	return PyLong_FromSize_t(s);
}

static PyGetSetDef getsets[] = {
	{ "numberOfDeclarations", (getter)&DCVertexBufferNumberOfDeclarations, 0, 0, 0 },
	{ "numberOfVertices", (getter)&DCVertexBufferNumberOfVertices, 0, 0, 0 },
	{ "vertexSize", (getter)&DCVertexBufferVertexSize, 0, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".VertexBuffer",			/* tp_name */
	sizeof(DCVertexBuffer),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCVertexBufferDealloc,			/* tp_dealloc */
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
	(initproc)&DCVertexBufferInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCVertexBufferNew,								/* tp_new */
};

PyTypeObject* DCVertexBufferTypeObject(void)
{
	return &objectType;
}

PyObject* DCVertexBufferFromObject(DKVertexBuffer* buffer)
{
	if (buffer)
	{
		DCVertexBuffer* self = (DCVertexBuffer*)DCObjectFromAddress(buffer);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCVertexBuffer*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKVertexBuffer* DCVertexBufferToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCVertexBuffer*)obj)->buffer;
	}
	return NULL;
}
