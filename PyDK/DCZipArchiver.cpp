#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKGL;
using namespace DKGL;

struct DCZipArchiver
{
	PyObject_HEAD
	DKObject<DKZipArchiver> zip;
};

static PyObject* DCZipArchiverNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCZipArchiver* self = (DCZipArchiver*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->zip) DKObject<DKZipArchiver>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCZipArchiverInit(DCZipArchiver *self, PyObject *args, PyObject *kwds)
{
	if (self->zip)
		return 0;

	const char* file;
	int append;
	char* kwlist[] = { "file", "append", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "sp", kwlist, &file, &append))
		return -1;

	Py_BEGIN_ALLOW_THREADS
	self->zip = DKZipArchiver::Create(file, append != 0);
	Py_END_ALLOW_THREADS

	if (self->zip)
	{
		DCObjectSetAddress(self->zip, (PyObject*)self);
		return 0;
	}
	PyErr_SetString(PyExc_IOError, "Cannot open file");
	return -1;
}

static void DCZipArchiverDealloc(DCZipArchiver* self)
{
	if (self->zip)
	{
		DCObjectSetAddress(self->zip, NULL);
		self->zip = NULL;
	}
	self->zip.~DKObject<DKZipArchiver>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCZipArchiverWrite(DCZipArchiver* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->zip, NULL);

	const char* file;
	PyObject* source = NULL;
	int level;
	const char* password = NULL;
	char* kwlist[] = { "file", "source", "compressionLevel", "password", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "sOi|s", kwlist, &file, &source, &level, &password))
		return NULL;

	if (password && password[0] == 0)
		password = NULL;
	
	bool result = false;
	DKStream* stream = DCStreamToObject(source);
	if (stream)
		result = self->zip->Write(file, stream, level, password);
	else
	{
		DKData* data = DCDataToObject(source);
		if (data)
		{
			Py_BEGIN_ALLOW_THREADS
			const void* p = data->LockShared();
			result = self->zip->Write(file, p, data->Length(), level, password);
			data->UnlockShared();
			Py_END_ALLOW_THREADS
		}
		else if (PyObject_CheckBuffer(source))
		{
			Py_buffer view;
			if (PyObject_GetBuffer(source, &view, PyBUF_SIMPLE) == 0)
			{
				Py_BEGIN_ALLOW_THREADS
				result = self->zip->Write(file, view.buf, view.len, level, password);
				Py_END_ALLOW_THREADS
				PyBuffer_Release(&view);
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "second argument must be Stream or Data or Buffer object.");
			return NULL;
		}
	}

	if (result)
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "write", (PyCFunction)&DCZipArchiverWrite, METH_VARARGS | METH_KEYWORDS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCZipArchiverFilename(DCZipArchiver* self, void*)
{
	DCOBJECT_VALIDATE(self->zip, NULL);

	return PyUnicode_FromWideChar(self->zip->GetArchiveName(), -1);
}

static PyGetSetDef getsets[] = {
	{ "filename", (getter)&DCZipArchiverFilename, NULL, 0, 0},
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ZipArchiver",			/* tp_name */
	sizeof(DCZipArchiver),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCZipArchiverDealloc,			/* tp_dealloc */
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
	(initproc)&DCZipArchiverInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCZipArchiverNew,							/* tp_new */
};

PyTypeObject* DCZipArchiverTypeObject(void)
{
	return &objectType;
}

PyObject* DCZipArchiverFromObject(DKZipArchiver* zip)
{
	if (zip)
	{
		DCZipArchiver* self = (DCZipArchiver*)DCObjectFromAddress(zip);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCZipArchiver*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKZipArchiver* DCZipArchiverToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCZipArchiver*)obj)->zip;
	}
	return NULL;
}
