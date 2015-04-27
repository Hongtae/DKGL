#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCAnimatedTransform.h"
#include "DCObject.h"

class DCLocalAnimationController : public DKAnimationController
{
public:
	DCLocalAnimationController(PyObject* obj) : object(obj)
	{
		DKASSERT_DEBUG(object);
	}

	void UpdateFrame(float frame) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object,
				"updateFrame", "f", frame);
			Py_XDECREF(tmp);
		});
	}
	bool GetTransform(const NodeId& key, DKTransformUnit& out) override
	{
		DKASSERT_DEBUG(this->object);

		bool result = false;
		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object,
				"getTransform", "N", PyUnicode_FromWideChar(key, -1));

			if (tmp && PyObject_TypeCheck(tmp, DCTransformUnitTypeObject()))
			{
				out = *DCTransformUnitToObject(tmp);
				result = true;
			}
			Py_XDECREF(tmp);
		});

		return result;
	}
	bool IsPlaying(void) const override
	{
		DKASSERT_DEBUG(this->object);

		bool result = false;
		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object, "isPlaying", NULL);
			if (tmp && PyObject_IsTrue(tmp) > 0)
				result = true;
			Py_XDECREF(tmp);
		});
		return result;
	}
	float Duration(void) const override
	{
		DKASSERT_DEBUG(this->object);

		float result = 0.0f;

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object, "duration", NULL);
			if (tmp && PyErr_Occurred() == NULL)
			{
				result = PyFloat_AsDouble(tmp);
				if (PyErr_Occurred())
				{
					PyErr_Clear();
					PyErr_SetString(PyExc_TypeError, "duration must return float number.");
				}
			}
			Py_XDECREF(tmp);
		});
		return Max(result, 0.0f);
	}
	void Play(void) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object, "play", NULL);
			Py_XDECREF(tmp);
		});
	}
	void Stop(void) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object, "stop", NULL);
			Py_XDECREF(tmp);
		});
	}

	PyObject* object = NULL;
};

struct DCAnimationController
{
	DCAnimatedTransform base;
	DKAnimationController* anim;
	bool localObject;
};

static PyObject* DCAnimationControllerNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCAnimationController* self = (DCAnimationController*)DCAnimatedTransformTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->anim = NULL;
		self->localObject = false;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCAnimationControllerInit(DCAnimationController *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKAnimationController> ac = NULL;
	if (self->anim == NULL)
	{
		ac = DKOBJECT_NEW DCLocalAnimationController((PyObject*)self);
		self->anim = ac;
		self->localObject = true;
	}

	self->base.anim = self->anim;
	return DCAnimatedTransformTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCAnimationControllerDealloc(DCAnimationController* self)
{
	self->anim = NULL;
	DCAnimatedTransformTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCAnimationControllerUpdate(DCAnimationController* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->anim, NULL);

	double delta;
	unsigned long tick;
	if (!PyArg_ParseTuple(args, "dk", &delta, &tick))
		return NULL;

	if (self->anim)
		self->anim->Update(delta, tick);
	Py_RETURN_NONE;
}

static PyObject* DCAnimationControllerGetTransform(DCAnimationController* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->anim, NULL);

	if (!self->localObject)
	{
		const char* node = NULL;
		if (!PyArg_ParseTuple(args, "s", &node))
			return NULL;

		DKTransformUnit unit;
		if (self->anim->GetTransform(node, unit))
		{
			return DCTransformUnitFromObject(&unit);
		}
	}
	Py_RETURN_NONE;
}

static PyObject* DCAnimationControllerIsPlaying(DCAnimationController* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->anim, NULL);

	if (!self->localObject)
	{
		return PyBool_FromLong(self->anim->IsPlaying());
	}
	Py_RETURN_FALSE;
}

static PyObject* DCAnimationControllerDuration(DCAnimationController* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->anim, NULL);

	if (!self->localObject)
	{
		return PyFloat_FromDouble(self->anim->Duration());
	}
	return PyLong_FromLong(0);
}

static PyObject* DCAnimationControllerPlay(DCAnimationController* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->anim, NULL);

	if (!self->localObject)
	{
		self->anim->Play();
	}
	Py_RETURN_NONE;
}

static PyObject* DCAnimationControllerStop(DCAnimationController* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->anim, NULL);

	if (!self->localObject)
	{
		self->anim->Stop();
	}
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "update", (PyCFunction)&DCAnimationControllerUpdate, METH_VARARGS },
	{ "updateFrame", &DCObjectMethodNone, METH_VARARGS },
	{ "getTransform", (PyCFunction)&DCAnimationControllerGetTransform, METH_VARARGS },
	{ "isPlaying", (PyCFunction)&DCAnimationControllerIsPlaying, METH_NOARGS },
	{ "duration", (PyCFunction)&DCAnimationControllerDuration, METH_NOARGS },
	{ "play", (PyCFunction)&DCAnimationControllerPlay, METH_NOARGS },
	{ "stop", (PyCFunction)&DCAnimationControllerStop, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCAnimationControllerFrame(DCAnimationController* self, void*)
{
	float frame = 0.0f;
	if (self->anim)
		frame = self->anim->Frame();
	return PyFloat_FromDouble(frame);
}

static int DCAnimationControllerSetFrame(DCAnimationController* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->anim, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double f = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be float number.");
		return -1;
	}
	self->anim->SetFrame(f);
	return 0;
}

static PyObject* DCAnimationControllerSpeed(DCAnimationController* self, void*)
{
	float speed = 0.0f;
	if (self->anim)
		speed = self->anim->Speed();
	return PyFloat_FromDouble(speed);
}

static int DCAnimationControllerSetSpeed(DCAnimationController* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->anim, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double f = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be float number.");
		return -1;
	}
	self->anim->SetSpeed(f);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "frame", (getter)&DCAnimationControllerFrame, (setter)&DCAnimationControllerSetFrame, 0, 0 },
	{ "speed", (getter)&DCAnimationControllerSpeed, (setter)&DCAnimationControllerSetSpeed, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".AnimationController",	/* tp_name */
	sizeof(DCAnimationController),				/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCAnimationControllerDealloc,	/* tp_dealloc */
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
	DCAnimatedTransformTypeObject(),			/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCAnimationControllerInit,		/* tp_init */
	0,											/* tp_alloc */
	&DCAnimationControllerNew,					/* tp_new */
};

PyTypeObject* DCAnimationControllerTypeObject(void)
{
	return &objectType;
}

PyObject* DCAnimationControllerFromObject(DKAnimationController* anim)
{
	if (anim)
	{
		DCAnimationController* self = (DCAnimationController*)DCObjectFromAddress(anim);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DKASSERT_DEBUG(dynamic_cast<DCLocalAnimationController*>(anim) == NULL);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCAnimationController*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->anim = anim;
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKAnimationController* DCAnimationControllerToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCAnimationController*)obj)->anim;
	}
	return NULL;
}
