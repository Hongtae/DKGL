#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCCollisionObject.h"
#include "DCObject.h"

struct DCRigidBody
{
	DCCollisionObject base;
	DKRigidBody* body;
};

static PyObject* DCRigidBodyNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCRigidBody* self = (DCRigidBody*)DCCollisionObjectTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->body = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCRigidBodyInit(DCRigidBody *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKRigidBody> body = NULL;
	if (self->body == NULL)
	{
		PyObject* shapeObj;
		DKRigidBody::ObjectData data;
		int additionalDamping = data.additionalDamping;

		char* kwlist[] = {
			"shape",
			"mass",
			"localInertia",
			"linearDamping",
			"angularDamping",
			"friction",
			"rollingFriction",
			"restitution",
			"linearSleepingThreshold",
			"angularSleepingThreshold",
			"additionalDamping",
			"additionalDampingFactor",
			"additionalLinearDampingThresholdSqr",
			"additionalAngularDampingThresholdSqr",
			"additionalAngularDampingFactor",
			NULL };

		if (!PyArg_ParseTupleAndKeywords(args, kwds,
			"O|fO&fffffffpffff", kwlist,
			&shapeObj, &data.mass, &DCVector3Converter, &data.localInertia,
			&data.linearDamping,
			&data.angularDamping,
			&data.friction,
			&data.rollingFriction,
			&data.restitution,
			&data.linearSleepingThreshold,
			&data.angularSleepingThreshold,
			&additionalDamping,
			&data.additionalDampingFactor,
			&data.additionalLinearDampingThresholdSqr,
			&data.additionalAngularDampingThresholdSqr,
			&data.additionalAngularDampingFactor))
			return -1;

		data.additionalDamping = additionalDamping != 0;

		DKCollisionShape* shape = NULL;
		if (shapeObj != Py_None)
		{
			shape = DCCollisionShapeToObject(shapeObj);
			if (shape == NULL)
			{
				PyErr_SetString(PyExc_TypeError, "first argument must be CollisionShape object.");
				return -1;
			}
		}

		body = DKOBJECT_NEW DKRigidBody(shape, data);
		self->body = body;
		DCObjectSetAddress(self->body, (PyObject*)self);
	}

	self->base.object = self->body;
	return DCCollisionObjectTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCRigidBodyDealloc(DCRigidBody* self)
{
	self->body = NULL;
	DCCollisionObjectTypeObject()->tp_dealloc((PyObject*)self);
}

template <DKVector3 (DKRigidBody::*fn)(void) const>
static PyObject* GetVector3(DCRigidBody* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->body, NULL);
	DKVector3 v = (self->body->*fn)();
	return DCVector3FromObject(&v);
}

template <void (DKRigidBody::*fn)(const DKVector3&)>
static PyObject* SetVector3(DCRigidBody* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->body, NULL);
	DKVector3 v;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &v))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	(self->body->*fn)(v);
	Py_RETURN_NONE;
}

template <void (DKRigidBody::*fn)(const DKVector3&, const DKVector3&)>
static PyObject* SetVector3x2(DCRigidBody* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->body, NULL);
	DKVector3 v1, v2;
	if (!PyArg_ParseTuple(args, "O&O&", &DCVector3Converter, &v1, &DCVector3Converter, &v2))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3, Vector3 objects.");
		return NULL;
	}
	(self->body->*fn)(v1, v2);
	Py_RETURN_NONE;
}

template <float (DKRigidBody::*fn)(void) const>
static PyObject* GetFloat(DCRigidBody* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->body, NULL);
	float f = (self->body->*fn)();
	return PyFloat_FromDouble(f);
}

template <void (DKRigidBody::*fn)(float)>
static PyObject* SetFloat(DCRigidBody* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->body, NULL);
	float f;
	if (!PyArg_ParseTuple(args, "f", &f))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be float.");
		return NULL;
	}
	(self->body->*fn)(f);
	Py_RETURN_NONE;
}

template <void (DKRigidBody::*fn)(void)>
static PyObject* CallVFunc(DCRigidBody* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->body, NULL);
	(self->body->*fn)();
	Py_RETURN_NONE;
}

static PyObject* DCRigidBodyCenterOfMassTransform(DCRigidBody* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->body, NULL);
	DKNSTransform t = self->body->CenterOfMassTransform();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCRigidBodyCenterOfMassPosition(DCRigidBody* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->body, NULL);
	DKVector3 v = self->body->CenterOfMassPosition();
	return DCVector3FromObject(&v);
}

static PyObject* DCRigidBodyInverseWorldInertiaTensor(DCRigidBody* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->body, NULL);

	DKMatrix3 m = self->body->InverseWorldInertiaTensor();
	return DCMatrix3FromObject(&m);
}

static PyObject* DCRigidBodyVelocityInLocalPoint(DCRigidBody* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->body, NULL);

	DKVector3 pos;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &pos))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	DKVector3 v = self->body->VelocityInLocalPoint(pos);
	return DCVector3FromObject(&v);
}

static PyObject* DCRigidBodyComputeImpulseDenominator(DCRigidBody* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->body, NULL);

	DKVector3 pos, normal;
	if (!PyArg_ParseTuple(args, "O&O&", &DCVector3Converter, &pos, &DCVector3Converter, &normal))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be two Vector3 objects.");
		return NULL;
	}
	return PyFloat_FromDouble(self->body->ComputeImpulseDenominator(pos, normal));
}

static PyObject* DCRigidBodyComputeAngularImpulseDenominator(DCRigidBody* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->body, NULL);

	DKVector3 axis;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &axis))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	return PyFloat_FromDouble(self->body->ComputeAngularImpulseDenominator(axis));
}

static PyObject* DCRigidBodyComputeGyroscopicForce(DCRigidBody* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->body, NULL);

	float maxGyroscopicForce;
	if (!PyArg_ParseTuple(args, "f", &maxGyroscopicForce))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be float object.");
		return NULL;
	}
	DKVector3 v = self->body->ComputeGyroscopicForce(maxGyroscopicForce);
	return DCVector3FromObject(&v);
}


static PyMethodDef methods[] =
{
	{ "centerOfMassTransform", (PyCFunction)DCRigidBodyCenterOfMassTransform, METH_NOARGS },
	{ "centerOfMassPosition", (PyCFunction)DCRigidBodyCenterOfMassPosition, METH_NOARGS },

	{ "setMass", (PyCFunction)&SetFloat<&DKRigidBody::SetMass>, METH_VARARGS },
	{ "mass", (PyCFunction)&GetFloat<&DKRigidBody::Mass>, METH_NOARGS },
	{ "inverseMass", (PyCFunction)&GetFloat<&DKRigidBody::InverseMass>, METH_NOARGS },

	{ "setLocalInertia", (PyCFunction)&SetVector3<&DKRigidBody::SetLocalInertia>, METH_VARARGS },
	{ "localInertia", (PyCFunction)&GetVector3<&DKRigidBody::LocalInertia>, METH_NOARGS },
	{ "inverseDiagLocalInertia", (PyCFunction)&GetVector3<&DKRigidBody::InverseDiagLocalInertia>, METH_NOARGS },
	{ "inverseWorldInertiaTensor", (PyCFunction)&DCRigidBodyInverseWorldInertiaTensor, METH_NOARGS },

	{ "setLinearVelocity", (PyCFunction)&SetVector3<&DKRigidBody::SetLinearVelocity>, METH_VARARGS },
	{ "linearVelocity", (PyCFunction)&GetVector3<&DKRigidBody::LinearVelocity>, METH_NOARGS },

	{ "setAngularVelocity", (PyCFunction)&SetVector3<&DKRigidBody::SetAngularVelocity>, METH_VARARGS },
	{ "angularVelocity", (PyCFunction)&GetVector3<&DKRigidBody::AngularVelocity>, METH_NOARGS },

	{ "setLinearFactor", (PyCFunction)&SetVector3<&DKRigidBody::SetLinearFactor>, METH_VARARGS },
	{ "linearFactor", (PyCFunction)&GetVector3<&DKRigidBody::LinearFactor>, METH_NOARGS },

	{ "setAngularFactor", (PyCFunction)&SetVector3<&DKRigidBody::SetAngularFactor>, METH_VARARGS },
	{ "angularFactor", (PyCFunction)&GetVector3<&DKRigidBody::AngularFactor>, METH_NOARGS },

	{ "setLinearDamping", (PyCFunction)&SetFloat<&DKRigidBody::SetLinearDamping>, METH_VARARGS },
	{ "linearDamping", (PyCFunction)&GetFloat<&DKRigidBody::LinearDamping>, METH_NOARGS },

	{ "setAngularDamping", (PyCFunction)&SetFloat<&DKRigidBody::SetAngularDamping>, METH_VARARGS },
	{ "angularDamping", (PyCFunction)&GetFloat<&DKRigidBody::AngularDamping>, METH_NOARGS },

	{ "totalForce", (PyCFunction)&GetVector3<&DKRigidBody::TotalForce>, METH_NOARGS },
	{ "totalTorque", (PyCFunction)&GetVector3<&DKRigidBody::TotalTorque>, METH_NOARGS },

	{ "velocityInLocalPoint", (PyCFunction)&DCRigidBodyVelocityInLocalPoint, METH_VARARGS },

	{ "computeImpulseDenominator", (PyCFunction)&DCRigidBodyComputeImpulseDenominator, METH_VARARGS },
	{ "computeAngularImpulseDenominator", (PyCFunction)&DCRigidBodyComputeAngularImpulseDenominator, METH_VARARGS },
	{ "computeGyroscopicForce", (PyCFunction)&DCRigidBodyComputeGyroscopicForce, METH_VARARGS },

	{ "applyForce", (PyCFunction)&SetVector3x2<&DKRigidBody::ApplyForce>, METH_VARARGS },
	{ "applyImpulse", (PyCFunction)&SetVector3x2<&DKRigidBody::ApplyImpulse>, METH_VARARGS },

	{ "applyTorque", (PyCFunction)&SetVector3<&DKRigidBody::ApplyTorque>, METH_VARARGS },
	{ "applyCentralForce", (PyCFunction)&SetVector3<&DKRigidBody::ApplyCentralForce>, METH_VARARGS },
	{ "applyCentralImpulse", (PyCFunction)&SetVector3<&DKRigidBody::ApplyCentralImpulse>, METH_VARARGS },
	{ "applyTorqueImpulse", (PyCFunction)&SetVector3<&DKRigidBody::ApplyTorqueImpulse>, METH_VARARGS },

	{ "clearForces", (PyCFunction)&CallVFunc<&DKRigidBody::ClearForces>, METH_NOARGS },

	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

template <float (DKRigidBody::*fn)(void) const>
static PyObject* GetFloatAttr(DCRigidBody* self, void*)
{
	return GetFloat<fn>(self, NULL);
}

template <void (DKRigidBody::*fn)(float)>
static int SetFloatAttr(DCRigidBody* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->body, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Float.");
		return -1;
	}
	(self->body->*fn)(val);
	return 0;
}

static PyGetSetDef getsets[] =
{
	{ "linearSleepingThreshold", (getter)&GetFloatAttr<&DKRigidBody::LinearSleepingThreshold>, (setter)&SetFloatAttr<&DKRigidBody::SetLinearSleepingThreshold>, 0, 0 },
	{ "angularSleepingThreshold", (getter)&GetFloatAttr<&DKRigidBody::AngularSleepingThreshold>, (setter)&SetFloatAttr<&DKRigidBody::SetAngularSleepingThreshold>, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".RigidBody",				/* tp_name */
	sizeof(DCRigidBody),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCRigidBodyDealloc,			/* tp_dealloc */
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
	0,											/* tp_as_body */
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
	DCCollisionObjectTypeObject(),				/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCRigidBodyInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCRigidBodyNew,								/* tp_new */
};

PyTypeObject* DCRigidBodyTypeObject(void)
{
	return &objectType;
}

PyObject* DCRigidBodyFromObject(DKRigidBody* body)
{
	if (body)
	{
		DCRigidBody* self = (DCRigidBody*)DCObjectFromAddress(body);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCRigidBody*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->body = body;
				DCObjectSetAddress(self->body, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKRigidBody* DCRigidBodyToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCRigidBody*)obj)->body;
	}
	return NULL;
}
