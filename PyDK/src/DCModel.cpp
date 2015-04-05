#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCModel.h"
#include "DCScene.h"
#include "DCObject.h"

void DCModel::UpdateChildren(void)
{
	size_t num = this->model->NumberOfChildren();
	PyObject* tuple = PyTuple_New(num);
	for (unsigned int i = 0; i < num; ++i)
	{
		DKModel* child = this->model->ChildAtIndex(i);
		DKASSERT_DEBUG(child);
		PyObject* obj = DCModelFromObject(child);
		DKASSERT_DEBUG(obj);
		DKASSERT_DEBUG(DCModelToObject(obj) == child);

		PyTuple_SET_ITEM(tuple, i, obj);
	}

	Py_XDECREF(this->children);
	this->children = tuple;
}

void DCModel::UpdateAnimation(void)
{
	PyObject* old = this->animation;
	this->animation = NULL;
	DKAnimatedTransform* anim = this->model->Animation();
	if (anim)
	{
		this->animation = DCAnimatedTransformFromObject(anim);
		DKASSERT_DEBUG(DCAnimatedTransformToObject(this->animation) == anim);
	}
	Py_XDECREF(old);
}

static PyObject* DCModelNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCModel* self = (DCModel*)DCResourceTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->model = NULL;
		self->children = NULL;
		self->animation = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCModelInit(DCModel *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKModel> model = NULL;
	if (self->model == NULL)
	{
		model = DKOBJECT_NEW DKModel();
		self->model = model;
		DCObjectSetAddress(self->model, (PyObject*)self);
	}
	else
	{
		self->UpdateChildren();
	}

	self->base.resource = self->model;
	return DCResourceTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static int DCModelClear(DCModel* self)
{
	if (self->model)
	{
		self->model->RemoveFromParent();
		self->model->RemoveFromScene();
	}

	Py_CLEAR(self->animation);
	Py_CLEAR(self->children);
	return 0;
}

static int DCModelTraverse(DCModel* self, visitproc visit, void* arg)
{
	Py_VISIT(self->children);
	Py_VISIT(self->animation);
	return 0;
}

static void DCModelDealloc(DCModel* self)
{
	DCModelClear(self);
	if (self->model)
	{
		DKASSERT_DEBUG(self->model->Parent() == NULL);
		DKASSERT_DEBUG(self->model->Scene() == NULL);
	}
	self->model = NULL;
	DCResourceTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCModelScene(DCModel* self, PyObject*)
{
	if (self->model)
	{
		DKScene* s = self->model->Scene();
		if (s)
			return DCSceneFromObject(s);
	}
	Py_RETURN_NONE;
}

static PyObject* DCModelParent(DCModel* self, PyObject*)
{
	if (self->model)
	{
		DKModel* parent = self->model->Parent();
		if (parent)
			return DCModelFromObject(parent);
		DKScene* scene = self->model->Scene();
		if (scene)
			return DCSceneFromObject(scene);
	}
	Py_RETURN_NONE;
}

static PyObject* DCModelAddChild(DCModel* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->model, NULL);

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

	if (self->model->AddChild(model))
	{
		self->UpdateChildren();
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject* DCModelChildren(DCModel* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->model, NULL);

	if (self->children == NULL)
		self->UpdateChildren();
	DKASSERT_DEBUG(self->children);
	DKASSERT_DEBUG(PyTuple_Check(self->children));

	Py_INCREF(self->children);
	return self->children;
}

static PyObject* DCModelNumberOfChildren(DCModel* self, PyObject*)
{
	size_t num = 0;
	if (self->model)
		num = self->model->NumberOfChildren();
	return PyLong_FromSize_t(num);
}

static PyObject* DCModelNumberOfDescendants(DCModel* self, PyObject*)
{
	size_t num = 0;
	if (self->model)
		num = self->model->NumberOfDescendants();
	return PyLong_FromSize_t(num);
}

static PyObject* DCModelFindDescendant(DCModel* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->model, NULL);

	const char* name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	DKModel* model = self->model->FindDescendant(name);
	if (model)
		return DCModelFromObject(model);
	Py_RETURN_NONE;
}

static PyObject* DCModelRemoveFromParent(DCModel* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->model, NULL);
	DKModel* parent = self->model->Parent();
	if (parent)
	{
		self->model->RemoveFromParent();
		PyObject* obj = DCObjectFromAddress(parent);
		if (obj)
		{
			Py_INCREF(obj);
			DKASSERT_DEBUG(DCModelToObject(obj) == parent);
			((DCModel*)obj)->UpdateChildren();
			Py_DECREF(obj);
		}
	}
	DKScene* scene = self->model->Scene();
	if (scene)
	{
		self->model->RemoveFromScene();
		PyObject* obj = DCObjectFromAddress(scene);
		if (obj)
		{
			Py_INCREF(obj);
			DKASSERT_DEBUG(DCSceneToObject(obj) == scene);
			((DCScene*)obj)->UpdateNodes();
			Py_DECREF(obj);
		}
	}
	Py_RETURN_NONE;
}

static PyObject* DCModelDidAncestorHideDescendants(DCModel* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->model, NULL);
	return PyBool_FromLong(self->model->DidAncestorHideDescendants());
}

static PyObject* DCModelWorldTransform(DCModel* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->model, NULL);
	const DKNSTransform& t = self->model->WorldTransform();
	return DCNSTransformFromObject(const_cast<DKNSTransform*>(&t));
}

static PyObject* DCModelSetWorldTransform(DCModel* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->model, NULL);
	DKNSTransform trans;
	if (!PyArg_ParseTuple(args, "O&", &DCNSTransformConverter, &trans))
		return NULL;

	self->model->SetWorldTransform(trans);
	Py_RETURN_NONE;
}

static PyObject* DCModelLocalTransform(DCModel* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->model, NULL);
	const DKNSTransform& t = self->model->LocalTransform();
	return DCNSTransformFromObject(const_cast<DKNSTransform*>(&t));
}

static PyObject* DCModelSetLocalTransform(DCModel* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->model, NULL);

	DKNSTransform trans;
	if (!PyArg_ParseTuple(args, "O&", &DCNSTransformConverter, &trans))
		return NULL;

	self->model->SetLocalTransform(trans);
	Py_RETURN_NONE;
}

static PyObject* DCModelAnimation(DCModel* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->model, NULL);
	self->UpdateAnimation();
	if (self->animation)
	{
		Py_INCREF(self->animation);
		return self->animation;
	}
	Py_RETURN_NONE;
}

static PyObject* DCModelSetAnimation(DCModel* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->model, NULL);

	PyObject* obj;
	int recursive = 1;
	if (!PyArg_ParseTuple(args, "O|p", &obj, recursive))
		return NULL;

	DKAnimatedTransform* anim = NULL;
	if (obj != Py_None)
	{
		anim = DCAnimatedTransformToObject(obj);
		if (anim == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument must be AnimatedTransform object.");
			return NULL;
		}
	}
	self->model->SetAnimation(anim, recursive != 0);

	if (recursive)
	{
		struct AnimUpdate
		{
			DCModel* target;
			AnimUpdate(DCModel* m) : target(m) {}
			size_t Update(void)
			{
				size_t numUpdated = 1;
				target->UpdateAnimation();
				if (target->children == NULL)
					target->UpdateChildren();
				DKASSERT_DEBUG(target->children);
				DKASSERT_DEBUG(PyTuple_Check(target->children));
				Py_ssize_t num = PyTuple_GET_SIZE(target->children);
				for (Py_ssize_t i = 0; i < num; ++i)
				{
					PyObject* c = PyTuple_GET_ITEM(target->children, i);
					DKASSERT_DEBUG(c);
					DKASSERT_DEBUG(PyObject_TypeCheck(c, DCModelTypeObject()));
					numUpdated += AnimUpdate((DCModel*)c).Update();
				}
				return numUpdated;
			}
		};
		size_t numUpdated = AnimUpdate(self).Update();
	}
	else
	{
		self->UpdateAnimation();
	}

	Py_RETURN_NONE;
}

static PyObject* DCModelClone(DCModel* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->model, NULL);

	DKObject<DKModel> model = self->model->Clone();
	if (model)
		return DCModelFromObject(model);
	
	PyErr_SetString(PyExc_RuntimeError, "DKModel::Clone failed!");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "scene", (PyCFunction)&DCModelScene, METH_VARARGS },
	{ "parent", (PyCFunction)&DCModelParent, METH_VARARGS },
	{ "addChild", (PyCFunction)&DCModelAddChild, METH_VARARGS },
	{ "children", (PyCFunction)&DCModelChildren, METH_NOARGS },
	{ "numberOfChildren", (PyCFunction)&DCModelNumberOfChildren, METH_NOARGS },
	{ "numberOfDescendants", (PyCFunction)&DCModelNumberOfDescendants, METH_NOARGS },	
	{ "findDescendant", (PyCFunction)&DCModelFindDescendant, METH_VARARGS },
	{ "removeFromParent", (PyCFunction)&DCModelRemoveFromParent, METH_NOARGS },
	{ "didAncestorHideDescendants", (PyCFunction)&DCModelDidAncestorHideDescendants, METH_NOARGS },
	{ "worldTransform", (PyCFunction)&DCModelWorldTransform, METH_NOARGS },
	{ "setWorldTransform", (PyCFunction)&DCModelSetWorldTransform, METH_VARARGS },
	{ "localTransform", (PyCFunction)&DCModelLocalTransform, METH_NOARGS },
	{ "setLocalTransform", (PyCFunction)&DCModelSetLocalTransform, METH_VARARGS },
	{ "animation", (PyCFunction)&DCModelAnimation, METH_NOARGS },
	{ "setAnimation", (PyCFunction)&DCModelSetAnimation, METH_VARARGS },
	{ "clone", (PyCFunction)&DCModelClone, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCModelDescendantsHidden(DCModel* self, void*)
{
	DCOBJECT_VALIDATE(self->model, NULL);
	return PyBool_FromLong(self->model->AreDescendantsHidden());
}

static int DCModelSetDescendantsHidden(DCModel* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->model, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int h = PyObject_IsTrue(value);
	if (h < 0)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}
	self->model->SetDescendantsHidden(h != 0);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "descendantsHidden", (getter)&DCModelDescendantsHidden, (setter)&DCModelSetDescendantsHidden, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Model",					/* tp_name */
	sizeof(DCModel),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCModelDealloc,				/* tp_dealloc */
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
	0,											/* tp_as_model */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCModelTraverse,				/* tp_traverse */
	(inquiry)&DCModelClear,						/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	getsets,									/* tp_getset */
	DCResourceTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCModelInit,						/* tp_init */
	0,											/* tp_alloc */
	&DCModelNew,								/* tp_new */
};

PyTypeObject* DCModelTypeObject(void)
{
	return &objectType;
}

PyObject* DCModelFromObject(DKModel* model)
{
	if (model)
	{
		DCModel* self = (DCModel*)DCObjectFromAddress(model);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(ActionController, model);
			DCOBJECT_DYANMIC_CAST_CONVERT(CollisionObject, model);
			DCOBJECT_DYANMIC_CAST_CONVERT(Constraint, model);
			DCOBJECT_DYANMIC_CAST_CONVERT(Mesh, model);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCModel*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->model = model;
				DCObjectSetAddress(self->model, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKModel* DCModelToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCModel*)obj)->model;
	}
	return NULL;
}
