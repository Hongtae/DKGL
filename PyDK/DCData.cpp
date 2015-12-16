#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCData
{
	PyObject_HEAD
	unsigned int sharedLocks;
	unsigned int exclusiveLocks;
	void* exclusivePtr;
	DKThread::ThreadId exclusiveOwner;
	DKObject<DKData> data;
};

static PyObject* DCDataNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCData* self = (DCData*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->data) DKObject<DKData>();
		self->sharedLocks = 0;
		self->exclusiveLocks = 0;
		self->exclusivePtr = NULL;
		self->exclusiveOwner = DKThread::invalidId;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCDataInit(DCData *self, PyObject *args, PyObject *kwds)
{
	// Data() create empty object.
	// Data(source=obj) copy data from obj. (obj should be compliance with buffer-protocol)
	// Data(source=string) copy data from url-string
	// Data(filemap=string, writable=False) open file-map (file must be exists)
	// Data(filemap=string, length=int, overwrite=False) create new file and map file.
	// Data(length=int) create buffer object with given length. always writable.

	// buffer, url, (filemap or length) cannot be combined together.
	// 'writable' parameter should be combined with 'filemap'.
	// 'overwrite' parameter should be combined with 'filemap'.
	// 'length' parameter can be alone or combined with 'filemap'+'overwrite'.

	if (self->data)
		return 0;

	PyObject* source = NULL;
	const char* filemap = NULL;
	PyObject* writable = NULL;
	PyObject* length = NULL;
	PyObject* overwrite = NULL;

	char* kwlist[] = { "source", "filemap", "writable", "length", "overwrite", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|$OsOOO", kwlist, &source, &filemap, &writable, &length, &overwrite))
		return -1;

	const void* tmp[] = { (const void*)source, filemap, writable, length, overwrite };
	int numArgs = 0;
	for (const void* p : tmp) { if (p) numArgs++; }

	if ((numArgs < 2)  ||
		(numArgs == 2 && filemap && writable) ||
		(numArgs == 2 && filemap && length) ||
		(numArgs == 3 && filemap && length && overwrite))
	{
		if (numArgs == 1 && writable)
		{
			PyErr_SetString(PyExc_TypeError, "argument 'writable' must combine with argument 'filemap'");
			return -1;
		}
		if (numArgs == 1 && overwrite)
		{
			PyErr_SetString(PyExc_TypeError, "argument 'overwrite' must combine with argument 'filemap' and 'length'");
			return -1;
		}
	}
	else
	{
		if (filemap)
			PyErr_SetString(PyExc_TypeError, "argument 'filemap' must combine with 'writable' or 'length'");
		else
			PyErr_SetString(PyExc_TypeError, "argument must be single except 'filemap'.");
		return -1;
	}

	// get 'length', if length > 0 is OK, otherwise error!
	PY_LONG_LONG dataLength = 0;
	if (length)
	{
		if (PyNumber_Check(length))
		{
			dataLength = PyLong_AsLongLong(length);
			if (PyErr_Occurred())
				return -1;
		}

		if (dataLength <= 0)
		{
			PyErr_SetString(PyExc_TypeError, "argument 'length' must be positive integer");
			return -1;
		}
	}

	if (source) // copy from buffer object or download from URL.
	{
		if (PyUnicode_Check(source))		// download from URL.
		{
			const char* url = PyUnicode_AsUTF8(source);
			if (url && url[0])
			{
				DKObject<DKBuffer> buffer = NULL;
				
				Py_BEGIN_ALLOW_THREADS
				buffer = DKBuffer::Create(DKString(url));
				Py_END_ALLOW_THREADS

				if (buffer)
					self->data = buffer.SafeCast<DKData>();
			}
		}
		else if (PyObject_TypeCheck(source, DCStreamTypeObject()))	// copy from stream.
		{
			DKStream* s = DCStreamToObject(source);
			// if stream is file, it can takes long time.
			Py_BEGIN_ALLOW_THREADS
			self->data = DKBuffer::Create(s);
			Py_END_ALLOW_THREADS
		}
		else if (PyObject_CheckBuffer(source))	// copy buffer
		{
			Py_buffer view;
			if (PyObject_GetBuffer(source, &view, PyBUF_SIMPLE) != 0)
				return -1;

			if (view.len > 0)
			{
				Py_BEGIN_ALLOW_THREADS
				self->data = DKBuffer::Create(view.buf, view.len);
				Py_END_ALLOW_THREADS
			}
			PyBuffer_Release(&view);
		}
		if (self->data == NULL)
		{
			PyErr_SetString(PyExc_RuntimeError, "cannot retrieve data from source.");
			return -1;
		}
	}
	else if (filemap)		// map file with DKFileMap object.
	{
		auto getBool = [](PyObject* obj, bool& output)->bool
		{
			if (obj)
			{
				int n = PyObject_IsTrue(obj);
				if (n < 0)
					return false;
				output = n > 0;
			}
			return true;
		};

		DKObject<DKFileMap> fm = NULL;
		if (length)			// create new file and map with DKFileMap::Create()
		{
			bool over = false;
			if (!getBool(overwrite, over))
			{
				PyErr_SetString(PyExc_TypeError, "argument 'overwrite' must be True or False");
				return -1;
			}
			// It can takes long time if a disk spun down.
			Py_BEGIN_ALLOW_THREADS
			fm = DKFileMap::Create(filemap, dataLength, over);
			Py_END_ALLOW_THREADS
		}
		else				// open existing file with DKFileMap::Open()
		{
			bool wt = false;
			if (!getBool(writable, wt))
			{
				PyErr_SetString(PyExc_TypeError, "argument 'writable' must be True or False");
				return -1;
			}
			// It can takes long time if a disk spun down.
			Py_BEGIN_ALLOW_THREADS
			fm = DKFileMap::Open(filemap, 0, wt);
			Py_END_ALLOW_THREADS
		}
		if (fm)
			self->data = fm.SafeCast<DKData>();
		else
		{
			PyErr_SetString(PyExc_RuntimeError, "cannot mapping file with attribute 'filemap'");
			return -1;
		}
	}
	if (dataLength > 0)		// create buffer object contains zero-filled buffer with given length.
	{
		self->data = DKBuffer::Create((const void*)NULL, dataLength);
	}

	if (self->data)
	{
		DCObjectSetAddress(self->data, (PyObject*)self);
	}
	self->sharedLocks = 0;
	self->exclusiveLocks = 0;
	self->exclusivePtr = NULL;
	self->exclusiveOwner = DKThread::invalidId;

	return 0;
}

static void DCDataDealloc(DCData* self)
{
	if (self->data)
	{
		DCObjectSetAddress(self->data, NULL);
		self->data = NULL;
	}
	self->data.~DKObject<DKData>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static int DCDataGetBuffer(DCData* self, Py_buffer* view, int flags)
{
	DKASSERT_DEBUG(view);
	view->obj = NULL;

	if (self->data)
	{
		void* buf = NULL;
		int readonly = 0;

		DKThread::ThreadId currentThread = DKThread::CurrentThreadId();

		if (flags & PyBUF_WRITABLE)
		{
			if (self->data->IsWritable())
			{
				if (self->exclusiveLocks > 0 && self->exclusiveOwner == currentThread)
				{
					buf = self->exclusivePtr;
					self->exclusiveLocks++;
				}
				else
				{
					DKLOG_DEBUG("DKData(%x) trying to lock (exclusive) with thread:%x\n", currentThread);
					buf = self->data->LockExclusive();
					self->exclusiveLocks++;
					self->exclusivePtr = buf;
					self->exclusiveOwner = DKThread::CurrentThreadId();
				}
			}
		}
		if (buf == NULL)
		{
			DKLOG_DEBUG("DKData(%x) trying to lock (shared) with thread:%x\n", (void*)self->data, currentThread);
			buf = (void*)self->data->LockShared();
			self->sharedLocks++;
			readonly = 1;
		}

		if (buf == NULL)
		{
			PyErr_SetString(PyExc_BufferError, "Lock failed.");
			return -1;
		}

		if (PyBuffer_FillInfo(view, (PyObject*)self, buf, self->data->Length(), readonly, flags) < 0)
			return -1;

		return 0;
	}
	PyErr_SetString(PyExc_BufferError, "Invalid Buffer Object");
	return -1;
}

static void DCDataReleaseBuffer(DCData* self, Py_buffer* view)
{
	if (self->exclusiveLocks > 0)
	{
		DKThread::ThreadId currentThread = DKThread::CurrentThreadId();
		if (self->exclusiveOwner == currentThread)
		{
			self->exclusiveLocks--;
			if (self->exclusiveLocks == 0)
			{
				DKLOG_DEBUG("DKData(%x) unlock (exclusively) with thread:%x\n", (void*)self->data, currentThread);
				self->data->UnlockExclusive();
				self->exclusivePtr = NULL;
				self->exclusiveOwner = DKThread::invalidId;
			}
		}
		else
		{
			PyErr_SetString(PyExc_BufferError, "Cannot release lock! (this is not owner thread)");
		}
	}
	else if (self->sharedLocks > 0)
	{
		DKThread::ThreadId currentThread = DKThread::CurrentThreadId();
		DKLOG_DEBUG("DKData(%x) unlock (shared) with thread:%x\n", (void*)self->data, currentThread);
		self->data->UnlockShared();
		self->sharedLocks--;
	}
}

static PyBufferProcs bufferProcs[] = {
	(getbufferproc)DCDataGetBuffer,
	(releasebufferproc)DCDataReleaseBuffer,
};

static PyObject* DCDataWriteToFile(DCData* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->data, NULL);

	const char* filename = NULL;
	int overwrite = 1;

	char* kwlist[] = { "filename", "overwrite", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|p", kwlist, &filename, &overwrite))
		return NULL;

	self->data->WriteToFile(filename, overwrite != 0);
	return PyLong_FromSize_t(self->data->Length());
}

static PyObject* DCDataStreamObject(DCData* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->data, NULL);

	DKObject<DKDataStream> ds = DKOBJECT_NEW DKDataStream(self->data);
	return DCStreamFromObject(ds.SafeCast<DKStream>());
}

static PyMethodDef methods[] = {
	{ "writeToFile", (PyCFunction)&DCDataWriteToFile, METH_VARARGS|METH_KEYWORDS, "write to file" },
	{ "streamObject", (PyCFunction)&DCDataStreamObject, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCDataLength(DCData* self, void*)
{
	size_t len = 0;
	if (self->data)
		len = self->data->Length();
	return PyLong_FromSize_t(len);
}

static PyObject* DCDataReadable(DCData* self, void*)
{
	if (self->data && self->data->IsReadable())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCDataWritable(DCData* self, void*)
{
	if (self->data && self->data->IsWritable())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCDataExcutable(DCData* self, void*)
{
	if (self->data && self->data->IsExcutable())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyGetSetDef getsets[] = {
	{ "length", (getter)&DCDataLength, (setter)NULL, 0, NULL },
	{ "readable", (getter)&DCDataReadable, (setter)NULL, 0, NULL },
	{ "writable", (getter)&DCDataWritable, (setter)NULL, 0, NULL },
	{ "excutable", (getter)&DCDataExcutable, (setter)NULL, 0, NULL },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Data",				/* tp_name */
	sizeof(DCData),							/* tp_basicsize */
	0,										/* tp_itemsize */
	(destructor)&DCDataDealloc,				/* tp_dealloc */
	0,										/* tp_print */
	0,										/* tp_getattr */
	0,										/* tp_setattr */
	0,										/* tp_reserved */
	0,										/* tp_repr */
	0,										/* tp_as_number */
	0,										/* tp_as_sequence */
	0,										/* tp_as_mapping */
	0,										/* tp_hash  */
	0,										/* tp_call */
	0,										/* tp_str */
	0,										/* tp_getattro */
	0,										/* tp_setattro */
	bufferProcs,							/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,					/* tp_flags */
	0,										/* tp_doc */
	0,										/* tp_traverse */
	0,										/* tp_clear */
	0,										/* tp_richcompare */
	0,										/* tp_weaklistoffset */
	0,										/* tp_iter */
	0,										/* tp_iternext */
	methods,								/* tp_methods */
	0,										/* tp_members */
	getsets,								/* tp_getset */
	0,										/* tp_base */
	0,										/* tp_dict */
	0,										/* tp_descr_get */
	0,										/* tp_descr_set */
	0,										/* tp_dictoffset */
	(initproc)&DCDataInit,					/* tp_init */
	0,										/* tp_alloc */
	&DCDataNew,								/* tp_new */
};

PyTypeObject* DCDataTypeObject(void)
{
	return &objectType;
}

PyObject* DCDataFromObject(DKData* data)
{
	if (data && data->Length() > 0)
	{
		DCData* self = (DCData*)DCObjectFromAddress(data);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCData*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->data = data;
				DCObjectSetAddress(self->data, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKData* DCDataToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCData*)obj)->data;
	}
	return NULL;
}
