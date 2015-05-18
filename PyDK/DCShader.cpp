#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCShader
{
	PyObject_HEAD
	DKObject<DKShader> shader;
};

static PyObject* DCShaderNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCShader* self = (DCShader*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->shader) DKObject<DKShader>();
		self->shader = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCShaderInit(DCShader *self, PyObject *args, PyObject *kwds)
{
	if (self->shader)
		return 0;
	
	const char* source;
	int type;
	PyObject* dict = NULL;
	char* kwlist[] = { "source", "type", "result", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "si|O", kwlist, &source, &type))
		return -1;

	if (type <= DKShader::TypeUnknown || type > DKShader::TypeGeometryShader)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range.");
		return -1;
	}
	if (dict == Py_None)
		dict = NULL;
	if (dict && !PyMapping_Check(dict))
	{
		PyErr_SetString(PyExc_TypeError, "optional third argument must be mapping object.");
		return -1;
	}

	DKString err;
	Py_BEGIN_ALLOW_THREADS
	self->shader = DKShader::Create(source, (DKShader::Type)type, err);
	Py_END_ALLOW_THREADS

	if (dict)
	{
		PyObject* mesg = PyUnicode_FromWideChar(err, -1);
		PyObject* result = PyBool_FromLong(self->shader != NULL);

		PyMapping_SetItemString(dict, "message", mesg);
		PyMapping_SetItemString(dict, "result", result);

		Py_DECREF(mesg);
		Py_DECREF(result);
	}

	if (self->shader == NULL)
	{
		DKLog("compile shader error: %ls\n", (const wchar_t*)err);
		PyErr_SetString(PyExc_RuntimeError, "compile error");
		return -1;
	}

	DCObjectSetAddress(self->shader, (PyObject*)self);
	return 0;
}

static void DCShaderDealloc(DCShader* self)
{
	if (self->shader)
	{
		DCObjectSetAddress(self->shader, NULL);
		self->shader = NULL;
	}
	self->shader.~DKObject<DKShader>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCShaderIsValid(DCShader* self, PyObject*)
{
	if (self->shader && self->shader->IsValid())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "isValid", (PyCFunction)&DCShaderIsValid, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCShaderType(DCShader* self, void*)
{
	DCOBJECT_VALIDATE(self->shader, NULL);
	return PyLong_FromLong( self->shader->GetType() );
}

static PyGetSetDef getsets[] = {
	{ "type", (getter)&DCShaderType, 0, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Shader",					/* tp_name */
	sizeof(DCShader),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCShaderDealloc,				/* tp_dealloc */
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
	(initproc)&DCShaderInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCShaderNew,								/* tp_new */
};

PyTypeObject* DCShaderTypeObject(void)
{
	return &objectType;
}

PyObject* DCShaderFromObject(DKShader* shader)
{
	if (shader)
	{
		DCShader* self = (DCShader*)DCObjectFromAddress(shader);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCShader*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->shader = shader;
				DCObjectSetAddress(self->shader, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKShader* DCShaderToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCShader*)obj)->shader;
	}
	return NULL;
}
