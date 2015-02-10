#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCModel.h"
#include "DCObject.h"

class DCLocalActionController : public DKActionController
{
public:
	DCLocalActionController(PyObject* obj) : object(obj)
	{
		DKASSERT_DEBUG(object);
	}
#if 0
	void OnAddedToScene(void) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object,
				"onAddedToScene", NULL);
			Py_XDECREF(tmp);
		});
	}
	void OnRemovedFromScene(void) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object,
				"onRemovedFromScene", NULL);
			Py_XDECREF(tmp);
		});
	}
	void OnAddedToParent(void) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object,
				"onAddedToParent", NULL);
			Py_XDECREF(tmp);
		});
	}
	void OnRemovedFromParent(void) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* tmp = PyObject_CallMethod(object,
				"onRemovedFromParent", NULL);
			Py_XDECREF(tmp);
		});
	}
	void OnSetAnimation(DKAnimatedTransform* anim) override
	{
		DKASSERT_DEBUG(this->object);

		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* animObj = DCAnimatedTransformFromObject(anim);
			DKASSERT_DEBUG(DCAnimatedTransformToObject(animObj) == anim);

			PyObject* tmp = PyObject_CallMethod(object,
				"onSetAnimation", "N", animObj);
			Py_XDECREF(tmp);
		});
	}
#endif
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

	PyObject* object = NULL;
};

struct DCActionController
{
	DCModel base;
	DKActionController* controller;
	bool localObject;
};

static PyObject* DCActionControllerNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCActionController* self = (DCActionController*)DCModelTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->controller = NULL;
		self->localObject = false;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCActionControllerInit(DCActionController *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKActionController> con = NULL;
	if (self->controller == NULL)
	{
		con = DKOBJECT_NEW DCLocalActionController((PyObject*)self);
		self->controller = con;
		self->localObject = true;
		DCObjectSetAddress(self->controller, (PyObject*)self);
	}

	self->base.model = self->controller;
	return DCModelTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCActionControllerDealloc(DCActionController* self)
{
	self->controller = NULL;
	DCModelTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCActionControllerUpdate(DCActionController* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->controller, NULL);

	if (!self->localObject)
	{
		double delta = 0.0;
		unsigned long tick;
		if (!PyArg_ParseTuple(args, "dk", &delta, &tick))
			return NULL;

		self->controller->Update(delta, tick);
	}
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
#if 0
	{ "onAddedToScene", (PyCFunction)&DCObjectMethodNone, METH_NOARGS },
	{ "onRemovedFromScene", (PyCFunction)&DCObjectMethodNone, METH_NOARGS },
	{ "onAddedToParent", (PyCFunction)&DCObjectMethodNone, METH_NOARGS },
	{ "onRemovedFromParent", (PyCFunction)&DCObjectMethodNone, METH_NOARGS },
	{ "onSetAnimation", (PyCFunction)&DCObjectMethodNone, METH_VARARGS },
#endif
	{ "update", (PyCFunction)&DCActionControllerUpdate, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ActionController",		/* tp_name */
	sizeof(DCActionController),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCActionControllerDealloc,		/* tp_dealloc */
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
	DCModelTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCActionControllerInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCActionControllerNew,						/* tp_new */
};

PyTypeObject* DCActionControllerTypeObject(void)
{
	return &objectType;
}

PyObject* DCActionControllerFromObject(DKActionController* controller)
{
	if (controller)
	{
		DCActionController* self = (DCActionController*)DCObjectFromAddress(controller);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DKASSERT_DEBUG(dynamic_cast<DCLocalActionController*>(controller) == NULL);
			
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCActionController*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->controller = controller;
				DCObjectSetAddress(self->controller, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKActionController* DCActionControllerToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCActionController*)obj)->controller;
	}
	return NULL;
}
