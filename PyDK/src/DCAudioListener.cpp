#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCAudioListener
{
	PyObject_HEAD
	DKObject<DKAudioListener> listener;
};

static PyObject* DCAudioListenerNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCAudioListener* self = (DCAudioListener*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->listener) DKObject<DKAudioListener>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCAudioListenerInit(DCAudioListener *self, PyObject *args, PyObject *kwds)
{
	if (self->listener == NULL)
	{
		self->listener = DKAudioListener::SharedInstance();
		DCObjectSetAddress(self->listener, (PyObject*)self);
	}
	return 0;
}

static void DCAudioListenerDealloc(DCAudioListener* self)
{
	if (self->listener)
	{
		DCObjectSetAddress(self->listener, NULL);
		self->listener = NULL;
	}

	self->listener.~DKObject<DKAudioListener>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCAudioListenerSetOrientation(DCAudioListener* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->listener, NULL);

	DKASSERT_DEBUG(PyTuple_Check(args));

	Py_ssize_t numItems = PyTuple_GET_SIZE(args);
	if (numItems == 2)
	{
		DKVector3 forward, up;
		if (PyArg_ParseTuple(args, "O&O&",
			&DCVector3Converter, &forward,
			&DCVector3Converter, &up))
		{
			self->listener->SetOrientation(forward, up);
			Py_RETURN_NONE;
		}
	}
	else if (numItems == 1)
	{
		DKMatrix3 mat;
		if (PyArg_ParseTuple(args, "O&", &DCMatrix3Converter, &mat))
		{
			self->listener->SetOrientation(mat);
			Py_RETURN_NONE;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Matrix3 or two Vector3 objects.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "setOrientation", (PyCFunction)&DCAudioListenerSetOrientation, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCAudioListenerGain(DCAudioListener* self, void*)
{
	DCOBJECT_VALIDATE(self->listener, NULL);
	return PyFloat_FromDouble(self->listener->Gain());
}

static int DCAudioListenerSetGain(DCAudioListener* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->listener, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Float.");
		return -1;
	}

	self->listener->SetGain(val);
	return 0;
}

struct DCAudioListenerVector3Attr
{
	const DKVector3& (DKAudioListener::*getter)(void) const;
	void (DKAudioListener::*setter)(const DKVector3&);
};
static DCAudioListenerVector3Attr vector3Attrs[] =
{
	{ &DKAudioListener::Position, &DKAudioListener::SetPosition },
	{ &DKAudioListener::Velocity, &DKAudioListener::SetVelocity },
	{ &DKAudioListener::Forward, NULL},
	{ &DKAudioListener::Up, NULL },
};

static PyObject* DCAudioListenerGetVector3Attr(DCAudioListener* self, void* closure)
{
	DCOBJECT_VALIDATE(self->listener, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);

	DKASSERT_DEBUG(offset < (sizeof(vector3Attrs) / sizeof(vector3Attrs[0])));
	DKASSERT_DEBUG(vector3Attrs[offset].getter);

	const DKVector3& v = (self->listener->*(vector3Attrs[offset].getter))();
	return Py_BuildValue("fff", v.x, v.y, v.z);
}

static int DCAudioListenerSetVector3Attr(DCAudioListener* self, PyObject* value, void* closure)
{
	DCOBJECT_VALIDATE(self->listener, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3 v;
	if (!DCVector3Converter(value, &v))
		return -1;

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);

	DKASSERT_DEBUG(offset < (sizeof(vector3Attrs) / sizeof(vector3Attrs[0])));
	DKASSERT_DEBUG(vector3Attrs[offset].setter);

	(self->listener->*(vector3Attrs[offset].setter))(v);
	return 0;
}

static int DCAudioListenerSetForward(DCAudioListener* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->listener, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3 v;
	if (!DCVector3Converter(value, &v))
		return -1;

	DKVector3 up = self->listener->Up();
	self->listener->SetOrientation(v, up);
	return 0;
}

static int DCAudioListenerSetUp(DCAudioListener* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->listener, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3 v;
	if (!DCVector3Converter(value, &v))
		return -1;

	DKVector3 forward = self->listener->Forward();
	self->listener->SetOrientation(forward, v);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "gain", (getter)&DCAudioListenerGain, (setter)&DCAudioListenerSetGain, 0, 0 },
	{ "position", (getter)&DCAudioListenerGetVector3Attr, (setter)&DCAudioListenerSetVector3Attr, 0, (void*)0 },
	{ "velocity", (getter)&DCAudioListenerGetVector3Attr, (setter)&DCAudioListenerSetVector3Attr, 0, (void*)1 },
	{ "forward", (getter)&DCAudioListenerGetVector3Attr, (setter)&DCAudioListenerSetForward, 0, (void*)2 },
	{ "up", (getter)&DCAudioListenerGetVector3Attr, (setter)&DCAudioListenerSetUp, 0, (void*)3 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".AudioListener",			/* tp_name */
	sizeof(DCAudioListener),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCAudioListenerDealloc,		/* tp_dealloc */
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
	(initproc)&DCAudioListenerInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCAudioListenerNew,						/* tp_new */
};

PyTypeObject* DCAudioListenerTypeObject(void)
{
	return &objectType;
}

PyObject* DCAudioListenerFromObject(DKAudioListener* listener)
{
	if (listener)
	{
		DCAudioListener* self = (DCAudioListener*)DCObjectFromAddress(listener);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCAudioListener*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->listener = listener;
				DCObjectSetAddress(self->listener, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKAudioListener* DCAudioListenerToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCAudioListener*)obj)->listener;
	}
	return NULL;
}
