#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCTexture.h"
#include "DCObject.h"

using namespace DKGL;
using namespace DKGL;

struct DCTextureCube
{
	DCTexture base;
	DKTextureCube* texture;
};

static PyObject* DCTextureCubeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCTextureCube* self = (DCTextureCube*)DCTextureTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->texture = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCTextureCubeInit(DCTextureCube *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKTextureCube> tex = NULL;
	if (self->texture == NULL)
	{
		//tex = DKOBJECT_NEW DKTextureCube();
		//self->texture = tex;
		PyErr_SetString(PyExc_NotImplementedError, "Not implemented yet.");
		return -1;
	}
	self->base.texture = tex;
	return DCTextureTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCTextureCubeDealloc(DCTextureCube* self)
{
	self->texture = NULL;
	DCTextureTypeObject()->tp_dealloc((PyObject*)self);
}

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".TextureCube",				/* tp_name */
	sizeof(DCTextureCube),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCTextureCubeDealloc,			/* tp_dealloc */
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
	(initproc)&DCTextureCubeInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCTextureCubeNew,								/* tp_new */
};

PyTypeObject* DCTextureCubeTypeObject(void)
{
	return &objectType;
}

PyObject* DCTextureCubeFromObject(DKTextureCube* texture)
{
	if (texture)
	{
		DCTextureCube* self = (DCTextureCube*)DCObjectFromAddress(texture);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCTextureCube*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKTextureCube* DCTextureCubeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCTextureCube*)obj)->texture;
	}
	return NULL;
}
