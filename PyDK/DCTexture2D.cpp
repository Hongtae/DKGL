#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCTexture.h"
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

struct DCTexture2D
{
	DCTexture base;
	DKTexture2D* texture;
};

static PyObject* DCTexture2DNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCTexture2D* self = (DCTexture2D*)DCTextureTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->texture = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCTexture2DInit(DCTexture2D *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKTexture2D> tex = NULL;

	if (self->texture == NULL)
	{
		// to create emtpy texture following arguments must be followed: 'width', 'height', 'format', 'type'.
		// to read from data following arugments must be followed: 'source', and 'width', 'height' can be combined optinally.
		// source must be file-path (string) or data (Py_Buffer)
		PyObject* source = NULL;
		int width = 0;
		int height = 0;
		int format = -1;
		int type = -1;

		char* kwlist[] = { "width", "height", "format", "type", "source", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "|$iiiiO", kwlist,
			&width, &height, &format, &type, &source))
			return -1;

		if (source && source != Py_None)
		{
			if (format != -1 || type != -1)
			{
				PyErr_SetString(PyExc_ValueError, "For argument 'source', argument 'format' and 'type' should be omitted.");
				return -1;
			}

			if (PyUnicode_Check(source))
			{
				DKString url = PyUnicode_AsUTF8(source);
				Py_BEGIN_ALLOW_THREADS
				if (width > 0 && height > 0)
					tex = DKTexture2D::Create(url, width, height);
				else
					tex = DKTexture2D::Create(url);
				Py_END_ALLOW_THREADS
			}
			else if (PyObject_TypeCheck(source, DCStreamTypeObject()))
			{
				DKStream* stream = DCStreamToObject(source);
				if (stream)
				{
					Py_BEGIN_ALLOW_THREADS
					if (width > 0 && height > 0)
						tex = DKTexture2D::Create(stream, width, height);
					else
						tex = DKTexture2D::Create(stream);
					Py_END_ALLOW_THREADS
				}
			}
			if (tex == NULL && PyObject_CheckBuffer(source))
			{
				auto createTextureFromData = [](DKData* data, int w, int h) -> DKObject<DKTexture2D>
				{
					DKObject<DKTexture2D> ret = NULL;
					Py_BEGIN_ALLOW_THREADS
					if (w > 0 && h > 0)
						ret = DKTexture2D::Create(data, w, h);
					else
						ret = DKTexture2D::Create(data);
					Py_END_ALLOW_THREADS
					return ret;
				};

				DKObject<DKData> data = DCDataToObject(source);
				if (data)
				{
					tex = createTextureFromData(data, width, height);
				}
				else
				{
					Py_buffer view;
					if (PyObject_GetBuffer(source, &view, PyBUF_SIMPLE) == 0)
					{
						data = DKData::StaticData(view.buf, view.len, true);
						if (data)
							tex = createTextureFromData(data, width, height);
						data = NULL;
						PyBuffer_Release(&view);
					}
				}
			}
		}
		else
		{
			if (width < 1 || height < 1)
			{
				PyErr_SetString(PyExc_ValueError, "argument 'width', 'height' must be greater than zero.");
				return -1;
			}
			if (format < 0 || format >(int)DKTexture::FormatDepth32F)
			{
				PyErr_SetString(PyExc_ValueError, "argument 'format' is out of range.");
				return -1;
			}
			if (type < 0 || type >(int)DKTexture::TypeDouble)
			{
				PyErr_SetString(PyExc_ValueError, "argument 'type' is out of range.");
				return -1;
			}

			tex = DKTexture2D::Create(width, height, (DKTexture::Format)format, (DKTexture::Type)type, NULL);
		}

		if (tex == NULL)
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_RuntimeError, "Cannot create texture!");
			return -1;
		}
		self->texture = tex;
		DCObjectSetAddress(self->texture, (PyObject*)self);
	}

	self->base.texture = self->texture;
	return DCTextureTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCTexture2DDealloc(DCTexture2D* self)
{
	self->texture = NULL;
	DCTextureTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCTexture2DSetPixelData(DCTexture2D* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->texture, NULL);

	DKRect rc;
	Py_buffer data;
	if (!PyArg_ParseTuple(args, "y*O&", &data, &DCRectConverter, &rc))
		return NULL;

	size_t size = rc.size.width * rc.size.height;
	if (data.len >= size)
	{
		self->texture->SetPixelData(rc, data.buf);
		PyBuffer_Release(&data);
		Py_RETURN_NONE;
	}
	PyBuffer_Release(&data);

	PyErr_SetString(PyExc_ValueError, "Too small data length.");
	return NULL;
}

static PyObject* DCTexture2DCopyPixelData(DCTexture2D* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->texture, NULL);

	DKRect rc;
	int format = (int)DKTexture::FormatUnknown;
	int type = (int)DKTexture::TypeUnsignedByte;

	if (!PyArg_ParseTuple(args, "O&|ii", &DCRectConverter, &rc, &format, &type))
		return NULL;

	DKObject<DKData> data = self->texture->CopyPixelData(rc, (DKTexture::Format)format, (DKTexture::Type)type);
	if (data)
	{
		return DCDataFromObject(data);
	}
	PyErr_SetString(PyExc_RuntimeError, "Operation failed.");
	return NULL;
}

static PyObject* DCTexture2DDuplicate(DCTexture2D* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->texture, NULL);

	DKObject<DKTexture2D> texture = self->texture->Duplicate();
	if (texture)
	{
		return DCTexture2DFromObject(texture);
	}
	PyErr_SetString(PyExc_RuntimeError, "Operation failed.");
	return NULL;
}

static PyObject* DCTexture2DCreateImageFileData(DCTexture2D* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->texture, NULL);

	const char* format = NULL;
	if (!PyArg_ParseTuple(args, "s", &format))
		return NULL;

	DKObject<DKData> data = self->texture->CreateImageFileData(format);
	if (data)
	{
		return DCDataFromObject(data);
	}
	PyErr_SetString(PyExc_RuntimeError, "Operation failed.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "setPixelData", (PyCFunction)&DCTexture2DSetPixelData, METH_VARARGS },
	{ "copyPixelData", (PyCFunction)&DCTexture2DCopyPixelData, METH_VARARGS },
	{ "duplicate", (PyCFunction)&DCTexture2DDuplicate, METH_NOARGS },
	{ "createImageFileData", (PyCFunction)&DCTexture2DCreateImageFileData, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Texture2D",				/* tp_name */
	sizeof(DCTexture2D),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCTexture2DDealloc,			/* tp_dealloc */
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
	DCTextureTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCTexture2DInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCTexture2DNew,								/* tp_new */
};

PyTypeObject* DCTexture2DTypeObject(void)
{
	return &objectType;
}

PyObject* DCTexture2DFromObject(DKTexture2D* texture)
{
	if (texture)
	{
		DCTexture2D* self = (DCTexture2D*)DCObjectFromAddress(texture);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCTexture2D*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKTexture2D* DCTexture2DToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCTexture2D*)obj)->texture;
	}
	return NULL;
}
