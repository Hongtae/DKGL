#include <Python.h>
#include <structmember.h>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include "DCScene.h"
#include "DCObject.h"

void DCScene::UpdateNodes(void)
{
	DKArray<DKModel*> models;
	models.Reserve(this->scene->NumberOfSceneObjects());
	this->scene->Enumerate(DKFunction([&](const DKModel* m)
	{
		models.Add(const_cast<DKModel*>(m));
	}));

	size_t num = models.Count();
	PyObject* tuple = PyTuple_New(num);
	for (size_t i = 0; i < num; ++i)
	{
		PyObject* obj = DCModelFromObject(models.Value(i));
		DKASSERT_DEBUG(obj);
		DKASSERT_DEBUG(PyObject_TypeCheck(obj, DCModelTypeObject()));
		PyTuple_SET_ITEM(tuple, i, obj);
	}
	Py_XDECREF(this->nodes);
	this->nodes = tuple;
}

static PyObject* DCSceneNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCScene* self = (DCScene*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->scene) DKObject<DKScene>();
		self->nodes = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCSceneInit(DCScene *self, PyObject *args, PyObject *kwds)
{
	if (self->scene == NULL)
	{
		self->scene = DKObject<DKScene>::New();
		DCObjectSetAddress(self->scene, (PyObject*)self);
	}
	self->UpdateNodes();
	return 0;
}

static int DCSceneClear(DCScene* self)
{
	Py_CLEAR(self->nodes);
	return 0;
}

static int DCSceneTraverse(DCScene* self, visitproc visit, void* arg)
{
	Py_VISIT(self->nodes);
	return 0;
}

static void DCSceneDealloc(DCScene* self)
{
	if (self->scene)
	{
		self->scene->RemoveAllObjects();
		DCObjectSetAddress(self->scene, NULL);
	}
	self->scene = NULL;

	Py_CLEAR(self->nodes);

	self->scene.~DKObject<DKScene>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCSceneAddObject(DCScene* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->scene, NULL);

	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	if (!PyObject_TypeCheck(obj, DCModelTypeObject()))
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Model object.");
		return NULL;
	}

	DKModel* model = DCModelToObject(obj);
	if (model == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid model object.");
		return NULL;
	}

	if (self->scene->AddObject(model))
	{
		self->UpdateNodes();
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCSceneRemoveAllObjects(DCScene* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->scene, NULL);

	self->scene->RemoveAllObjects();
	Py_RETURN_NONE;
}

static PyObject* DCSceneRayTest(DCScene* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->scene, NULL);

	PyObject* callback = NULL;
	DKVector3 begin, end;
	if (!PyArg_ParseTuple(args, "O&O&O",
						  &DCVector3Converter, &begin,
						  &DCVector3Converter, &end,
						  &callback))
		return NULL;

	DKVector3 hitPoint;
	DKVector3 hitNormal;
	size_t ret = 0;
	if (callback && callback != Py_None)
	{
		if (!PyCallable_Check(callback)) {
			PyErr_SetString(PyExc_TypeError, "third argument must be callable.");
			return NULL;
		}
		auto cb = [callback](DKCollisionObject* obj, float fraction, const DKVector3& normal) -> float
		{
			if (!PyErr_Occurred())
			{
				PyObject* tuple = PyTuple_New(3);
				PyTuple_SET_ITEM(tuple, 0, DCCollisionObjectFromObject(obj));
				PyTuple_SET_ITEM(tuple, 1, PyFloat_FromDouble(fraction));
				PyTuple_SET_ITEM(tuple, 2, DCVector3FromObject((DKVector3*)&normal));

				PyObject* ret = PyObject_Call(callback, tuple, NULL);

				Py_DECREF(tuple);

				if (ret)
				{
					double hitFraction = PyFloat_AsDouble(ret);
					Py_DECREF(ret);
					if (!PyErr_Occurred())
						return hitFraction;
				}
			}
			return 0.0f;
		};

		ret = self->scene->RayTest(begin, end, DKFunction(cb));
	}
	else
	{
		ret = self->scene->RayTest(begin, end, NULL);
	}
	if (PyErr_Occurred())
		return NULL;

	return PyLong_FromSize_t(ret);
}

static PyObject* DCSceneRayTestClosest(DCScene* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->scene, NULL);

	DKVector3 begin, end;
	if (!PyArg_ParseTuple(args, "O&O&",
		&DCVector3Converter, &begin,
		&DCVector3Converter, &end))
		return NULL;

	DKVector3 hitPoint;
	DKVector3 hitNormal;
	DKCollisionObject* col = self->scene->RayTestClosest(begin, end, &hitPoint, &hitNormal);
	if (col)
	{
		return Py_BuildValue("NNN",
							 DCCollisionObjectFromObject(col),
							 DCVector3FromObject(&hitPoint),
							 DCVector3FromObject(&hitNormal));
	}
	Py_RETURN_NONE;
}

static PyObject* DCSceneUpdate(DCScene* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->scene, NULL);

	double delta = 0.0;
	unsigned long tick;
	if (!PyArg_ParseTuple(args, "dk", &delta, &tick))
		return NULL;

	self->scene->Update(delta, tick);
	Py_RETURN_NONE;
}

static PyObject* DCSceneSetAmbientColor(DCScene* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->scene, NULL);

	DKColor color;
	if (!PyArg_ParseTuple(args, "O&", &DCColorConverter, &color))
		return NULL;

	self->scene->ambientColor = color;
	Py_RETURN_NONE;
}

static PyObject* DCSceneAmbientColor(DCScene* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->scene, NULL);
	return DCColorFromObject(&self->scene->ambientColor);
}

static PyObject* DCSceneSetLights(DCScene* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->scene, NULL);

	DKASSERT_DEBUG(PyTuple_Check(args));
	Py_ssize_t numItems = PyTuple_GET_SIZE(args);

	self->scene->lights.Clear();
	self->scene->lights.Reserve(numItems);
	for (int i = 0; i < numItems; ++i)
	{
		PyObject* obj = PyTuple_GET_ITEM(args, i);
		DKLight* light = DCLightToObject(obj);
		if (light == NULL)
		{
			self->scene->lights.Clear();
			PyErr_SetString(PyExc_TypeError, "argument must be Light object.");
			return NULL;
		}		
		self->scene->lights.Add(*light);
	}
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "addObject", (PyCFunction)&DCSceneAddObject, METH_VARARGS },
	{ "removeAllObjects", (PyCFunction)&DCSceneRemoveAllObjects, METH_NOARGS },
	{ "rayTest", (PyCFunction)&DCSceneRayTest, METH_VARARGS },
	{ "rayTestClosest", (PyCFunction)&DCSceneRayTestClosest, METH_VARARGS },
	{ "update", (PyCFunction)&DCSceneUpdate, METH_VARARGS },
	{ "setAmbientColor", (PyCFunction)&DCSceneSetAmbientColor, METH_VARARGS },
	{ "ambientColor", (PyCFunction)&DCSceneAmbientColor, METH_VARARGS },
	{ "setLights", (PyCFunction)&DCSceneSetLights, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCSceneDrawMode(DCScene* self, void*)
{
	DCOBJECT_VALIDATE(self->scene, NULL);
	return PyLong_FromLong(self->scene->drawMode);
}

static int DCSceneSetDrawMode(DCScene* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->scene, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	long val = PyLong_AsLong(value);
	if (PyErr_Occurred())
	{
		PyErr_SetString(PyExc_TypeError, "attribute must be Integer.");
		return -1;
	}
	self->scene->drawMode = (unsigned int)val;
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "drawMode", (getter)&DCSceneDrawMode, (setter)&DCSceneSetDrawMode, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Scene",					/* tp_name */
	sizeof(DCScene),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCSceneDealloc,				/* tp_dealloc */
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
	Py_TPFLAGS_DEFAULT | 
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCSceneTraverse,				/* tp_traverse */
	(inquiry)&DCSceneClear,						/* tp_clear */
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
	(initproc)&DCSceneInit,						/* tp_init */
	0,											/* tp_alloc */
	&DCSceneNew,								/* tp_new */
};

PyTypeObject* DCSceneTypeObject(void)
{
	return &objectType;
}

PyObject* DCSceneFromObject(DKScene* scene)
{
	if (scene)
	{
		DCScene* self = (DCScene*)DCObjectFromAddress(scene);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(DynamicsScene, scene);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCScene*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKScene* DCSceneToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCScene*)obj)->scene;
	}
	return NULL;
}
