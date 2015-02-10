#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCLight
{
	PyObject_HEAD
	DKLight light;
};

static PyObject* DCLightNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCLight* self = (DCLight*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->light) DKLight();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCLightInit(DCLight *self, PyObject *args, PyObject *kwds)
{
	int type;
	DKVector3 pos;
	DKColor color;

	if (!PyArg_ParseTuple(args, "iO&O&", &type,
		&DCVector3Converter, &pos,
		&DCColorConverter, &color))
		return NULL;

	if (type <= 0 || type > 2)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid.");
		return NULL;
	}

	self->light.SetType((DKLight::LightType)type);
	self->light.position = pos;
	self->light.color = color;
	return 0;
}

static void DCLightDealloc(DCLight* self)
{
	self->light.~DKLight();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCLightAttenuation(DCLight* self, PyObject* args)
{
	DKVector3 pos;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &pos))
		return NULL;

	float f = self->light.Attenuation(pos);
	return PyFloat_FromDouble(f);
}

static PyMethodDef methods[] = {
	{ "attenuation", (PyCFunction)&DCLightAttenuation, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCLightType(DCLight* self, void*)
{
	int t = self->light.Type();
	return PyLong_FromLong(t);
}

static int DCLightSetType(DCLight* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	long type = PyLong_AsLong(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be integer.");
		return -1;
	}
	if (type <= 0 || type > 2)
	{
		PyErr_SetString(PyExc_ValueError, "attribute value is invalid.");
		return -1;
	}

	self->light.SetType((DKLight::LightType)type);
	return 0;
}

static PyObject* DCLightPosition(DCLight* self, void*)
{
	const DKVector3& p = self->light.position;
	return Py_BuildValue("fff", p.x, p.y, p.z);
}

static int DCLightSetPosition(DCLight* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3 pos;
	if (DCVector3Converter(value, &pos))
	{
		self->light.position = pos;
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Vector3 object.");
	return -1;
}

static PyObject* DCLightColor(DCLight* self, void*)
{
	const DKColor& c = self->light.color;
	return Py_BuildValue("ffff", c.r, c.g, c.b, c.a);
}

static int DCLightSetColor(DCLight* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKColor c;
	if (DCColorConverter(value, &c))
	{
		self->light.color = c;
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Color object.");
	return -1;
}

static PyObject* DCLightConstAttenuation(DCLight* self, void*)
{
	return PyFloat_FromDouble(self->light.constAttenuation);
}

static int DCLightSetConstAttenuation(DCLight* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	double v = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Flaot.");
		return -1;
	}
	self->light.constAttenuation = v;
	return 0;
}

static PyObject* DCLightLinearAttenuation(DCLight* self, void*)
{
	return PyFloat_FromDouble(self->light.linearAttenuation);
}

static int DCLightSetLinearAttenuation(DCLight* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	double v = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Flaot.");
		return -1;
	}
	self->light.linearAttenuation = v;
	return 0;
}

static PyObject* DCLightQuadraticAttenuation(DCLight* self, void*)
{
	return PyFloat_FromDouble(self->light.quadraticAttenuation);
}

static int DCLightSetQuadraticAttenuation(DCLight* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	double v = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Flaot.");
		return -1;
	}
	self->light.quadraticAttenuation = v;
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "type", (getter)&DCLightType, (setter)&DCLightSetType, 0, 0 },
	{ "position", (getter)&DCLightPosition, (setter)&DCLightSetPosition, 0, 0 },
	{ "color", (getter)&DCLightColor, (setter)&DCLightSetColor, 0, 0 },
	{ "constAttenuation", (getter)&DCLightConstAttenuation, (setter)&DCLightSetConstAttenuation, 0, 0 },
	{ "linearAttenuation", (getter)&DCLightLinearAttenuation, (setter)&DCLightSetLinearAttenuation, 0, 0 },
	{ "quadraticAttenuation", (getter)&DCLightQuadraticAttenuation, (setter)&DCLightSetQuadraticAttenuation, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Light",						/* tp_name */
	sizeof(DCLight),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCLightDealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	0,												/* tp_repr */
	0,												/* tp_as_number */
	0,												/* tp_as_sequence */
	0,												/* tp_as_mapping */
	0,												/* tp_hash  */
	0,												/* tp_call */
	0,												/* tp_str */
	0,												/* tp_getattro */
	0,												/* tp_setattro */
	0,												/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		/* tp_flags */
	0,												/* tp_doc */
	0,												/* tp_traverse */
	0,												/* tp_clear */
	0,												/* tp_richcompare */
	0,												/* tp_weaklistoffset */
	0,												/* tp_iter */
	0,												/* tp_iternext */
	methods,										/* tp_methods */
	0,												/* tp_members */
	getsets,										/* tp_getset */
	0,												/* tp_base */
	0,												/* tp_dict */
	0,												/* tp_descr_get */
	0,												/* tp_descr_set */
	0,												/* tp_dictoffset */
	(initproc)&DCLightInit,							/* tp_init */
	0,												/* tp_alloc */
	&DCLightNew,									/* tp_new */
};

PyTypeObject* DCLightTypeObject(void)
{
	return &objectType;
}

PyObject* DCLightFromObject(DKLight* light)
{
	if (light)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		DCLight* self = (DCLight*)PyObject_Call(tp, args, kwds);
		if (self)
		{
			DKASSERT_DEBUG(PyObject_TypeCheck(self, DCLightTypeObject()));
			self->light = *light;
		}
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKLight* DCLightToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCLight*)obj)->light;
	}
	return NULL;
}
