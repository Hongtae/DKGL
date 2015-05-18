#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCAudioSource
{
	PyObject_HEAD
	DKObject<DKAudioSource> source;
};

static PyObject* DCAudioSourceNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCAudioSource* self = (DCAudioSource*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->source) DKObject<DKAudioSource>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCAudioSourceInit(DCAudioSource *self, PyObject *args, PyObject *kwds)
{
	if (self->source == NULL)
	{
		self->source = DKObject<DKAudioSource>::New();
		DCObjectSetAddress(self->source, (PyObject*)self);
	}
	return 0;
}

static void DCAudioSourceDealloc(DCAudioSource* self)
{
	if (self->source)
	{
		DCObjectSetAddress(self->source, NULL);
		self->source = NULL;
	}

	self->source.~DKObject<DKAudioSource>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCAudioSourcePlay(DCAudioSource* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	self->source->Play();
	Py_RETURN_NONE;
}

static PyObject* DCAudioSourceStop(DCAudioSource* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	self->source->Stop();
	Py_RETURN_NONE;
}

static PyObject* DCAudioSourcePause(DCAudioSource* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	self->source->Pause();
	Py_RETURN_NONE;
}

static PyObject* DCAudioSourceState(DCAudioSource* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	return PyLong_FromLong((int)self->source->State());
}

static PyObject* DCAudioSourceEnqueueBuffer(DCAudioSource* self, PyObject* args, PyObject* kwds)
{
	int frequency;
	int bits;
	int channels;
	Py_buffer buffer;
	Py_ssize_t length;
	double timeStamp;
	char* kwlist[] = { "frequency", "bits", "channels", "buffer", "length", "timeStamp", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "iiiy*nd", kwlist, &frequency, &bits, &channels, &buffer, &length, &timeStamp))
		return NULL;

	if (length == 0)
		length = buffer.len;
	else if (length > buffer.len)
	{
		PyBuffer_Release(&buffer);
		PyErr_SetString(PyExc_ValueError, "fifth argument is invalid.");
		return NULL;
	}

	bool b = self->source->EnqueueBuffer(frequency, bits, channels, buffer.buf, length, timeStamp);
	PyBuffer_Release(&buffer);
	if (!b)
	{
		PyErr_SetString(PyExc_RuntimeError, "EnqueueBuffer failed.");
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject* DCAudioSourceUnqueueBuffers(DCAudioSource* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	self->source->UnqueueBuffers();
	Py_RETURN_NONE;
}

static PyObject* DCAudioSourceQueuedBuffers(DCAudioSource* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	return PyLong_FromSize_t(self->source->QueuedBuffers());
}

static PyObject* DCAudioSourceFormat(PyObject*, PyObject* args)
{
	int bits, channels;
	if (!PyArg_ParseTuple(args, "ii", &bits, &channels))
		return NULL;

	return PyLong_FromLong(DKAudioSource::Format(bits, channels));
}

static PyObject* DCAudioSourceErrorCode(PyObject*, PyObject* args)
{
	return PyLong_FromLong(DKAudioSource::ErrorCode());
}

static PyObject* DCAudioSourceErrorString(PyObject*, PyObject* args)
{
	int code;
	if (!PyArg_ParseTuple(args, "i", &code))
		return NULL;
	return PyUnicode_FromWideChar(DKAudioSource::ErrorString(code), -1);
}

static PyMethodDef methods[] = {
	{ "play", (PyCFunction)&DCAudioSourcePlay, METH_NOARGS },
	{ "stop", (PyCFunction)&DCAudioSourceStop, METH_NOARGS },
	{ "pause", (PyCFunction)&DCAudioSourcePause, METH_NOARGS },
	{ "state", (PyCFunction)&DCAudioSourceState, METH_NOARGS },

	{ "enqueueBuffer", (PyCFunction)&DCAudioSourceEnqueueBuffer, METH_VARARGS | METH_KEYWORDS },
	{ "unqueueBuffers", (PyCFunction)&DCAudioSourceUnqueueBuffers, METH_NOARGS },
	{ "queuedBuffers", (PyCFunction)&DCAudioSourceQueuedBuffers, METH_NOARGS },

	{ "format", &DCAudioSourceFormat, METH_VARARGS | METH_STATIC },
	{ "errorCode", &DCAudioSourceErrorCode, METH_VARARGS | METH_STATIC },
	{ "errorString", &DCAudioSourceErrorString, METH_VARARGS | METH_STATIC },

	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

template <typename GetterType, typename SetterType>
struct DCAudioSourceAttrT
{
	GetterType(DKAudioSource::*getter)(void) const;
	void (DKAudioSource::*setter)(SetterType);
};

static DCAudioSourceAttrT<DKVector3, const DKVector3&> vector3Attrs[] =
{
	{ &DKAudioSource::Position, &DKAudioSource::SetPosition },
	{ &DKAudioSource::Velocity, &DKAudioSource::SetVelocity },
	{ &DKAudioSource::Direction, &DKAudioSource::SetDirection }
};

static PyObject* DCAudioSourceGetVector3Attr(DCAudioSource* self, void* closure)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(vector3Attrs) / sizeof(vector3Attrs[0])));
	DKASSERT_DEBUG(vector3Attrs[offset].getter);

	DKVector3 v = (self->source->*(vector3Attrs[offset].getter))();
	return Py_BuildValue("fff", v.x, v.y, v.z);
}

static int DCAudioSourceSetVector3Attr(DCAudioSource* self, PyObject* value, void* closure)
{
	DCOBJECT_VALIDATE(self->source, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3 v;
	if (!DCVector3Converter(value, &v))
		return -1;

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(vector3Attrs) / sizeof(vector3Attrs[0])));
	DKASSERT_DEBUG(vector3Attrs[offset].setter);

	(self->source->*(vector3Attrs[offset].setter))(v);
	return 0;
}

static DCAudioSourceAttrT<float, float> floatAttrs[] =
{
	{ &DKAudioSource::Pitch, &DKAudioSource::SetPitch },
	{ &DKAudioSource::Gain, &DKAudioSource::SetGain },
	{ &DKAudioSource::MinGain, &DKAudioSource::SetMinGain },
	{ &DKAudioSource::MaxGain, &DKAudioSource::SetMaxGain },
	{ &DKAudioSource::MaxDistance, &DKAudioSource::SetMaxDistance },
	{ &DKAudioSource::RolloffFactor, &DKAudioSource::SetRolloffFactor },
	{ &DKAudioSource::ConeOuterGain, &DKAudioSource::SetConeOuterGain },
	{ &DKAudioSource::ConeInnerAngle, &DKAudioSource::SetConeInnerAngle },
	{ &DKAudioSource::ConeOuterAngle, &DKAudioSource::SetConeOuterAngle },
	{ &DKAudioSource::ReferenceDistance, &DKAudioSource::SetReferenceDistance }
};

static PyObject* DCAudioSourceGetFloatAttr(DCAudioSource* self, void* closure)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(floatAttrs) / sizeof(floatAttrs[0])));
	DKASSERT_DEBUG(floatAttrs[offset].getter);

	return PyFloat_FromDouble((self->source->*(floatAttrs[offset].getter))());
}

static int DCAudioSourceSetFloatAttr(DCAudioSource* self, PyObject* value, void* closure)
{
	DCOBJECT_VALIDATE(self->source, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double f = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be float number.");
		return -1;
	}

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(floatAttrs) / sizeof(floatAttrs[0])));
	DKASSERT_DEBUG(floatAttrs[offset].setter);

	(self->source->*(floatAttrs[offset].setter))(f);
	return 0;
}

static DCAudioSourceAttrT<double, double> doubleAttrs[] =
{
	{ &DKAudioSource::TimeOffset, &DKAudioSource::SetTimeOffset },
	{ &DKAudioSource::TimePosition, &DKAudioSource::SetTimePosition }
};

static PyObject* DCAudioSourceGetDoubleAttr(DCAudioSource* self, void* closure)
{
	DCOBJECT_VALIDATE(self->source, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(doubleAttrs) / sizeof(doubleAttrs[0])));
	DKASSERT_DEBUG(doubleAttrs[offset].getter);

	return PyFloat_FromDouble((self->source->*(doubleAttrs[offset].getter))());
}

static int DCAudioSourceSetDoubleAttr(DCAudioSource* self, PyObject* value, void* closure)
{
	DCOBJECT_VALIDATE(self->source, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double f = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be float number.");
		return -1;
	}

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(doubleAttrs) / sizeof(doubleAttrs[0])));
	DKASSERT_DEBUG(doubleAttrs[offset].setter);

	(self->source->*(doubleAttrs[offset].setter))(f);
	return 0;
}


static PyGetSetDef getsets[] = {
	{ "position", (getter)&DCAudioSourceGetVector3Attr, (setter)&DCAudioSourceSetVector3Attr, 0, (void*)0 },
	{ "velocity", (getter)&DCAudioSourceGetVector3Attr, (setter)&DCAudioSourceSetVector3Attr, 0, (void*)1 },
	{ "direction", (getter)&DCAudioSourceGetVector3Attr, (setter)&DCAudioSourceSetVector3Attr, 0, (void*)2 },

	{ "pitch", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)0 },
	{ "gain", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)1 },
	{ "minGain", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)2 },
	{ "maxGain", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)3 },
	{ "maxDistance", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)4 },
	{ "rolloffFactor", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)5 },
	{ "coneOuterGain", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)6 },
	{ "coneInnerAngle", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)7 },
	{ "coneOuterAngle", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)8 },
	{ "referenceDistance", (getter)&DCAudioSourceGetFloatAttr, (setter)&DCAudioSourceSetFloatAttr, 0, (void*)9 },

	{ "timeOffset", (getter)&DCAudioSourceGetDoubleAttr, (setter)&DCAudioSourceSetDoubleAttr, 0, (void*)0 },
	{ "timePosition", (getter)&DCAudioSourceGetDoubleAttr, (setter)&DCAudioSourceSetDoubleAttr, 0, (void*)1 },

	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".AudioSource",			/* tp_name */
	sizeof(DCAudioSource),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCAudioSourceDealloc,			/* tp_dealloc */
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
	(initproc)&DCAudioSourceInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCAudioSourceNew,							/* tp_new */
};

PyTypeObject* DCAudioSourceTypeObject(void)
{
	return &objectType;
}

PyObject* DCAudioSourceFromObject(DKAudioSource* source)
{
	if (source)
	{
		DCAudioSource* self = (DCAudioSource*)DCObjectFromAddress(source);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCAudioSource*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->source = source;
				DCObjectSetAddress(self->source, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKAudioSource* DCAudioSourceToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCAudioSource*)obj)->source;
	}
	return NULL;
}
