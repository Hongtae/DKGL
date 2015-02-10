#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCStream
{
	PyObject_HEAD
	DKObject<DKStream> stream;
};

static PyObject* DCStreamNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCStream* self = (DCStream*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->stream) DKObject<DKStream>();
		self->stream = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCStreamInit(DCStream *self, PyObject *args, PyObject *kwds)
{
	if (self->stream)
		return 0;

	const char* path;
	int openMode = 0;
	int shareMode = 0;

	char* kwlist[] = { "file", "mode", "share", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "sii", kwlist, &path, &openMode, &shareMode))
		return -1;

	if (openMode < DKFile::ModeOpenNew || openMode > DKFile::ModeOpenAlways)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is invalid.");
		return -1;
	}
	if (shareMode < DKFile::ModeShareAll || shareMode > DKFile::ModeShareExclusive)
	{
		PyErr_SetString(PyExc_ValueError, "third argument is invalid.");
		return -1;
	}

	DKObject<DKFile> file = NULL;
	Py_BEGIN_ALLOW_THREADS
	file = DKFile::Create(path, (DKFile::ModeOpen)openMode, (DKFile::ModeShare)shareMode);
	Py_END_ALLOW_THREADS

	if (file)
	{
		self->stream = file;
		DCObjectSetAddress(self->stream, (PyObject*)self);
		return 0;
	}
	PyErr_SetString(PyExc_IOError, "Cannot open file");
	return -1;
}

static void DCStreamDealloc(DCStream* self)
{
	if (self->stream)
	{
		DCObjectSetAddress(self->stream, NULL);
		self->stream = NULL;
	}
	self->stream.~DKObject<DKStream>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCStreamRead(DCStream* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->stream, NULL);

	if (!self->stream->IsReadable())
	{
		PyErr_SetString(PyExc_IOError, "object is not readable.");
		return NULL;
	}

	Py_ssize_t bytesToRead = 0;
	if (!PyArg_ParseTuple(args, "n", &bytesToRead))
		return NULL;

	if (bytesToRead > 0)
	{
		PyObject* bytesObj = PyBytes_FromStringAndSize(NULL, bytesToRead);
		if (bytesObj == NULL)
			return NULL;

		DKASSERT_DEBUG(PyBytes_Check(bytesObj));
		void* buff = PyBytes_AS_STRING(bytesObj);
		bytesToRead = PyBytes_GET_SIZE(bytesObj);

		size_t numRead = 0;
		Py_BEGIN_ALLOW_THREADS
		numRead = self->stream->Read(buff, bytesToRead);
		Py_END_ALLOW_THREADS

		if (numRead > bytesToRead)
		{
			Py_DECREF(bytesObj);
			PyErr_SetString(PyExc_IOError, "Operation failed");
			return NULL;
		}
		else if (numRead != bytesToRead)
		{
			if (_PyBytes_Resize(&bytesObj, numRead) < 0)
			{
				Py_DECREF(bytesObj);
				return NULL;
			}
		}

		return bytesObj;
	}
	return PyBytes_FromStringAndSize(0,0);
}

static PyObject* DCStreamWrite(DCStream* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->stream, NULL);

	if (!self->stream->IsWritable())
	{
		PyErr_SetString(PyExc_IOError, "object is not writable.");
		return NULL;
	}

	Py_buffer buffer;
	PY_LONG_LONG length = -1;
	if (!PyArg_ParseTuple(args, "y*|L", &buffer, &length))
		return NULL;

	size_t written = 0;
	size_t len = buffer.len;
	if (length >= 0)
		len = Min<size_t>(buffer.len, length);

	if (len > 0)
	{
		Py_BEGIN_ALLOW_THREADS
		written = self->stream->Write(buffer.buf, len);
		Py_END_ALLOW_THREADS
	}

	PyBuffer_Release(&buffer);

	if (written > len)
	{
		PyErr_SetString(PyExc_IOError, "operation failed");
		return NULL;
	}

	return PyLong_FromSize_t(written);
}

static PyObject* DCStreamDataObject(DCStream* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->stream, NULL);

	DKObject<DKBuffer> buffer = NULL;
	Py_BEGIN_ALLOW_THREADS
	buffer = DKBuffer::Create(self->stream);
	Py_END_ALLOW_THREADS

	if (buffer)
		return DCDataFromObject(buffer);

	PyErr_SetString(PyExc_RuntimeError, "Internal Error.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "read", (PyCFunction)&DCStreamRead, METH_VARARGS },
	{ "write", (PyCFunction)&DCStreamWrite, METH_VARARGS },
	{ "dataObject", (PyCFunction)&DCStreamDataObject, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCStreamPosition(DCStream* self, void*)
{
	DKStream::Position pos = 0;
	if (self->stream)
		pos = self->stream->GetPos();
	return PyLong_FromLongLong(pos);
}

static int DCStreamSetPosition(DCStream* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->stream, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (!PyLong_Check(value))
	{
		PyErr_SetString(PyExc_TypeError, "attribute must be integer.");
		return -1;
	}
	PY_LONG_LONG pos = PyLong_AsLongLong(value);
	if (PyErr_Occurred())
		return -1;

	if (pos < 0)
	{
		PyErr_SetString(PyExc_ValueError, "attribute must be positive integer.");
		return -1;
	}

	DKStream::Position p = self->stream->SetPos(pos);
	return 0;
}

static PyObject* DCStreamRemainLength(DCStream* self, void*)
{
	DKStream::Position len = 0;
	if (self->stream)
		len = self->stream->RemainLength();
	return PyLong_FromLongLong(len);
}

static PyObject* DCStreamTotalLength(DCStream* self, void*)
{
	DKStream::Position len = 0;
	if (self->stream)
		len = self->stream->TotalLength();
	return PyLong_FromLongLong(len);
}

static PyObject* DCStreamReadable(DCStream* self, void*)
{
	if (self->stream && self->stream->IsReadable())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCStreamWritable(DCStream* self, void*)
{
	if (self->stream && self->stream->IsWritable())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCStreamSeekable(DCStream* self, void*)
{
	if (self->stream && self->stream->IsSeekable())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyGetSetDef getsets[] = {
	{ "position", (getter)&DCStreamPosition, (setter)DCStreamSetPosition, 0, NULL },
	{ "remainLength", (getter)&DCStreamRemainLength, (setter)NULL, 0, NULL },
	{ "totalLength", (getter)&DCStreamTotalLength, (setter)NULL, 0, NULL },
	{ "readable", (getter)&DCStreamReadable, (setter)NULL, 0, NULL },
	{ "writable", (getter)&DCStreamWritable, (setter)NULL, 0, NULL },
	{ "seekable", (getter)&DCStreamSeekable, (setter)NULL, 0, NULL },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Stream",					/* tp_name */
	sizeof(DCStream),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCStreamDealloc,				/* tp_dealloc */
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
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCStreamInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCStreamNew,								/* tp_new */
};

PyTypeObject* DCStreamTypeObject(void)
{
	return &objectType;
}

PyObject* DCStreamFromObject(DKStream* stream)
{
	if (stream)
	{
		DCStream* self = (DCStream*)DCObjectFromAddress(stream);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCStream*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->stream = stream;
				DCObjectSetAddress(self->stream, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKStream* DCStreamToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCStream*)obj)->stream;
	}
	return NULL;
}
