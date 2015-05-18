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
	// Data()    빈 객체 생성
	// Data(source=obj) buffer 객체로부터 복사
	// Data(source=string) string(url) 로부터 다운로드
	// Data(filemap=string, writable=False) file map 열기 (기존 파일)
	// Data(filemap=string, length=int, overwrite=False) file map 열기, (새파일 생성)
	// Data(length=int) 빈 데이터 생성 (항상 writable)

	// buffer, url, (filemap or length) 는 동시에 사용할 수 없음.
	// writable 은 filemap 하고 같이 사용할 수 있음. 혼자서는 못씀.
	// overwrite 는 filemap 하고 같이 사용할 수 있음. 혼자서는 못씀.
	// length 는 혼자 또는 filemap (+ overwrite) 하고 같이 쓸수 있음.


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

	// 'length' 데이터 가져오기. length > 0 이면 ok, 그외엔 모두 오류
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

	if (source)		// buffer object 로부터 데이터 복사 또는 URL 다운로드
	{
		if (PyUnicode_Check(source))		// url 로부터 다운로드
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
		else if (PyObject_TypeCheck(source, DCStreamTypeObject()))	// 스트림으로부터 복사
		{
			DKStream* s = DCStreamToObject(source);
			// 스트림이 파일에서 읽어오는 경우, 오래걸릴수 있다.
			Py_BEGIN_ALLOW_THREADS
			self->data = DKBuffer::Create(s);
			Py_END_ALLOW_THREADS
		}
		else if (PyObject_CheckBuffer(source))	// buffer 복사
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
	else if (filemap)		// DKFileMap 객체로 파일 매핑
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
		if (length)			// 새 파일 생성 DKFileMap::Create
		{
			bool over = false;
			if (!getBool(overwrite, over))
			{
				PyErr_SetString(PyExc_TypeError, "argument 'overwrite' must be True or False");
				return -1;
			}
			// 디스크가 스핀다운 되었을 경우 오래 걸릴 수 있음.
			Py_BEGIN_ALLOW_THREADS
			fm = DKFileMap::Create(filemap, dataLength, over);
			Py_END_ALLOW_THREADS
		}
		else				// 기존 파일 열기 DKFileMap::Open
		{
			bool wt = false;
			if (!getBool(writable, wt))
			{
				PyErr_SetString(PyExc_TypeError, "argument 'writable' must be True or False");
				return -1;
			}
			// 디스크가 스핀다운 되었을 경우 오래 걸릴 수 있음.
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
	if (dataLength > 0)		// 빈 버퍼 생성 (0 으로 초기화)
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
