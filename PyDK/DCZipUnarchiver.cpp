#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

struct DCZipUnarchiver
{
	PyObject_HEAD
	DKObject<DKZipUnarchiver> zip;
};

static PyObject* DCZipUnarchiverNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCZipUnarchiver* self = (DCZipUnarchiver*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->zip) DKObject<DKZipUnarchiver>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCZipUnarchiverInit(DCZipUnarchiver *self, PyObject *args, PyObject *kwds)
{
	if (self->zip)
		return 0;

	const char* file;
	if (!PyArg_ParseTuple(args, "s", &file))
		return -1;

	Py_BEGIN_ALLOW_THREADS
	self->zip = DKZipUnarchiver::Create(file);
	Py_END_ALLOW_THREADS

	if (self->zip)
	{
		DCObjectSetAddress(self->zip, (PyObject*)self);
		return 0;
	}
	PyErr_SetString(PyExc_IOError, "Cannot open file");
	return -1;
}

static void DCZipUnarchiverDealloc(DCZipUnarchiver* self)
{
	if (self->zip)
	{
		DCObjectSetAddress(self->zip, NULL);
		self->zip = NULL;
	}
	self->zip.~DKObject<DKZipUnarchiver>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCZipUnarchiverAllFiles(DCZipUnarchiver* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->zip, NULL);

	const DKArray<DKZipUnarchiver::FileInfo>& files = self->zip->GetFileList();

	size_t count = files.Count();
	PyObject* tuple = PyTuple_New(count);
	for (size_t i = 0; i < count; ++i)
	{
		const DKZipUnarchiver::FileInfo& fi = files.Value(i);
		PyTuple_SET_ITEM(tuple, i, PyUnicode_FromWideChar(fi.name, -1));
	}
	return tuple;
}

static PyObject* DCZipUnarchiverFileInfo(DCZipUnarchiver* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->zip, NULL);
	const char* file;
	if (!PyArg_ParseTuple(args, "s", &file))
		return NULL;

	const DKZipUnarchiver::FileInfo* fi = NULL;
	Py_BEGIN_ALLOW_THREADS
	fi = self->zip->GetFileInfo(file);
	Py_END_ALLOW_THREADS
	if (fi)
	{
		if (PyDateTimeAPI == NULL)
			PyDateTime_IMPORT;

		PyObject* name = PyUnicode_FromWideChar(fi->name, -1);
		PyObject* date = PyDateTime_FromDateAndTime(
			fi->date.Year(),
			fi->date.Month(),
			fi->date.Day(),
			fi->date.Hour(),
			fi->date.Minute(),
			fi->date.Second(),
			fi->date.Microsecond()
			);
		PyObject* method = PyLong_FromLong((long)fi->method);
		PyObject* uncompressedSize = PyLong_FromSize_t(fi->uncompressedSize);
		PyObject* compressedSize = PyLong_FromSize_t(fi->compressedSize);
		PyObject* compressLevel = PyLong_FromLong(fi->compressLevel);
		PyObject* crc32 = PyLong_FromUnsignedLong(fi->crc32);

		PyObject* dict = PyDict_New();
		PyDict_SetItemString(dict, "name", name);
		PyDict_SetItemString(dict, "date", date);
		PyDict_SetItemString(dict, "method", method);
		PyDict_SetItemString(dict, "compressLevel", compressLevel);
		PyDict_SetItemString(dict, "compressedSize", compressedSize);
		PyDict_SetItemString(dict, "uncompressedSize", uncompressedSize);
		PyDict_SetItemString(dict, "directory", (fi->directory ? Py_True : Py_False));
		PyDict_SetItemString(dict, "crypted", (fi->crypted ? Py_True : Py_False));
		PyDict_SetItemString(dict, "crc32", crc32);

		Py_DECREF(name);
		Py_DECREF(date);
		Py_DECREF(method);
		Py_DECREF(uncompressedSize);
		Py_DECREF(compressedSize);
		Py_DECREF(compressLevel);
		Py_DECREF(crc32);
		return dict;
	}
	PyErr_SetString(PyExc_FileNotFoundError, "File not found.");
	return NULL;
}

static PyObject* DCZipUnarchiverOpenFileStream(DCZipUnarchiver* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->zip, NULL);
	const char* file;
	const char* password = NULL;
	char* kwlist[] = { "file", "password", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|s", kwlist, &file, &password))
		return NULL;

	if (password && password[0] == 0)
		password = NULL;

	DKObject<DKStream> stream = NULL;
	Py_BEGIN_ALLOW_THREADS
	stream = self->zip->OpenFileStream(file, password);
	Py_END_ALLOW_THREADS
	if (stream)
	{
		return DCStreamFromObject(stream);
	}
	PyErr_SetString(PyExc_FileNotFoundError, "File not found.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "allFiles", (PyCFunction)&DCZipUnarchiverAllFiles, METH_NOARGS },
	{ "fileInfo", (PyCFunction)&DCZipUnarchiverFileInfo, METH_NOARGS },
	{ "openFileStream", (PyCFunction)&DCZipUnarchiverOpenFileStream, METH_VARARGS|METH_KEYWORDS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCZipUnarchiverFilename(DCZipUnarchiver* self, void*)
{
	DCOBJECT_VALIDATE(self->zip, NULL);
	return PyUnicode_FromWideChar(self->zip->GetArchiveName(), -1);
}

static PyGetSetDef getsets[] = {
	{ "filename", (getter)&DCZipUnarchiverFilename, NULL, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ZipUnarchiver",			/* tp_name */
	sizeof(DCZipUnarchiver),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCZipUnarchiverDealloc,		/* tp_dealloc */
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
	(initproc)&DCZipUnarchiverInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCZipUnarchiverNew,							/* tp_new */
};

PyTypeObject* DCZipUnarchiverTypeObject(void)
{
	return &objectType;
}

PyObject* DCZipUnarchiverFromObject(DKZipUnarchiver* zip)
{
	if (zip)
	{
		DCZipUnarchiver* self = (DCZipUnarchiver*)DCObjectFromAddress(zip);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCZipUnarchiver*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->zip = zip;
				DCObjectSetAddress(self->zip, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKZipUnarchiver* DCZipUnarchiverToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCZipUnarchiver*)obj)->zip;
	}
	return NULL;
}
