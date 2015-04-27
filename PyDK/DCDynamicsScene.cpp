#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCScene.h"
#include "DCObject.h"

class DCLocalDynamicsScene : public DKDynamicsScene
{
public:
	DCLocalDynamicsScene(PyObject* obj) : object(obj)
	{
		DKASSERT_DEBUG(object);
	}

	bool NeedCollision(DKCollisionObject* objA, DKCollisionObject* objB) override
	{
		DKASSERT_DEBUG(this->object);

		int result = -1;
		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* obj1 = DCObjectFromAddress(objA);
			PyObject* obj2 = DCObjectFromAddress(objB);
			Py_XINCREF(obj1);
			Py_XINCREF(obj2);
			if (obj1 && obj2)
			{
				PyObject* ret = PyObject_CallMethod(object, "needCollision", "OO", obj1, obj2);
				if (ret)
					result = PyObject_IsTrue(ret);
				Py_XDECREF(ret);
			}
			Py_XDECREF(obj1);
			Py_XDECREF(obj2);
		});
		if (result < 0)
			return DKDynamicsScene::NeedCollision(objA, objB);
		return result > 0;
	}
	bool NeedResponse(DKCollisionObject* objA, DKCollisionObject* objB) override
	{
		DKASSERT_DEBUG(this->object);

		int result = -1;
		DCObjectCallPyCallableGIL([&]()
		{
			PyObject* obj1 = DCObjectFromAddress(objA);
			PyObject* obj2 = DCObjectFromAddress(objB);
			Py_XINCREF(obj1);
			Py_XINCREF(obj2);
			if (obj1 && obj2)
			{
				PyObject* ret = PyObject_CallMethod(object, "needResponse", "OO", obj1, obj2);
				if (ret)
					result = PyObject_IsTrue(ret);
				Py_XDECREF(ret);
			}
			Py_XDECREF(obj1);
			Py_XDECREF(obj2);
		});
		if (result < 0)
			return DKDynamicsScene::NeedCollision(objA, objB);
		return result > 0;
	}

	PyObject* object = NULL;
};

struct DCDynamicsScene
{
	DCScene base;
	DKDynamicsScene* scene;
};

static PyObject* DCDynamicsSceneNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCDynamicsScene* self = (DCDynamicsScene*)DCSceneTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->scene = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCDynamicsSceneInit(DCDynamicsScene *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKDynamicsScene> scene = NULL;
	if (self->scene == NULL)
	{
		scene = DKOBJECT_NEW DCLocalDynamicsScene((PyObject*)self);
		self->scene = scene;
		DCObjectSetAddress(self->scene, (PyObject*)self);
	}

	self->base.scene = self->scene;
	return DCSceneTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCDynamicsSceneDealloc(DCDynamicsScene* self)
{
	self->scene = NULL;
	DCSceneTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCDynamicsSceneGravity(DCDynamicsScene* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->scene, NULL);
	DKVector3 v = self->scene->Gravity();
	return DCVector3FromObject(&v);
}

static PyObject* DCDynamicsSceneSetGravity(DCDynamicsScene* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->scene, NULL);
	DKVector3 v;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &v))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	self->scene->SetGravity(v);
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "gravity", (PyCFunction)&DCDynamicsSceneGravity, METH_NOARGS },
	{ "setGravity", (PyCFunction)&DCDynamicsSceneSetGravity, METH_VARARGS },
	{ "needCollision", &DCObjectMethodTrue, METH_VARARGS },
	{ "needResponse", &DCObjectMethodTrue, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCDynamicsSceneFixedFrameRate(DCDynamicsScene* self, void*)
{
	DCOBJECT_VALIDATE(self->scene, NULL);
	return PyFloat_FromDouble(self->scene->FixedFrameRate());
}

static int DCDynamicsSceneSetFixedFrameRate(DCDynamicsScene* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->scene, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	double d = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be flaot.");
		return -1;
	}
	self->scene->SetFixedFrameRate(d);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "fixedFrameRate", (getter)&DCDynamicsSceneFixedFrameRate, (setter)&DCDynamicsSceneSetFixedFrameRate, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".DynamicsScene",			/* tp_name */
	sizeof(DCDynamicsScene),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCDynamicsSceneDealloc,		/* tp_dealloc */
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
	DCSceneTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCDynamicsSceneInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCDynamicsSceneNew,						/* tp_new */
};

PyTypeObject* DCDynamicsSceneTypeObject(void)
{
	return &objectType;
}

PyObject* DCDynamicsSceneFromObject(DKDynamicsScene* scene)
{
	if (scene)
	{
		DCDynamicsScene* self = (DCDynamicsScene*)DCObjectFromAddress(scene);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DKASSERT_DEBUG(dynamic_cast<DCLocalDynamicsScene*>(scene) == NULL);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCDynamicsScene*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->scene = scene;
				DCObjectSetAddress(self->scene, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKDynamicsScene* DCDynamicsSceneToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCDynamicsScene*)obj)->scene;
	}
	return NULL;
}
