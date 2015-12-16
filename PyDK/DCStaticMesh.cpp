#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCStaticMesh.h"
#include "DCObject.h"

void DCStaticMesh::UpdateVertexBuffers(void)
{
	size_t numVB = this->mesh->NumberOfVertexBuffers();
	PyObject* tuple = PyTuple_New(numVB);
	for (int i = 0; i < numVB; ++i)
	{
		DKVertexBuffer* buffer = this->mesh->VertexBufferAtIndex(i);
		DKASSERT_DEBUG(buffer);
		PyObject* obj = DCVertexBufferFromObject(buffer);
		DKASSERT_DEBUG(DCVertexBufferToObject(obj) == buffer);
		PyTuple_SET_ITEM(tuple, i, obj);
	}
	Py_CLEAR(this->vertexBuffers);
	this->vertexBuffers = tuple;
}

void DCStaticMesh::UpdateIndexBuffer(void)
{
	PyObject* old = this->indexBuffer;
	this->indexBuffer = NULL;
	DKIndexBuffer* buffer = this->mesh->IndexBuffer();
	if (buffer)
	{
		this->indexBuffer = DCIndexBufferFromObject(buffer);
		DKASSERT_DEBUG(DCIndexBufferToObject(this->indexBuffer) == buffer);
	}
	Py_XDECREF(old);
}

static PyObject* DCStaticMeshNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCStaticMesh* self = (DCStaticMesh*)DCMeshTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->mesh = NULL;
		self->vertexBuffers = NULL;
		self->indexBuffer = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCStaticMeshInit(DCStaticMesh *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKStaticMesh> mesh = NULL;
	if (self->mesh == NULL)
	{
		mesh = DKOBJECT_NEW DKStaticMesh();
		self->mesh = mesh;
		DCObjectSetAddress(self->mesh, (PyObject*)self);
	}
	else
	{
		self->UpdateIndexBuffer();
		self->UpdateVertexBuffers();
	}

	self->base.mesh = self->mesh;
	return DCMeshTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static int DCStaticMeshClear(DCStaticMesh* self)
{
	Py_CLEAR(self->indexBuffer);
	Py_CLEAR(self->vertexBuffers);
	return DCMeshTypeObject()->tp_clear((PyObject*)self);
}

static int DCStaticMeshTraverse(DCStaticMesh* self, visitproc visit, void* arg)
{
	Py_VISIT(self->indexBuffer);
	Py_VISIT(self->vertexBuffers);
	return DCMeshTypeObject()->tp_traverse((PyObject*)self, visit, arg);
}

static void DCStaticMeshDealloc(DCStaticMesh* self)
{
	self->mesh = NULL;
	DCMeshTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCStaticMeshAddVertexBuffer(DCStaticMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	PyObject* bufferObj;
	if (!PyArg_ParseTuple(args, "O", &bufferObj))
		return NULL;

	DKVertexBuffer* vb = DCVertexBufferToObject(bufferObj);
	if (vb == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be VertexBuffer object.");
		return NULL;
	}
	if (!self->mesh->AddVertexBuffer(vb))
	{
		PyErr_SetString(PyExc_ValueError, "Cannot add VertexBuffer object. (Object may incompatible)");
	}
	self->UpdateVertexBuffers();
	Py_RETURN_NONE;
}

static PyObject* DCStaticMeshNumberOfVertexBuffers(DCStaticMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	return PyLong_FromSize_t(self->mesh->NumberOfVertexBuffers());
}

static PyObject* DCStaticMeshVertexBufferAtIndex(DCStaticMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	int index = 0;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	Py_ssize_t numVbs = self->mesh->NumberOfVertexBuffers();
	if (index < 0 || index >= numVbs)
	{
		PyErr_SetString(PyExc_IndexError, "index is out of range.");
		return NULL;
	}
	DKVertexBuffer* vb = self->mesh->VertexBufferAtIndex(index);
	DKASSERT_DEBUG(vb);

	PyObject* obj = DCVertexBufferFromObject(vb);
	DKASSERT_DEBUG(DCVertexBufferToObject(obj) == vb);
	return obj;
}

static PyObject* DCStaticMeshRemoveVertexBuffer(DCStaticMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKVertexBuffer* vb = DCVertexBufferToObject(obj);
	if (vb == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be VertexBuffer object.");
		return NULL;
	}
	self->mesh->RemoveVertexBuffer(vb);
	self->UpdateVertexBuffers();
	Py_RETURN_NONE;
}

static PyObject* DCStaticMeshRemoveAllVertexBuffers(DCStaticMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	self->mesh->RemoveAllVertexBuffers();
	self->UpdateVertexBuffers();
	Py_RETURN_NONE;
}

static PyObject* DCStaticMeshIndexBuffer(DCStaticMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKIndexBuffer* ib = self->mesh->IndexBuffer();
	if (ib)
	{
		PyObject* obj = DCIndexBufferFromObject(ib);
		DKASSERT_DEBUG(DCIndexBufferToObject(obj) == ib);
		return obj;
	}
	Py_RETURN_NONE;
}

static PyObject* DCStaticMeshSetIndexBuffer(DCStaticMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	if (obj == Py_None)
	{
		self->mesh->SetIndexBuffer(NULL);
	}
	else
	{
		DKIndexBuffer* ib = DCIndexBufferToObject(obj);
		if (ib == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument must be IndexBuffer object.");
			return NULL;
		}
		self->mesh->SetIndexBuffer(ib);
	}
	self->UpdateIndexBuffer();
	Py_RETURN_NONE;
}

static PyObject* DCVertexDeclToDict(const DKVertexBuffer::Decl* decl)
{
	DKASSERT_DEBUG(decl);

	PyObject* declId = PyLong_FromLong((long)decl->id);
	PyObject* declName = PyUnicode_FromWideChar(decl->name, -1);
	PyObject* declType = PyLong_FromLong((long)decl->type);
	PyObject* declNorm = PyBool_FromLong(decl->normalize);

	PyObject* dict = PyDict_New();
	PyDict_SetItemString(dict, "id", declId);
	PyDict_SetItemString(dict, "name", declName);
	PyDict_SetItemString(dict, "type", declType);
	PyDict_SetItemString(dict, "normalize", declNorm);

	Py_DECREF(declId);
	Py_DECREF(declName);
	Py_DECREF(declType);
	Py_DECREF(declNorm);

	return dict;
}

static PyObject* DCStaticMeshFindVertexStream(DCStaticMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	long stream;
	const char* name = NULL;

	if (!PyArg_ParseTuple(args, "l|s", &stream, name))
		return NULL;

	if (stream <= (long)DKVertexStream::StreamUnknown || stream >= (long)DKVertexStream::StreamMax)
	{
		PyErr_SetString(PyExc_ValueError, "stream is out of range");
		return NULL;
	}
	DKStaticMesh::StreamInfo si = { NULL, NULL, 0 };
	if (name)
		si = self->mesh->FindVertexStream((DKVertexStream::Stream)stream, DKString(name));
	else
		si = self->mesh->FindVertexStream((DKVertexStream::Stream)stream);

	if (si.decl)
	{
		DKASSERT_DEBUG(si.buffer);
		PyObject* bufferObj = DCVertexBufferFromObject(const_cast<DKVertexBuffer*>(si.buffer));
		DKASSERT_DEBUG(DCVertexBufferToObject(bufferObj) == si.buffer);

		PyObject* dict = DCVertexDeclToDict(si.decl);
		DKASSERT_DEBUG(PyDict_Check(dict));
		PyDict_SetItemString(dict, "buffer", bufferObj);
		Py_DECREF(bufferObj);

		return dict;
	}
	PyErr_SetString(PyExc_ValueError, "failed to get vertex declaration");
	return NULL;
}

static PyObject* DCStaticMeshMakeInterleaved(DCStaticMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	int location = (int)DKVertexBuffer::MemoryLocationStatic;
	int usage = (int)DKVertexBuffer::BufferUsageDraw;
	if (!PyArg_ParseTuple(args, "ii", &location, &usage))
		return NULL;
	
	// DKGeometryBuffer::MemoryLocation
	if (location < DKGeometryBuffer::MemoryLocationStatic || location > DKGeometryBuffer::MemoryLocationStream)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid. (wrong location)");
		return NULL;
	}
	// DKGeometryBuffer::BufferUsage
	if (usage < DKGeometryBuffer::BufferUsageDraw || usage > DKGeometryBuffer::BufferUsageCopy)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is invalid. (wrong usage)");
		return NULL;
	}
	if (!self->mesh->MakeInterleaved())
	{
		PyErr_SetString(PyExc_RuntimeError, "DKStaticMesh::MakeInterleaved() failed.");
	}
	self->UpdateVertexBuffers();
	Py_RETURN_NONE;
}

static PyObject* DCStaticMeshMakeSeparated(DCStaticMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	int location = (int)DKVertexBuffer::MemoryLocationStatic;
	int usage = (int)DKVertexBuffer::BufferUsageDraw;
	if (!PyArg_ParseTuple(args, "ii", &location, &usage))
		return NULL;
	
	// DKGeometryBuffer::MemoryLocation
	if (location < DKGeometryBuffer::MemoryLocationStatic || location > DKGeometryBuffer::MemoryLocationStream)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid. (wrong location)");
		return NULL;
	}
	// DKGeometryBuffer::BufferUsage
	if (usage < DKGeometryBuffer::BufferUsageDraw || usage > DKGeometryBuffer::BufferUsageCopy)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is invalid. (wrong usage)");
		return NULL;
	}
	if (!self->mesh->MakeSeparated())
	{
		PyErr_SetString(PyExc_RuntimeError, "DKStaticMesh::MakeSeparated() failed.");
	}
	self->UpdateVertexBuffers();
	Py_RETURN_NONE;
}

static PyObject* DCStaticMeshUpdateStream(DCStaticMesh* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	int stream, type, normalize;
	Py_ssize_t size, count;
	Py_buffer data;
	const char* name;
	int location = (int)DKVertexBuffer::MemoryLocationStatic;
	int usage = (int)DKVertexBuffer::BufferUsageDraw;
	char* kwlist[] = { "stream", "name", "type", "normalize", "size", "count", "data", "location", "usage", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "isipnny*|ii", kwlist,
		&stream, &name, &type, &normalize, &size, &count, &data, &location, &usage))
		return NULL;

	DCBufferRelease _tmp[] = { &data };		// release buffer when return

	if (stream <= DKVertexStream::StreamUnknown || stream >= DKVertexStream::StreamMax)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range");
		return NULL;
	}
	if (type <= DKVertexStream::TypeUnknown || type >= DKVertexStream::TypeMax)
	{
		PyErr_SetString(PyExc_ValueError, "third argument is out of range");
		return NULL;
	}
	if (size <= 0)
	{
		PyErr_SetString(PyExc_ValueError, "fifth argument must be positive integer.");
		return NULL;
	}
	if (count <= 0)
	{
		PyErr_SetString(PyExc_ValueError, "sixth argument must be positive integer.");
		return NULL;
	}
	if (location < DKGeometryBuffer::MemoryLocationStatic || location > DKGeometryBuffer::MemoryLocationStream)
	{
		PyErr_SetString(PyExc_ValueError, "eighth argument is invalid. (wrong location)");
		return NULL;
	}
	if (usage < DKGeometryBuffer::BufferUsageDraw || usage > DKGeometryBuffer::BufferUsageCopy)
	{
		PyErr_SetString(PyExc_ValueError, "ninth argument is invalid. (wrong usage)");
		return NULL;
	}

	size_t bufferLenReq = size * count;
	if (bufferLenReq > data.len)
	{
		PyErr_Format(PyExc_ValueError, "seventh argument (data) is too samll. it should be %zu bytes at least.", bufferLenReq);
		return NULL;
	}

	if (!self->mesh->UpdateStream((DKVertexStream::Stream)stream, DKString(name),
		(DKVertexStream::Type)type, normalize != 0, size, count, data.buf,
		(DKGeometryBuffer::MemoryLocation)location,
		(DKGeometryBuffer::BufferUsage)usage))
	{
		PyErr_SetString(PyExc_RuntimeError, "UpdateStream failed.");
		return NULL;
	}
	self->UpdateVertexBuffers();
	Py_RETURN_NONE;
}

static PyObject* DCStaticMeshRemoveStream(DCStaticMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	int stream;
	const char* name;
	if (!PyArg_ParseTuple(args, "is", &stream, &name))
		return NULL;
	if (stream <= DKVertexStream::StreamUnknown || stream >= DKVertexStream::StreamMax)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range");
		return NULL;
	}
	if (!self->mesh->RemoveStream((DKVertexStream::Stream)stream, DKString(name)))
	{
		PyErr_SetString(PyExc_RuntimeError, "RemoveStream failed.");
		return NULL;
	}
	self->UpdateVertexBuffers();
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "addVertexBuffer", (PyCFunction)&DCStaticMeshAddVertexBuffer, METH_VARARGS },
	{ "numberOfVertexBuffers", (PyCFunction)&DCStaticMeshNumberOfVertexBuffers, METH_NOARGS },
	{ "vertexBufferAtIndex", (PyCFunction)&DCStaticMeshVertexBufferAtIndex, METH_VARARGS },
	{ "removeVertexBuffer", (PyCFunction)&DCStaticMeshRemoveVertexBuffer, METH_VARARGS },
	{ "removeAllVertexBuffers", (PyCFunction)&DCStaticMeshRemoveAllVertexBuffers, METH_NOARGS },
	{ "indexBuffer", (PyCFunction)&DCStaticMeshIndexBuffer, METH_NOARGS },
	{ "setIndexBuffer", (PyCFunction)&DCStaticMeshSetIndexBuffer, METH_VARARGS },
	{ "findVertexStream", (PyCFunction)&DCStaticMeshFindVertexStream, METH_VARARGS },
	{ "makeInterleaved", (PyCFunction)&DCStaticMeshMakeInterleaved, METH_VARARGS },
	{ "makeSeparated", (PyCFunction)&DCStaticMeshMakeSeparated, METH_VARARGS },
	{ "updateStream", (PyCFunction)&DCStaticMeshUpdateStream, METH_VARARGS | METH_KEYWORDS },
	{ "removeStream", (PyCFunction)&DCStaticMeshRemoveStream, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".StaticMesh",				/* tp_name */
	sizeof(DCStaticMesh),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCStaticMeshDealloc,			/* tp_dealloc */
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
	0,											/* tp_as_mesh */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCStaticMeshTraverse,		/* tp_traverse */
	(inquiry)&DCStaticMeshClear,				/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	0,											/* tp_getset */
	DCMeshTypeObject(),							/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCStaticMeshInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCStaticMeshNew,							/* tp_new */
};

PyTypeObject* DCStaticMeshTypeObject(void)
{
	return &objectType;
}

PyObject* DCStaticMeshFromObject(DKStaticMesh* mesh)
{
	if (mesh)
	{
		DCStaticMesh* self = (DCStaticMesh*)DCObjectFromAddress(mesh);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(SkinMesh, mesh);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCStaticMesh*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->mesh = mesh;
				DCObjectSetAddress(self->mesh, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKStaticMesh* DCStaticMeshToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCStaticMesh*)obj)->mesh;
	}
	return NULL;
}
