#include <Python.h>
#include <structmember.h>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include "DCAnimatedTransform.h"
#include "DCObject.h"

class DCLocalAnimatedTransform : public DKAnimatedTransform
{
public:
	DCLocalAnimatedTransform(PyObject* obj) : object(obj)
	{
		DKASSERT_DEBUG(object);
	}

	void Update(double delta, DKTimeTick tick) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object,
				"update", "dk", delta, tick);
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

	PyObject* object = NULL;
};

static PyObject* DCAnimatedTransformNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCAnimatedTransform* self = (DCAnimatedTransform*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->anim) DKObject<DKAnimatedTransform>();
		self->localObject = false;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCAnimatedTransformInit(DCAnimatedTransform *self, PyObject *args, PyObject *kwds)
{
	if (self->anim == NULL)
	{
		self->anim = DKOBJECT_NEW DCLocalAnimatedTransform((PyObject*)self);
		self->localObject = true;
	}

	DCObjectSetAddress(self->anim, (PyObject*)self);
	return 0;
}

static void DCAnimatedTransformDealloc(DCAnimatedTransform* self)
{
	if (self->anim)
	{
		DCObjectSetAddress(self->anim, NULL);
		self->anim = NULL;
	}
	self->anim.~DKObject<DKAnimatedTransform>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCAnimatedTransformUpdate(DCAnimatedTransform* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->anim, NULL);

	if (!self->localObject)
	{
		double delta = 0.0;
		unsigned long tick;
		if (!PyArg_ParseTuple(args, "dk", &delta, &tick))
			return NULL;

		self->anim->Update(delta, tick);
	}
	Py_RETURN_NONE;
}

static PyObject* DCAnimatedTransformGetTransform(DCAnimatedTransform* self, PyObject* args)
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

static PyMethodDef methods[] = {
	{ "update", (PyCFunction)&DCAnimatedTransformUpdate, METH_VARARGS },
	{ "getTransform", (PyCFunction)&DCAnimatedTransformGetTransform, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".AnimatedTransform",		/* tp_name */
	sizeof(DCAnimatedTransform),				/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCAnimatedTransformDealloc,	/* tp_dealloc */
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
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCAnimatedTransformInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCAnimatedTransformNew,					/* tp_new */
};

PyTypeObject* DCAnimatedTransformTypeObject(void)
{
	return &objectType;
}

PyObject* DCAnimatedTransformFromObject(DKAnimatedTransform* anim)
{
	if (anim)
	{
		DCAnimatedTransform* self = (DCAnimatedTransform*)DCObjectFromAddress(anim);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DKASSERT_DEBUG(dynamic_cast<DCLocalAnimatedTransform*>(anim) == NULL);

			DCOBJECT_DYANMIC_CAST_CONVERT(AnimationController, anim);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCAnimatedTransform*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKAnimatedTransform* DCAnimatedTransformToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCAnimatedTransform*)obj)->anim;
	}
	return NULL;
}
