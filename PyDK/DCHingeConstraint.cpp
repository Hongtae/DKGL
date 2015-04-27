#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConstraint.h"
#include "DCObject.h"

struct DCHingeConstraint
{
	DCConstraint base;
	DKHingeConstraint* constraint;
};

static PyObject* DCHingeConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCHingeConstraint* self = (DCHingeConstraint*)DCConstraintTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCHingeConstraintInit(DCHingeConstraint *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKHingeConstraint> con = NULL;
	if (self->constraint == NULL)
	{
		PyObject* objA;
		PyObject* objB;
		DKNSTransform ta, tb;
		char* kwlist[] = { "bodyA", "bodyB", "frameA", "frameB", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO&O&", kwlist,
			&objA, &objB, &DCNSTransformConverter, &ta, &DCNSTransformConverter, &tb))
			return -1;

		auto getBody = [](PyObject* obj, const char* err)
		{
			DKRigidBody* body = NULL;
			if (obj != Py_None)
			{
				body = DCRigidBodyToObject(obj);
				if (body == NULL)
				{
					PyErr_SetString(PyExc_TypeError, err);
				}
			}
			return body;
		};
		DKRigidBody* rbA = getBody(objA, "first arugment must be RigidBody object.");
		if (PyErr_Occurred()) return -1;

		DKRigidBody* rbB = getBody(objB, "second argument must be RigidBody object.");
		if (PyErr_Occurred()) return -1;

		con = DKOBJECT_NEW DKHingeConstraint(rbA, rbB, ta, tb);
		self->constraint = con;
		DCObjectSetAddress(self->constraint, (PyObject*)self);
	}
	self->base.constraint = self->constraint;
	return DCConstraintTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCHingeConstraintDealloc(DCHingeConstraint* self)
{
	self->constraint = NULL;
	DCConstraintTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCHingeConstraintFrameA(DCHingeConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameA();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCHingeConstraintFrameB(DCHingeConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameB();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCHingeConstraintSetFrameA(DCHingeConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform ta;
	if (!PyArg_ParseTuple(args, "O&", &DCNSTransformConverter, &ta))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be NSTransform object.");
		return NULL;
	}
	DKNSTransform tb = self->constraint->FrameB();
	self->constraint->SetFrames(ta, tb);
	Py_RETURN_NONE;
}

static PyObject* DCHingeConstraintSetFrameB(DCHingeConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform tb;
	if (!PyArg_ParseTuple(args, "O&", &DCNSTransformConverter, &tb))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be NSTransform object.");
		return NULL;
	}
	DKNSTransform ta = self->constraint->FrameA();
	self->constraint->SetFrames(ta, tb);
	Py_RETURN_NONE;
}

static PyObject* DCHingeConstraintSetFrames(DCHingeConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform ta, tb;
	if (!PyArg_ParseTuple(args, "O&O&", &DCNSTransformConverter, &ta, &DCNSTransformConverter, &tb))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be two NSTransform objects.");
		return NULL;
	}
	self->constraint->SetFrames(ta, tb);
	Py_RETURN_NONE;
}

static PyObject* DCHingeConstraintSetLimit(DCHingeConstraint* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	float lower;
	float upper;
	float softness = 0.9f;
	float biasFactor = 0.3f;
	float relaxationFactor = 1.0f;
	char* kwlist[] = { "lower", "upper", "softness", "biasFactor", "relaxationFactor", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ff|fff", kwlist,
		&lower, &upper, &softness, &biasFactor, &relaxationFactor))
		return NULL;

	self->constraint->SetLimit(lower, upper, softness, biasFactor, relaxationFactor);
	Py_RETURN_NONE;
}

static PyObject* DCHingeConstraintSetAngularMotor(DCHingeConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int enable;
	float velocity;
	float impulse;
	if (!PyArg_ParseTuple(args, "pff", &enable, &velocity, &impulse))
		return NULL;

	self->constraint->SetAngularMotor(enable, velocity, impulse);
	Py_RETURN_NONE;
}

static PyObject* DCHingeConstraintHingeAngle(DCHingeConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return PyFloat_FromDouble(self->constraint->HingeAngle());
}

static PyMethodDef methods[] = {
	{ "frameA", (PyCFunction)&DCHingeConstraintFrameA, METH_NOARGS },
	{ "frameB", (PyCFunction)&DCHingeConstraintFrameB, METH_NOARGS },
	{ "setFrameA", (PyCFunction)&DCHingeConstraintSetFrameA, METH_VARARGS },
	{ "setFrameB", (PyCFunction)&DCHingeConstraintSetFrameB, METH_VARARGS },
	{ "setFrames", (PyCFunction)&DCHingeConstraintSetFrames, METH_VARARGS },
	{ "setLimit", (PyCFunction)&DCHingeConstraintSetLimit, METH_VARARGS | METH_KEYWORDS },
	{ "setAngularMotor", (PyCFunction)&DCHingeConstraintSetAngularMotor, METH_VARARGS },
	{ "hingeAngle", (PyCFunction)&DCHingeConstraintHingeAngle, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

struct FloatAttr
{
	float (DKHingeConstraint::*getter)() const;
	void(*setter)(DKHingeConstraint*,float);
};
static FloatAttr floatAttrs[] = {
	{	/* lowerLimit */
		&DKHingeConstraint::LowerLimit,
		[](DKHingeConstraint* c, float lower)
		{
			float upper = c->UpperLimit();
			float softness = c->Softness();
			float biasFactor = c->BiasFactor();
			float relaxationFactor = c->RelaxationFactor();
			c->SetLimit(lower, upper, softness, biasFactor, relaxationFactor);
		}
	},
	{	/* upperLimit */
		&DKHingeConstraint::UpperLimit,
		[](DKHingeConstraint* c, float upper)
		{
			float lower = c->LowerLimit();
			float softness = c->Softness();
			float biasFactor = c->BiasFactor();
			float relaxationFactor = c->RelaxationFactor();
			c->SetLimit(lower, upper, softness, biasFactor, relaxationFactor);
		}
	},
	{	/* softness */
		&DKHingeConstraint::Softness,
		[](DKHingeConstraint* c, float softness)
		{
			float lower = c->LowerLimit();
			float upper = c->UpperLimit();
			float biasFactor = c->BiasFactor();
			float relaxationFactor = c->RelaxationFactor();
			c->SetLimit(lower, upper, softness, biasFactor, relaxationFactor);
		}
	},
	{	/* biasFactor */
		&DKHingeConstraint::BiasFactor,
		[](DKHingeConstraint* c, float biasFactor)
		{
			float lower = c->LowerLimit();
			float upper = c->UpperLimit();
			float softness = c->Softness();
			float relaxationFactor = c->RelaxationFactor();
			c->SetLimit(lower, upper, softness, biasFactor, relaxationFactor);
		}
	},
	{	/* relaxationFactor */
		&DKHingeConstraint::RelaxationFactor,
		[](DKHingeConstraint* c, float relaxationFactor)
		{
			float lower = c->LowerLimit();
			float upper = c->UpperLimit();
			float softness = c->Softness();
			float biasFactor = c->BiasFactor();
			c->SetLimit(lower, upper, softness, biasFactor, relaxationFactor);
		}
	},
	{	/* maxMotorImpulse */
		&DKHingeConstraint::MaxMotorImpulse,
		[](DKHingeConstraint* c, float impulse)
		{
			bool e = c->IsAngularMotorEnabled();
			float velocity = c->MotorTargetVelocity();
			c->SetAngularMotor(e, velocity, impulse);
		}
	},
	{	/* motorTargetVelocity */
		&DKHingeConstraint::MotorTargetVelocity,
		[](DKHingeConstraint* c, float velocity)
		{
			bool e = c->IsAngularMotorEnabled();
			float impulse = c->MaxMotorImpulse();
			c->SetAngularMotor(e, velocity, impulse);
		}
	},
	{	/* hingeAngle */
		&DKHingeConstraint::HingeAngle,
		NULL
	},
};

static PyObject* DCHingeConstraintGetFloatAttr(DCHingeConstraint* self, void* closure)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);

	DKASSERT_DEBUG(offset < (sizeof(floatAttrs) / sizeof(floatAttrs[0])));
	DKASSERT_DEBUG(floatAttrs[offset].getter);
	float v = (self->constraint->*(floatAttrs[offset].getter))();
	return PyFloat_FromDouble(v);
}

static int DCHingeConstraintSetFloatAttr(DCHingeConstraint* self, PyObject* value, void* closure)
{
	DCOBJECT_VALIDATE(self->constraint, -1);
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

	floatAttrs[offset].setter(self->constraint, val);
	return 0;
}

static PyObject* DCHingeConstraintAngularOnly(DCHingeConstraint* self, void*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return PyBool_FromLong(self->constraint->IsAngularOnly());
}

static int DCHingeConstraintSetAngularOnly(DCHingeConstraint* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->constraint, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int angularOnly = PyObject_IsTrue(value);
	if (angularOnly < 0)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}
	self->constraint->SetAngularOnly(angularOnly != 0);
	return 0;
}

static PyObject* DCHingeConstraintAngularMotorEnabled(DCHingeConstraint* self, void*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return PyBool_FromLong(self->constraint->IsAngularMotorEnabled());
}

static int DCHingeConstraintSetAngularMotorEnabled(DCHingeConstraint* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->constraint, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int enable = PyObject_IsTrue(value);
	if (enable < 0)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}
	self->constraint->EnableAngularMotor(enable != 0);
	return 0;
}

static PyGetSetDef getsets[] = {
	/* float attrs */
	{ "lowerLimit", (getter)&DCHingeConstraintGetFloatAttr, (setter)&DCHingeConstraintSetFloatAttr, 0, (void*)0 },
	{ "upperLimit", (getter)&DCHingeConstraintGetFloatAttr, (setter)&DCHingeConstraintSetFloatAttr, 0, (void*)1 },
	{ "softness", (getter)&DCHingeConstraintGetFloatAttr, (setter)&DCHingeConstraintSetFloatAttr, 0, (void*)2 },
	{ "biasFactor", (getter)&DCHingeConstraintGetFloatAttr, (setter)&DCHingeConstraintSetFloatAttr, 0, (void*)3 },
	{ "relaxationFactor", (getter)&DCHingeConstraintGetFloatAttr, (setter)&DCHingeConstraintSetFloatAttr, 0, (void*)4 },
	{ "maxMotorImpulse", (getter)&DCHingeConstraintGetFloatAttr, (setter)&DCHingeConstraintSetFloatAttr, 0, (void*)5 },
	{ "motorTargetVelocity", (getter)&DCHingeConstraintGetFloatAttr, (setter)&DCHingeConstraintSetFloatAttr, 0, (void*)6 },

	/* other attrs */
	{ "angularOnly", (getter)&DCHingeConstraintAngularOnly, (setter)&DCHingeConstraintSetAngularOnly, 0, 0 },
	{ "angularMotorEnabled", (getter)&DCHingeConstraintAngularMotorEnabled, (setter)&DCHingeConstraintSetAngularMotorEnabled, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".HingeConstraint",		/* tp_name */
	sizeof(DCHingeConstraint),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCHingeConstraintDealloc,		/* tp_dealloc */
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
	0,											/* tp_as_constraint */
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
	DCConstraintTypeObject(),					/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCHingeConstraintInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCHingeConstraintNew,						/* tp_new */
};

PyTypeObject* DCHingeConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCHingeConstraintFromObject(DKHingeConstraint* constraint)
{
	if (constraint)
	{
		DCHingeConstraint* self = (DCHingeConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCHingeConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->constraint = constraint;
				DCObjectSetAddress(self->constraint, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKHingeConstraint* DCHingeConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCHingeConstraint*)obj)->constraint;
	}
	return NULL;
}
