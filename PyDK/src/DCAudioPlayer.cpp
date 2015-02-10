#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCAudioPlayer
{
	PyObject_HEAD
	DKObject<DKAudioPlayer> player;
	PyObject* audioSource;
};

static PyObject* DCAudioPlayerNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCAudioPlayer* self = (DCAudioPlayer*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->player) DKObject<DKAudioPlayer>();
		self->audioSource = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCAudioPlayerInit(DCAudioPlayer *self, PyObject *args, PyObject *kwds)
{
	if (self->player)
		return 0;

	PyObject* source;
	if (!PyArg_ParseTuple(args, "O", &source))
		return -1;

	if (PyUnicode_Check(source))
	{
		const char* file = PyUnicode_AsUTF8(source);
		Py_BEGIN_ALLOW_THREADS
		self->player = DKAudioPlayer::Create(file);
		Py_END_ALLOW_THREADS
	}
	else if (PyObject_TypeCheck(source, DCStreamTypeObject()))
	{
		DKStream* stream = DCStreamToObject(source);
		if (stream)
		{
			Py_BEGIN_ALLOW_THREADS
			self->player = DKAudioPlayer::Create(stream);
			Py_END_ALLOW_THREADS
		}
	}
	else if (PyObject_TypeCheck(source, DCDataTypeObject()))
	{
		DKData* data = DCDataToObject(source);
		if (data)
		{
			Py_BEGIN_ALLOW_THREADS
			DKObject<DKDataStream> ds = DKOBJECT_NEW DKDataStream(data);
			self->player = DKAudioPlayer::Create(ds.SafeCast<DKStream>());
			Py_END_ALLOW_THREADS
		}
	}
	else if (PyObject_CheckBuffer(source))
	{
		Py_buffer buffer;
		if (PyObject_GetBuffer(source, &buffer, PyBUF_SIMPLE) == 0)
		{
			if (buffer.len > 0)
			{
				Py_BEGIN_ALLOW_THREADS
				DKObject<DKBuffer> data = DKBuffer::Create(buffer.buf, buffer.len);
				if (data)
				{
					DKObject<DKDataStream> ds = DKOBJECT_NEW DKDataStream(data);
					self->player = DKAudioPlayer::Create(ds.SafeCast<DKStream>());
				}
				Py_END_ALLOW_THREADS
			}
			PyBuffer_Release(&buffer);
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "argument must be String or Stream or Data object.");
		return -1;
	}

	if (self->player)
	{
		DCObjectSetAddress(self->player, (PyObject*)self);
		return 0;
	}

	PyErr_SetString(PyExc_RuntimeError, "Cannot create object from source.");
	return -1;
}

static void DCAudioPlayerDealloc(DCAudioPlayer* self)
{
	if (self->player)
	{
		DCObjectSetAddress(self->player, NULL);
		self->player = NULL;
		Py_CLEAR(self->audioSource);
	}

	self->player.~DKObject<DKAudioPlayer>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCAudioPlayerPlayLoop(DCAudioPlayer* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->player, NULL);

	double pos;
	int loops;
	if (!PyArg_ParseTuple(args, "di", &pos, &loops))
		return NULL;

	self->player->PlayLoop(pos, loops);
	Py_RETURN_NONE;
}

template <void (DKAudioPlayer::*fn)(void)>
static PyObject* CallVFunc(DCAudioPlayer* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->player, NULL);
	(self->player->*fn)();
	Py_RETURN_NONE;
}

static PyObject* DCAudioPlayerState(DCAudioPlayer* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->player, NULL);
	return PyLong_FromLong((int)self->player->State());
}

static PyObject* DCAudioPlayerAudioSource(DCAudioPlayer* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->player, NULL);
	PyObject* oldObj = self->audioSource;
	self->audioSource = NULL;

	DKAudioSource* source = self->player->AudioSource();
	if (source)
	{
		self->audioSource = DCAudioSourceFromObject(source);
	}

	Py_XDECREF(oldObj);

	if (self->audioSource)
	{
		Py_INCREF(self->audioSource);
		return self->audioSource;
	}
	Py_RETURN_NONE;
}

static PyMethodDef methods[] =
{
	{ "playLoop", (PyCFunction)&DCAudioPlayerPlayLoop, METH_VARARGS },
	{ "play", (PyCFunction)&CallVFunc<&DKAudioPlayer::Play>, METH_NOARGS },
	{ "stop", (PyCFunction)&CallVFunc<&DKAudioPlayer::Stop>, METH_NOARGS },
	{ "pause", (PyCFunction)&CallVFunc<&DKAudioPlayer::Pause>, METH_NOARGS },
	{ "state", (PyCFunction)&DCAudioPlayerState, METH_NOARGS },
	{ "audioSource", (PyCFunction)&DCAudioPlayerAudioSource, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

template <double (DKAudioPlayer::*fn)(void) const>
static PyObject* GetDoubleAttr(DCAudioPlayer* self, void*)
{
	DCOBJECT_VALIDATE(self->player, NULL);
	return PyFloat_FromDouble((self->player->*fn)());
}

template <void (DKAudioPlayer::*fn)(double)>
static int SetDoubleAttr(DCAudioPlayer* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->player, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double f = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be float number.");
		return -1;
	}
	(self->player->*fn)(f);
	return 0;
}

template <int (DKAudioPlayer::*fn)(void) const>
static PyObject* GetIntAttr(DCAudioPlayer* self, void*)
{
	DCOBJECT_VALIDATE(self->player, NULL);
	return PyLong_FromLong((self->player->*fn)());
}

static PyGetSetDef getsets[] =
{
	{ "duration", (getter)&GetDoubleAttr<&DKAudioPlayer::Duration>, NULL, 0, 0 },
	{ "timePosition", (getter)&GetDoubleAttr<&DKAudioPlayer::TimePosition>, NULL, 0, 0 },
	{ "bufferingTime", (getter)&GetDoubleAttr<&DKAudioPlayer::BufferingTime>, (setter)&SetDoubleAttr<&DKAudioPlayer::SetBufferingTime>, 0, 0 },

	{ "channels", (getter)&GetIntAttr<&DKAudioPlayer::Channels>, NULL, 0, 0 },
	{ "bits", (getter)&GetIntAttr<&DKAudioPlayer::Bits>, NULL, 0, 0 },
	{ "frequency", (getter)&GetIntAttr<&DKAudioPlayer::Frequency>, NULL, 0, 0 },

	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".AudioPlayer",			/* tp_name */
	sizeof(DCAudioPlayer),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCAudioPlayerDealloc,			/* tp_dealloc */
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
	(initproc)&DCAudioPlayerInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCAudioPlayerNew,							/* tp_new */
};

PyTypeObject* DCAudioPlayerTypeObject(void)
{
	return &objectType;
}

PyObject* DCAudioPlayerFromObject(DKAudioPlayer* player)
{
	if (player)
	{
		DCAudioPlayer* self = (DCAudioPlayer*)DCObjectFromAddress(player);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCAudioPlayer*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->player = player;
				self->audioSource = NULL;
				DCObjectSetAddress(self->player, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKAudioPlayer* DCAudioPlayerToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCAudioPlayer*)obj)->player;
	}
	return NULL;
}
