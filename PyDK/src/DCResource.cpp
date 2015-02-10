#include <Python.h>
#include <structmember.h>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include "DCResource.h"
#include "DCObject.h"


static PyObject* DCResourceNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCResource* self = (DCResource*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->resource) DKObject<DKResource>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCResourceInit(DCResource *self, PyObject *args, PyObject *kwds)
{
	if (self->resource == NULL)
	{
		self->resource = DKObject<DKResource>::New();
		DCObjectSetAddress(self->resource, (PyObject*)self);
	}
	DKASSERT_DEBUG(DCObjectFromAddress(self->resource) == (PyObject*)self);
	return 0;
}

static void DCResourceDealloc(DCResource* self)
{
	if (self->resource)
	{
		DKASSERT_DEBUG(DCObjectFromAddress(self->resource) == (PyObject*)self);

		DCObjectSetAddress(self->resource, NULL);
		self->resource = NULL;
	}
	self->resource.~DKObject<DKResource>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCResourceSerialize(DCResource* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->resource, NULL);
	int type = -1;
	if (!PyArg_ParseTuple(args, "i", &type))
		return NULL;

	DKObject<DKData> data = NULL;
	switch (type)
	{
	case 0:
		data = self->resource->Serialize(DKSerializer::SerializeFormXML);
		break;
	case 1:
		data = self->resource->Serialize(DKSerializer::SerializeFormBinXML);
		break;
	case 2:
		data = self->resource->Serialize(DKSerializer::SerializeFormBinary);
		break;
	case 3:
		data = self->resource->Serialize(DKSerializer::SerializeFormCompressedBinary);
		break;
	default:
		PyErr_SetString(PyExc_ValueError, "Invalid argument.");
		return NULL;
		break;
	}
	if (data)
	{
		return DCDataFromObject(data);
	}
	PyErr_SetString(PyExc_RuntimeError, "Serialize Failed.");
	return NULL;
}

static PyObject* DCResourceDeserialize(DCResource* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->resource, NULL);
	Py_buffer buffer;		// str(xml), data, or buffer object
	PyObject* loaderObj = NULL;
	if (!PyArg_ParseTuple(args, "s*|O", &buffer, &loaderObj))
		return NULL;

	DCBufferRelease _tmp[] = { &buffer };

	DKResourceLoader* loader = NULL;
	if (loaderObj && loaderObj != Py_None)
	{
		loader = DCResourceLoaderToObject(loaderObj);
		if (loader == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "second argument must be ResourceLoader object.");
			return NULL;
		}
	}

	DKObject<DKData> data = DKData::StaticData(buffer.buf, buffer.len, true);
	bool result = self->resource->Deserialize(data, loader);
	data = NULL;

	if (result)
	{
		Py_RETURN_NONE;
	}

	PyErr_SetString(PyExc_RuntimeError, "Deserialize Failed.");
	return NULL;
}

static PyObject* DCResourceValidate(DCResource* self, void*)
{
	DCOBJECT_VALIDATE(self->resource, NULL);
	return PyBool_FromLong(self->resource->Validate());
}

static PyMethodDef methods[] = {
	{ "serialize", (PyCFunction)&DCResourceSerialize, METH_VARARGS },
	{ "deserialize", (PyCFunction)&DCResourceDeserialize, METH_VARARGS },
	{ "validate", (PyCFunction)&DCResourceValidate, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCResourceName(DCResource* self, void*)
{
	DCOBJECT_VALIDATE(self->resource, NULL);
	return PyUnicode_FromWideChar(self->resource->Name(), -1);
}

static int DCResourceSetName(DCResource* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->resource, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (!PyUnicode_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "attribute value must be a string");
		return -1;
	}

	char* name = PyUnicode_AsUTF8(value);
	self->resource->SetName(name);
	return 0;
}

static PyObject* DCResourceUUID(DCResource* self, void*)
{
	DCOBJECT_VALIDATE(self->resource, NULL);
	return PyUnicode_FromWideChar(self->resource->UUID().String(), -1);
}

static int DCResourceSetUUID(DCResource* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->resource, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (!PyUnicode_Check(value)) {
		PyErr_SetString(PyExc_TypeError, "attribute value must be a string");
		return -1;
	}

	char* str = PyUnicode_AsUTF8(value);
	DKUUID uuid(str);
	if (uuid.IsValid())
	{
		self->resource->SetUUID(uuid);
		return 0;
	}
	PyErr_SetString(PyExc_ValueError, "Invalid UUID.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "name", (getter)&DCResourceName, (setter)&DCResourceSetName, 0, 0},
	{ "uuid", (getter)&DCResourceUUID, (setter)&DCResourceSetUUID, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Resource",				/* tp_name */
	sizeof(DCResource),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCResourceDealloc,				/* tp_dealloc */
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
	(initproc)&DCResourceInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCResourceNew,								/* tp_new */
};

PyTypeObject* DCResourceTypeObject(void)
{
	return &objectType;
}

PyObject* DCResourceFromObject(DKResource* resource)
{
	if (resource)
	{
		DCResource* self = (DCResource*)DCObjectFromAddress(resource);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(Animation, resource);
			DCOBJECT_DYANMIC_CAST_CONVERT(GeometryBuffer, resource);
			DCOBJECT_DYANMIC_CAST_CONVERT(Material, resource);
			DCOBJECT_DYANMIC_CAST_CONVERT(Model, resource);
			DCOBJECT_DYANMIC_CAST_CONVERT(Texture, resource);
			DCOBJECT_DYANMIC_CAST_CONVERT(TextureSampler, resource);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCResource*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->resource = resource;
				DCObjectSetAddress(self->resource, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKResource* DCResourceToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCResource*)obj)->resource;
	}
	return NULL;
}
