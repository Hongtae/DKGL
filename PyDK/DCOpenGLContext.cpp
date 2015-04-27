#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCOpenGLContext
{
	PyObject_HEAD
	DKObject<DKOpenGLContext> context;
};

static PyObject* DCOpenGLContextNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCOpenGLContext* self = (DCOpenGLContext*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->context) DKObject<DKOpenGLContext>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCOpenGLContextInit(DCOpenGLContext *self, PyObject *args, PyObject *kwds)
{
	if (self->context == NULL)
	{
		Py_BEGIN_ALLOW_THREADS
		self->context = DKOpenGLContext::SharedInstance();
		Py_END_ALLOW_THREADS
	}

	if (self->context == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "OpenGL Error.");
		return -1;
	}
	return 0;
}

static void DCOpenGLContextDealloc(DCOpenGLContext* self)
{
	self->context = NULL;
	self->context.~DKObject<DKOpenGLContext>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCOpenGLContextBind(DCOpenGLContext* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->context, NULL);
	Py_BEGIN_ALLOW_THREADS
	self->context->Bind(0);
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject* DCOpenGLContextUnbind(DCOpenGLContext* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->context, NULL);
	Py_BEGIN_ALLOW_THREADS
	self->context->Unbind();
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject* DCOpenGLContextIsBound(DCOpenGLContext* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->context, NULL);
	return PyBool_FromLong(self->context->IsBound());
}

static PyMethodDef methods[] = {
	{ "bind", (PyCFunction)&DCOpenGLContextBind, METH_NOARGS },
	{ "unbind", (PyCFunction)&DCOpenGLContextUnbind, METH_NOARGS },
	{ "isBound", (PyCFunction)&DCOpenGLContextIsBound, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".OpenGLContext",			/* tp_name */
	sizeof(DCOpenGLContext),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCOpenGLContextDealloc,		/* tp_dealloc */
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
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCOpenGLContextInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCOpenGLContextNew,						/* tp_new */
};

PyTypeObject* DCOpenGLContextTypeObject(void)
{
	return &objectType;
}

PyObject* DCOpenGLContextFromObject(DKOpenGLContext* context)
{
	if (context)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		PyObject* self = PyObject_Call(tp, args, kwds);
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);
		return self;
	}
	Py_RETURN_NONE;
}

DKOpenGLContext* DCOpenGLContextToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCOpenGLContext*)obj)->context;
	}
	return NULL;
}
