#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCGeometryBuffer.h"
#include "DCObject.h"

static PyObject* DCGeometryBufferNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCGeometryBuffer* self = (DCGeometryBuffer*)DCResourceTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->buffer = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCGeometryBufferInit(DCGeometryBuffer *self, PyObject *args, PyObject *kwds)
{
	if (self->buffer == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid object");
		return -1;
	}

	self->base.resource = self->buffer;
	return DCResourceTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCGeometryBufferDealloc(DCGeometryBuffer* self)
{
	self->buffer = NULL;
	DCResourceTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCGeometryBufferCopyContent(DCGeometryBuffer* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);

	DKObject<DKBuffer> data = NULL;
	Py_BEGIN_ALLOW_THREADS
	data = self->buffer->CopyContent();
	Py_END_ALLOW_THREADS

	if (data)
	{
		return DCDataFromObject(data);
	}
	PyErr_SetString(PyExc_NotImplementedError, "not supported");
	return NULL;
}

static PyObject* DCGeometryBufferUpdateSubContent(DCGeometryBuffer* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);

	Py_buffer source;
	Py_ssize_t offset;
	Py_ssize_t length = -1;

	char* kwlist[] = { "source", "offset", "length", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "y*n|n", kwlist, &source, &offset, &length))
		return NULL;

	if (length < 0)
		length = source.len;
	else
		length = Min(length, source.len);

	bool result = false;
	Py_BEGIN_ALLOW_THREADS
	result = self->buffer->UpdateSubContent(source.buf, offset, length);
	Py_END_ALLOW_THREADS
	PyBuffer_Release(&source);

	if (result)
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCGeometryBufferIsValid(DCGeometryBuffer* self, PyObject*)
{
	if (self->buffer && self->buffer->IsValid())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "copyContent", (PyCFunction)&DCGeometryBufferCopyContent, METH_NOARGS },
	{ "UpdateSubContent", (PyCFunction)&DCGeometryBufferUpdateSubContent, METH_VARARGS | METH_KEYWORDS },
	{ "isValid", (PyCFunction)&DCGeometryBufferIsValid, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCGeometryBufferSize(DCGeometryBuffer* self, void*)
{
	size_t s = 0;
	if (self->buffer)
		s = self->buffer->Size();
	return PyLong_FromSize_t(s);
}

static PyObject* DCGeometryBufferType(DCGeometryBuffer* self, void*)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	return PyLong_FromLong((long)self->buffer->Type());
}

static PyObject* DCGeometryBufferLocation(DCGeometryBuffer* self, void*)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	return PyLong_FromLong((long)self->buffer->Location());
}

static PyObject* DCGeometryBufferUsage(DCGeometryBuffer* self, void*)
{
	DCOBJECT_VALIDATE(self->buffer, NULL);
	return PyLong_FromLong((long)self->buffer->Usage());
}

static PyGetSetDef getsets[] = {
	{ "size", (getter)&DCGeometryBufferSize, 0, 0, 0 },
	{ "type", (getter)&DCGeometryBufferType, 0, 0, 0 },
	{ "location", (getter)&DCGeometryBufferLocation, 0, 0, 0 },
	{ "usage", (getter)&DCGeometryBufferUsage, 0, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".GeometryBuffer",			/* tp_name */
	sizeof(DCGeometryBuffer),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCGeometryBufferDealloc,		/* tp_dealloc */
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
	DCResourceTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCGeometryBufferInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCGeometryBufferNew,								/* tp_new */
};

PyTypeObject* DCGeometryBufferTypeObject(void)
{
	return &objectType;
}

PyObject* DCGeometryBufferFromObject(DKGeometryBuffer* buffer)
{
	if (buffer)
	{
		DCGeometryBuffer* self = (DCGeometryBuffer*)DCObjectFromAddress(buffer);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(IndexBuffer, buffer);
			DCOBJECT_DYANMIC_CAST_CONVERT(VertexBuffer, buffer);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCGeometryBuffer*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKGeometryBuffer* DCGeometryBufferToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCGeometryBuffer*)obj)->buffer;
	}
	return NULL;
}
