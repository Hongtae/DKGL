#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCTexture.h"
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

struct DCTexture3D
{
	DCTexture base;
	DKTexture3D* texture;
};

static PyObject* DCTexture3DNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCTexture3D* self = (DCTexture3D*)DCTextureTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->texture = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCTexture3DInit(DCTexture3D *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKTexture3D> tex = NULL;
	if (self->texture == NULL)
	{
		//tex = DKOBJECT_NEW DKTexture3D();
		//self->texture = tex;
		PyErr_SetString(PyExc_NotImplementedError, "Not implemented yet.");
		return -1;
	}
	self->base.texture = tex;
	return DCTextureTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCTexture3DDealloc(DCTexture3D* self)
{
	self->texture = NULL;
	DCTextureTypeObject()->tp_dealloc((PyObject*)self);
}

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Texture3D",				/* tp_name */
	sizeof(DCTexture3D),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCTexture3DDealloc,			/* tp_dealloc */
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
	0,											/* tp_methods */
	0,											/* tp_members */
	0,											/* tp_getset */
	DCTextureTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCTexture3DInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCTexture3DNew,								/* tp_new */
};

PyTypeObject* DCTexture3DTypeObject(void)
{
	return &objectType;
}

PyObject* DCTexture3DFromObject(DKTexture3D* texture)
{
	if (texture)
	{
		DCTexture3D* self = (DCTexture3D*)DCObjectFromAddress(texture);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCTexture3D*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->texture = texture;
				DCObjectSetAddress(self->texture, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKTexture3D* DCTexture3DToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCTexture3D*)obj)->texture;
	}
	return NULL;
}
