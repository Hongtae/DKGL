#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCCollisionObject.h"
#include "DCObject.h"

void DCCollisionObject::UpdateCollisionShape(void)
{
	PyObject* old = this->collisionShape;
	DKCollisionShape* shape = this->object->CollisionShape();
	if (shape)
	{
		PyObject* obj = DCCollisionShapeFromObject(shape);
		DKASSERT_DEBUG(DCCollisionShapeToObject(obj) == shape);
		this->collisionShape = obj;
	}
	else
	{
		this->collisionShape = NULL;
	}
	Py_XDECREF(old);
}

static PyObject* DCCollisionObjectNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCCollisionObject* self = (DCCollisionObject*)DCModelTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->object = NULL;
		self->collisionShape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCCollisionObjectInit(DCCollisionObject *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKCollisionObject> co = NULL;
	if (self->object == NULL)
	{
		co = DKOBJECT_NEW DKCollisionObject();
		self->object = co;
		DCObjectSetAddress(self->object, (PyObject*)self);
	}
	self->UpdateCollisionShape();
	self->base.model = self->object;
	return DCModelTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static int DCCollisionObjectClear(DCCollisionObject* self)
{
	Py_CLEAR(self->collisionShape);
	return DCModelTypeObject()->tp_clear((PyObject*)self);
}

static int DCCollisionObjectTraverse(DCCollisionObject* self, visitproc visit, void* arg)
{
	Py_VISIT(self->collisionShape);
	return DCModelTypeObject()->tp_traverse((PyObject*)self, visit, arg);
}

static void DCCollisionObjectDealloc(DCCollisionObject* self)
{
	Py_CLEAR(self->collisionShape);
	self->object = NULL;
	DCModelTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCCollisionObjectActivate(DCCollisionObject* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	int force = 0;
	if (!PyArg_ParseTuple(args, "|p", &force))
		return NULL;

	self->object->Activate(force != 0);
	Py_RETURN_NONE;
}

static PyObject* DCCollisionObjectKeepActivating(DCCollisionObject* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	int active = 0;
	if (!PyArg_ParseTuple(args, "p", &active))
		return NULL;

	self->object->KeepActivating(active != 0);
	Py_RETURN_NONE;
}

static PyObject* DCCollisionObjectSetKinematic(DCCollisionObject* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	int kinematic = 0;
	if (!PyArg_ParseTuple(args, "p", &kinematic))
		return NULL;

	self->object->SetKinematic(kinematic != 0);
	Py_RETURN_NONE;
}

static PyObject* DCCollisionObjectSetContactResponse(DCCollisionObject* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	int response = 0;
	if (!PyArg_ParseTuple(args, "p", &response))
		return NULL;

	self->object->SetContactResponse(response != 0);
	Py_RETURN_NONE;
}

static PyObject* DCCollisionObjectIsActive(DCCollisionObject* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	return PyBool_FromLong(self->object->IsActive());
}

static PyObject* DCCollisionObjectIsStatic(DCCollisionObject* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	return PyBool_FromLong(self->object->IsStatic());
}

static PyObject* DCCollisionObjectIsKinematic(DCCollisionObject* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	return PyBool_FromLong(self->object->IsKinematic());
}

static PyObject* DCCollisionObjectHasContactResponse(DCCollisionObject* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	return PyBool_FromLong(self->object->HasContactResponse());
}

static PyObject* DCCollisionObjectCollisionShape(DCCollisionObject* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	if (self->collisionShape)
	{
		Py_INCREF(self->collisionShape);
		return self->collisionShape;
	}
	Py_RETURN_NONE;
}

static PyObject* DCCollisionObjectSetCollisionShape(DCCollisionObject* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKCollisionShape* shape = NULL;
	if (obj != Py_None)
	{
		shape = DCCollisionShapeToObject(obj);
		if (shape == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument must be CollisionShape object or None.");
			return NULL;
		}
	}
	self->object->SetCollisionShape(shape);
	self->UpdateCollisionShape();
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "activate", (PyCFunction)&DCCollisionObjectActivate, METH_VARARGS },
	{ "keepActivating", (PyCFunction)&DCCollisionObjectKeepActivating, METH_VARARGS },
	{ "setKinematic", (PyCFunction)&DCCollisionObjectSetKinematic, METH_VARARGS },
	{ "setContactResponse", (PyCFunction)&DCCollisionObjectSetContactResponse, METH_VARARGS },
	{ "isActive", (PyCFunction)&DCCollisionObjectIsActive, METH_NOARGS },
	{ "isStatic", (PyCFunction)&DCCollisionObjectIsStatic, METH_NOARGS },
	{ "isKinematic", (PyCFunction)&DCCollisionObjectIsKinematic, METH_NOARGS },
	{ "hasContactResponse", (PyCFunction)&DCCollisionObjectHasContactResponse, METH_NOARGS },
	{ "setCollisionShape", (PyCFunction)&DCCollisionObjectSetCollisionShape, METH_VARARGS },
	{ "collisionShape", (PyCFunction)&DCCollisionObjectCollisionShape, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

struct DCCollisionObjectMethod
{
	void (DKCollisionObject::*setter)(float);
	float(DKCollisionObject::*getter)(void) const;
};

static DCCollisionObjectMethod floatAttrs[] = {
	{ &DKCollisionObject::SetRestitution, &DKCollisionObject::Restitution },
	{ &DKCollisionObject::SetFriction, &DKCollisionObject::Friction },
	{ &DKCollisionObject::SetRollingFriction, &DKCollisionObject::RollingFriction },
	{ &DKCollisionObject::SetHitFraction, &DKCollisionObject::HitFraction },
};

static PyObject* DCCollisionObjectGetAttr(DCCollisionObject* self, void* closure)
{
	DCOBJECT_VALIDATE(self->object, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(floatAttrs) / sizeof(floatAttrs[0])));
	DKASSERT_DEBUG(floatAttrs[offset].getter);

	float v = (self->object->*(floatAttrs[offset].getter))();
	return PyFloat_FromDouble(v);
}

static int DCCollisionObjectSetAttr(DCCollisionObject* self, PyObject* value, void* closure)
{
	DCOBJECT_VALIDATE(self->object, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Float.");
		return -1;
	}

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	DKASSERT_DEBUG(offset < (sizeof(floatAttrs) / sizeof(floatAttrs[0])));
	DKASSERT_DEBUG(floatAttrs[offset].setter);

	(self->object->*(floatAttrs[offset].setter))(val);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "restitution", (getter)&DCCollisionObjectGetAttr, (setter)&DCCollisionObjectSetAttr, 0, (void*)0 },
	{ "friction", (getter)&DCCollisionObjectGetAttr, (setter)&DCCollisionObjectSetAttr, 0, (void*)1 },
	{ "rollingFriction", (getter)&DCCollisionObjectGetAttr, (setter)&DCCollisionObjectSetAttr, 0, (void*)2 },
	{ "hitFraction", (getter)&DCCollisionObjectGetAttr, (setter)&DCCollisionObjectSetAttr, 0, (void*)3 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".CollisionObject",		/* tp_name */
	sizeof(DCCollisionObject),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCCollisionObjectDealloc,		/* tp_dealloc */
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
	0,											/* tp_as_object */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCCollisionObjectTraverse,	/* tp_traverse */
	(inquiry)&DCCollisionObjectClear,			/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	getsets,									/* tp_getset */
	DCModelTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCCollisionObjectInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCCollisionObjectNew,						/* tp_new */
};

PyTypeObject* DCCollisionObjectTypeObject(void)
{
	return &objectType;
}

PyObject* DCCollisionObjectFromObject(DKCollisionObject* object)
{
	if (object)
	{
		DCCollisionObject* self = (DCCollisionObject*)DCObjectFromAddress(object);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(RigidBody, object);
			DCOBJECT_DYANMIC_CAST_CONVERT(SoftBody, object);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCCollisionObject*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->object = object;
				DCObjectSetAddress(self->object, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKCollisionObject* DCCollisionObjectToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCCollisionObject*)obj)->object;
	}
	return NULL;
}
