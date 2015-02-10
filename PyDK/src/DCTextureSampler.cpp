#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCResource.h"
#include "DCObject.h"

struct DCTextureSampler
{
	DCResource base;
	DKTextureSampler* sampler;
};

static PyObject* DCTextureSamplerNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCTextureSampler* self = (DCTextureSampler*)DCResourceTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->sampler = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCTextureSamplerInit(DCTextureSampler *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKTextureSampler> sampler = NULL;
	if (self->sampler == NULL)
	{
		sampler = DKOBJECT_NEW DKTextureSampler();
		self->sampler = sampler;
		DCObjectSetAddress(self->sampler, (PyObject*)self);
	}

	self->base.resource = self->sampler;
	return DCResourceTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCTextureSamplerDealloc(DCTextureSampler* self)
{
	self->sampler = NULL;
	DCResourceTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCTextureSamplerMinFilter(DCTextureSampler* self, void*)
{
	DCOBJECT_VALIDATE(self->sampler, NULL);
	return PyLong_FromLong(self->sampler->minFilter);
}

static int DCTextureSamplerSetMinFilter(DCTextureSampler* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->sampler, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	
	long val = PyLong_AsLong(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be integer.");
		return -1;
	}
	if (val < DKTextureSampler::MinFilterUndefined || val > DKTextureSampler::MinFilterLinearMipmapLinear)
	{
		PyErr_Format(PyExc_ValueError, "attribute is out of range.");
		return -1;
	}
	self->sampler->minFilter = (DKTextureSampler::MinificationFilter)val;
	return 0;
}

static PyObject* DCTextureSamplerMagFilter(DCTextureSampler* self, void*)
{
	DCOBJECT_VALIDATE(self->sampler, NULL);
	return PyLong_FromLong(self->sampler->minFilter);
}

static int DCTextureSamplerSetMagFilter(DCTextureSampler* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->sampler, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	
	long val = PyLong_AsLong(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be integer.");
		return -1;
	}
	if (val < DKTextureSampler::MagFilterUndefined || val > DKTextureSampler::MagFilterLinear)
	{
		PyErr_Format(PyExc_ValueError, "attribute is out of range.");
		return -1;
	}
	self->sampler->magFilter = (DKTextureSampler::MagnificationFilter)val;
	return 0;
}

static PyObject* DCTextureSamplerWrapMode(DCTextureSampler* self, void* closure)
{
	DCOBJECT_VALIDATE(self->sampler, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);

	DKASSERT_DEBUG(offset < (sizeof(self->sampler->wrap) / sizeof(self->sampler->wrap[0])));
	return PyLong_FromLong(self->sampler->wrap[offset]);
}

static int DCTextureSamplerSetWrapMode(DCTextureSampler* self, PyObject* value, void* closure)
{
	DCOBJECT_VALIDATE(self->sampler, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	long val = PyLong_AsLong(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be int.");
		return -1;
	}
	if (val < DKTextureSampler::WrapUndefined || val > DKTextureSampler::WrapClampToEdge)
	{
		PyErr_Format(PyExc_ValueError, "attribute is out of range.");
		return -1;
	}

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(self->sampler->wrap) / sizeof(self->sampler->wrap[0])));
	self->sampler->wrap[offset] = (DKTextureSampler::Wrap)val;
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "minFilter", (getter)&DCTextureSamplerMinFilter, (setter)&DCTextureSamplerSetMinFilter, 0, 0 },
	{ "magFilter", (getter)&DCTextureSamplerMagFilter, (setter)&DCTextureSamplerSetMagFilter, 0, 0 },
	{ "wrapS", (getter)&DCTextureSamplerWrapMode, (setter)&DCTextureSamplerSetWrapMode, 0, (void*)0 },
	{ "wrapT", (getter)&DCTextureSamplerWrapMode, (setter)&DCTextureSamplerSetWrapMode, 0, (void*)1 },
	{ "wrapR", (getter)&DCTextureSamplerWrapMode, (setter)&DCTextureSamplerSetWrapMode, 0, (void*)2 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".TextureSampler",		/* tp_name */
	sizeof(DCTextureSampler),				/* tp_basicsize */
	0,										/* tp_itemsize */
	(destructor)&DCTextureSamplerDealloc,	/* tp_dealloc */
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
	0,										/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,					/* tp_flags */
	0,										/* tp_doc */
	0,										/* tp_traverse */
	0,										/* tp_clear */
	0,										/* tp_richcompare */
	0,										/* tp_weaklistoffset */
	0,										/* tp_iter */
	0,										/* tp_iternext */
	0,										/* tp_methods */
	0,										/* tp_members */
	getsets,								/* tp_getset */
	DCResourceTypeObject(),					/* tp_base */
	0,										/* tp_dict */
	0,										/* tp_descr_get */
	0,										/* tp_descr_set */
	0,										/* tp_dictoffset */
	(initproc)&DCTextureSamplerInit,		/* tp_init */
	0,										/* tp_alloc */
	&DCTextureSamplerNew,					/* tp_new */
};

PyTypeObject* DCTextureSamplerTypeObject(void)
{
	return &objectType;
}

PyObject* DCTextureSamplerFromObject(DKTextureSampler* sampler)
{
	if (sampler)
	{
		DCTextureSampler* self = (DCTextureSampler*)DCObjectFromAddress(sampler);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCTextureSampler*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->sampler = sampler;
				DCObjectSetAddress(self->sampler, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKTextureSampler* DCTextureSamplerToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCTextureSampler*)obj)->sampler;
	}
	return NULL;
}
