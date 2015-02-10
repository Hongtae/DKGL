#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

template <typename T> static inline PyObject* GetAttrDict(T&& val)
{
	PyObject* name = PyUnicode_FromWideChar(val.name, -1);
	PyObject* tid = PyLong_FromLong(val.id);
	PyObject* type = PyLong_FromLong(val.type);
	PyObject* components = PyLong_FromSize_t(val.components);
	PyObject* location = PyLong_FromLong(val.location);

	PyObject* dict = PyDict_New();
	PyDict_SetItemString(dict, "name", name);
	PyDict_SetItemString(dict, "id", tid);
	PyDict_SetItemString(dict, "type", type);
	PyDict_SetItemString(dict, "components", components);
	PyDict_SetItemString(dict, "location", location);

	Py_DECREF(name);
	Py_DECREF(tid);
	Py_DECREF(type);
	Py_DECREF(components);
	Py_DECREF(location);

	return dict;
}

struct DCShaderProgram
{
	PyObject_HEAD
	DKObject<DKShaderProgram> program;
	PyObject* attributes;
	PyObject* uniforms;
	void UpdateUniforms(void)
	{
		size_t numUniforms = this->program->uniforms.Count();
		PyObject* tuple = PyTuple_New(numUniforms);
		for (int i = 0; i < numUniforms; ++i)
		{
			DKShaderConstant& sc = this->program->uniforms.Value(i);
			PyObject* dict = GetAttrDict(sc);
			DKASSERT_DEBUG(PyDict_Check(dict));
			PyTuple_SET_ITEM(tuple, i, dict);
		}
		Py_CLEAR(this->uniforms);
		this->uniforms = tuple;
	}
	void UpdateAttributes(void)
	{
		size_t numAttrs = this->program->attributes.Count();
		PyObject* tuple = PyTuple_New(numAttrs);
		for (int i = 0; i < numAttrs; ++i)
		{
			DKVertexStream& vs = this->program->attributes.Value(i);
			PyObject* dict = GetAttrDict(vs);
			DKASSERT_DEBUG(PyDict_Check(dict));
			PyTuple_SET_ITEM(tuple, i, dict);
		}

		Py_CLEAR(this->attributes);
		this->attributes = tuple;
	}
};

static PyObject* DCShaderProgramNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCShaderProgram* self = (DCShaderProgram*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->program) DKObject<DKShaderProgram>();
		self->program = NULL;
		self->uniforms = NULL;
		self->attributes = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCShaderProgramInit(DCShaderProgram *self, PyObject *args, PyObject *kwds)
{
	if (self->program)
	{
		self->UpdateAttributes();
		self->UpdateUniforms();
		return 0;
	}

	PyObject* shaders;
	PyObject* dict = NULL;
	char* kwlist[] = { "shaders", "result", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &shaders, &dict))
		return -1;

	if (!PySequence_Check(shaders))
	{
		PyErr_SetString(PyExc_TypeError, "first argument must be sequence object.");
		return -1;
	}
	if (dict == Py_None)
		dict = NULL;
	if (dict && !PyMapping_Check(dict))
	{
		PyErr_SetString(PyExc_TypeError, "optional second argument must be mapping object.");
		return -1;
	}

	Py_ssize_t numItems = PySequence_Size(shaders);
	DKArray<DKObject<DKShader>> shaderArray;
	shaderArray.Reserve(numItems);
	for (Py_ssize_t i = 0; i < numItems; ++i)
	{
		PyObject* obj = PySequence_GetItem(shaders, i);
		if (obj == NULL)
			return -1;
		DCObjectRelease obj_(obj);

		DKShader* shader = DCShaderToObject(obj);
		if (shader == NULL)
		{
			PyErr_Format(PyExc_TypeError, "first argument's items at %zd must be Shader object.", i);
			return -1;
		}
		shaderArray.Add(shader);
	}

	DKString err;
	Py_BEGIN_ALLOW_THREADS
	self->program = DKShaderProgram::Create(shaderArray, err);
	Py_END_ALLOW_THREADS

	if (dict)
	{
		PyObject* mesg = PyUnicode_FromWideChar(err, -1);
		PyObject* result = PyBool_FromLong(self->program != NULL);

		PyMapping_SetItemString(dict, "message", mesg);
		PyMapping_SetItemString(dict, "result", result);

		Py_DECREF(mesg);
		Py_DECREF(result);
	}

	if (self->program == NULL)
	{
		DKLog("build program error: %ls\n", (const wchar_t*)err);
		PyErr_SetString(PyExc_RuntimeError, "program error");
		return -1;
	}

	DCObjectSetAddress(self->program, (PyObject*)self);

	self->UpdateAttributes();
	self->UpdateUniforms();
	return 0;
}

static int DCShaderProgramClear(DCShaderProgram* self)
{
	Py_CLEAR(self->attributes);
	Py_CLEAR(self->uniforms);
	return 0;
}

static int DCShaderProgramTraverse(DCShaderProgram* self, visitproc visit, void* arg)
{
	Py_VISIT(self->attributes);
	Py_VISIT(self->uniforms);
	return 0;
}

static void DCShaderProgramDealloc(DCShaderProgram* self)
{
	if (self->program)
	{
		DCObjectSetAddress(self->program, NULL);
		self->program = NULL;
	}
	self->program.~DKObject<DKShaderProgram>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCShaderProgramIsValid(DCShaderProgram* self, PyObject*)
{
	if (self->program && self->program->IsValid())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "isValid", (PyCFunction)&DCShaderProgramIsValid, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyMemberDef members[] = {
	{ "attributes", T_OBJECT_EX, offsetof(DCShaderProgram, attributes), 0, 0 },
	{ "uniforms", T_OBJECT_EX, offsetof(DCShaderProgram, uniforms), 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ShaderProgram",			/* tp_name */
	sizeof(DCShaderProgram),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCShaderProgramDealloc,		/* tp_dealloc */
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
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCShaderProgramTraverse,		/* tp_traverse */
	(inquiry)&DCShaderProgramClear,				/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	members,									/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCShaderProgramInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCShaderProgramNew,						/* tp_new */
};

PyTypeObject* DCShaderProgramTypeObject(void)
{
	return &objectType;
}

PyObject* DCShaderProgramFromObject(DKShaderProgram* program)
{
	if (program)
	{
		DCShaderProgram* self = (DCShaderProgram*)DCObjectFromAddress(program);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCShaderProgram*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->program = program;
				DCObjectSetAddress(self->program, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKShaderProgram* DCShaderProgramToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCShaderProgram*)obj)->program;
	}
	return NULL;
}
