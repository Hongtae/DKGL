#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCTexture.h"
#include "DCObject.h"


static PyObject* DCTextureNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCTexture* self = (DCTexture*)DCResourceTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->texture = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCTextureInit(DCTexture *self, PyObject *args, PyObject *kwds)
{
	if (self->texture == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid object");
		return -1;
	}

	self->base.resource = self->texture;
	return DCResourceTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCTextureDealloc(DCTexture* self)
{
	self->texture = NULL;
	DCResourceTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCTextureMaxTextureSize(PyObject*, PyObject*)
{
	return PyLong_FromLong(DKTexture::MaxTextureSize());
}

static PyMethodDef methods[] = {
	{ "maxTextureSize", &DCTextureMaxTextureSize, METH_NOARGS | METH_STATIC },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCTextureTarget(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	return PyLong_FromLong(self->texture->ObjectTarget());
}

static PyObject* DCTextureFormat(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	return PyLong_FromLong(self->texture->TextureFormat());
}

static PyObject* DCTextureType(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	return PyLong_FromLong(self->texture->ComponentType());
}

static PyObject* DCTextureWidth(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	return PyLong_FromLong(self->texture->Width());
}

static PyObject* DCTextureHeight(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	return PyLong_FromLong(self->texture->Height());
}

static PyObject* DCTextureDepth(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	return PyLong_FromLong(self->texture->Depth());
}

static PyObject* DCTextureDimensions(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	DKVector3 dim = self->texture->Dimensions();
	return Py_BuildValue("fff", dim.x, dim.y, dim.z);
}

static PyObject* DCTextureNumberOfComponents(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	return PyLong_FromSize_t(self->texture->NumberOfComponents());
}

static PyObject* DCTextureBytesPerPixel(DCTexture* self, void*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);
	return PyLong_FromSize_t(self->texture->BytesPerPixel());
}

static PyGetSetDef getsets[] = {
	{ "target", (getter)&DCTextureTarget, NULL, 0, 0 },
	{ "format", (getter)&DCTextureFormat, NULL, 0, 0 },
	{ "componentType", (getter)&DCTextureType, NULL, 0, 0 },
	{ "width", (getter)&DCTextureWidth, NULL, 0, 0 },
	{ "height", (getter)&DCTextureHeight, NULL, 0, 0 },
	{ "depth", (getter)&DCTextureDepth, NULL, 0, 0 },
	{ "dimensions", (getter)&DCTextureDimensions, NULL, 0, 0 },
	{ "numberOfComponents", (getter)&DCTextureNumberOfComponents, NULL, 0, 0 },
	{ "bytesPerPixel", (getter)&DCTextureBytesPerPixel, NULL, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Texture",				/* tp_name */
	sizeof(DCTexture),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCTextureDealloc,				/* tp_dealloc */
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
	(initproc)&DCTextureInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCTextureNew,								/* tp_new */
};

PyTypeObject* DCTextureTypeObject(void)
{
	return &objectType;
}

PyObject* DCTextureFromObject(DKTexture* texture)
{
	if (texture)
	{
		DCTexture* self = (DCTexture*)DCObjectFromAddress(texture);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(Texture2D, texture);
			DCOBJECT_DYANMIC_CAST_CONVERT(Texture3D, texture);
			DCOBJECT_DYANMIC_CAST_CONVERT(TextureCube, texture);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCTexture*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKTexture* DCTextureToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCTexture*)obj)->texture;
	}
	return NULL;
}
