#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConstraint.h"
#include "DCObject.h"

void DCConstraint::UpdateTargets(void)
{
	PyObject* rbA = DCRigidBodyFromObject(this->constraint->BodyA());
	PyObject* rbB = DCRigidBodyFromObject(this->constraint->BodyB());

	PyObject* tmp1 = this->bodyA;
	PyObject* tmp2 = this->bodyB;

	this->bodyA = rbA;
	this->bodyB = rbB;

	Py_XDECREF(tmp1);
	Py_XDECREF(tmp2);
}

static PyObject* DCConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCConstraint* self = (DCConstraint*)DCModelTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		self->bodyA = NULL;
		self->bodyB = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCConstraintInit(DCConstraint *self, PyObject *args, PyObject *kwds)
{
	if (self->constraint == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "This class cannot be instantiated. (abstract class)");
		return -1;
	}

	self->UpdateTargets();

	self->base.model = self->constraint;
	return DCModelTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static int DCConstraintClear(DCConstraint* self)
{
	Py_CLEAR(self->bodyA);
	Py_CLEAR(self->bodyB);
	return DCModelTypeObject()->tp_clear((PyObject*)self);
}

static int DCConstraintTraverse(DCConstraint* self, visitproc visit, void* arg)
{
	Py_VISIT(self->bodyA);
	Py_VISIT(self->bodyB);
	return DCModelTypeObject()->tp_traverse((PyObject*)self, visit, arg);
}

static void DCConstraintDealloc(DCConstraint* self)
{
	DCConstraintClear(self);
	self->constraint = NULL;
	DCModelTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCConstraintSetParam(DCConstraint* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int param;
	int axis;
	float value;
	char* kwlist[] = { "param", "axis", "value", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "iif", kwlist, &param, &axis, &value))
		return NULL;

	if (param < 0 || param > 3)	// DKConstraint::ParamType
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range.");
		return NULL;
	}
	if (axis < 0 || axis > 6)	// DKConstraint::ParamAxis
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range.");
		return NULL;
	}
	if (self->constraint->IsValidParam((DKConstraint::ParamType)param, (DKConstraint::ParamAxis)axis))
	{
		self->constraint->SetParam((DKConstraint::ParamType)param, (DKConstraint::ParamAxis)axis, value);
		Py_RETURN_NONE;
	}
	PyErr_SetString(PyExc_ValueError, "Not supported parameter");
	return NULL;
}

static PyObject* DCConstraintGetParam(DCConstraint* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int param;
	int axis;
	char* kwlist[] = { "param", "axis", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &param, &axis))
		return NULL;

	if (param < 0 || param > 3)	// DKConstraint::ParamType
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range.");
		return NULL;
	}
	if (axis < 0 || axis > 6)	// DKConstraint::ParamAxis
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range.");
		return NULL;
	}
	if (self->constraint->IsValidParam((DKConstraint::ParamType)param, (DKConstraint::ParamAxis)axis))
	{
		float value = self->constraint->GetParam((DKConstraint::ParamType)param, (DKConstraint::ParamAxis)axis);
		return PyFloat_FromDouble(value);
	}
	PyErr_SetString(PyExc_ValueError, "Not supported parameter");
	return NULL;
}

static PyObject* DCConstraintIsValidParam(DCConstraint* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int param;
	int axis;
	char* kwlist[] = { "param", "axis", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &param, &axis))
		return NULL;

	if (param < 0 || param > 3)	// DKConstraint::ParamType
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range.");
		return NULL;
	}
	if (axis < 0 || axis > 6)	// DKConstraint::ParamAxis
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range.");
		return NULL;
	}
	return PyBool_FromLong(self->constraint->IsValidParam(
		(DKConstraint::ParamType)param,
		(DKConstraint::ParamAxis)axis));
}

static PyObject* DCConstraintHasParam(DCConstraint* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int param;
	int axis;
	char* kwlist[] = { "param", "axis", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &param, &axis))
		return NULL;

	if (param < 0 || param > 3)	// DKConstraint::ParamType
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range.");
		return NULL;
	}
	if (axis < 0 || axis > 6)	// DKConstraint::ParamAxis
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range.");
		return NULL;
	}
	return PyBool_FromLong(self->constraint->HasParam((DKConstraint::ParamType)param,(DKConstraint::ParamAxis)axis));
}

static PyObject* DCConstraintBodyA(DCConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKASSERT_DEBUG(self->bodyA);
	Py_INCREF(self->bodyA);
	return self->bodyA;
}

static PyObject* DCConstraintBodyB(DCConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKASSERT_DEBUG(self->bodyB);
	Py_INCREF(self->bodyB);
	return self->bodyB;
}

static PyObject* DCConstraintRetarget(DCConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	PyObject* objA;
	PyObject* objB;
	if (!PyArg_ParseTuple(args, "OO", &objA, &objB))
		return NULL;

	auto getBody = [](PyObject* obj, const char* mesg)->DKRigidBody*
	{
		DKRigidBody* body = NULL;
		if (obj != Py_None)
		{
			body = DCRigidBodyToObject(obj);
			if (body == NULL)
				PyErr_SetString(PyExc_TypeError, mesg);
		}
		return body;
	};

	DKRigidBody* bodyA = getBody(objA, "first argument must be RigidBody object.");
	if (PyErr_Occurred())
		return NULL;
	DKRigidBody* bodyB = getBody(objB, "second argument must be RigidBody object.");
	if (PyErr_Occurred())
		return NULL;

	if (self->constraint->BodyA() != bodyA || self->constraint->BodyB() != bodyB)
	{
		bool b = self->constraint->Retarget(bodyA, bodyB);
		self->UpdateTargets();
		if (!b)
		{
			PyErr_SetString(PyExc_ValueError, "object cannot be retargeted.");
			return NULL;
		}
	}
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "setParam", (PyCFunction)&DCConstraintSetParam, METH_VARARGS | METH_KEYWORDS },
	{ "getParam", (PyCFunction)&DCConstraintGetParam, METH_VARARGS | METH_KEYWORDS },
	{ "isValidParam", (PyCFunction)&DCConstraintIsValidParam, METH_VARARGS | METH_KEYWORDS },
	{ "hasParam", (PyCFunction)&DCConstraintHasParam, METH_VARARGS | METH_KEYWORDS },
	{ "bodyA", (PyCFunction)&DCConstraintBodyA, METH_NOARGS },
	{ "bodyB", (PyCFunction)&DCConstraintBodyB, METH_NOARGS },
	{ "retarget", (PyCFunction)&DCConstraintRetarget, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCConstraintDisableCollisionsBLB(DCConstraint* self, void*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return PyBool_FromLong(self->constraint->disableCollisionsBetweenLinkedBodies);
}

static int DCConstraintSetDisableCollisionBLB(DCConstraint* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->constraint, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	int d = PyObject_IsTrue(value);
	if (d < 0)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be boolean.");
		return -1;
	}
	self->constraint->disableCollisionsBetweenLinkedBodies = d != 0;
	return 0;
}

static PyObject* DCConstraintBreakingImpulseThreshold(DCConstraint* self, void*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return PyFloat_FromDouble(self->constraint->BreakingImpulseThreshold());
}

static int DCConstraintSetBreakingImpulseThreshold(DCConstraint* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->constraint, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	double d = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be float.");
		return -1;
	}
	self->constraint->SetBreakingImpulseThreshold(d);
	return 0;
}

static PyObject* DCConstraintEnabled(DCConstraint* self, void*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return PyBool_FromLong(self->constraint->IsEnabled());
}

static int DCConstraintSetEnabled(DCConstraint* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->constraint, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	int d = PyObject_IsTrue(value);
	if (d < 0)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be boolean.");
		return -1;
	}
	self->constraint->SetEnabled(d != 0);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "disableCollisionsBetweenLinkedBodies", (getter)&DCConstraintDisableCollisionsBLB, (setter)&DCConstraintSetDisableCollisionBLB, 0, 0 },
	{ "breakingImpulseThreshold", (getter)&DCConstraintBreakingImpulseThreshold, (setter)&DCConstraintSetBreakingImpulseThreshold, 0, 0 },
	{ "enabled", (getter)&DCConstraintEnabled, (setter)&DCConstraintSetEnabled, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Constraint",				/* tp_name */
	sizeof(DCConstraint),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCConstraintDealloc,			/* tp_dealloc */
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
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,	/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCConstraintTraverse,		/* tp_traverse */
	(inquiry)&DCConstraintClear,				/* tp_clear */
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
	(initproc)&DCConstraintInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCConstraintNew,							/* tp_new */
};

PyTypeObject* DCConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCConstraintFromObject(DKConstraint* constraint)
{
	if (constraint)
	{
		DCConstraint* self = (DCConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(ConeTwistConstraint, constraint);
			DCOBJECT_DYANMIC_CAST_CONVERT(FixedConstraint, constraint);
			DCOBJECT_DYANMIC_CAST_CONVERT(GearConstraint, constraint);
			DCOBJECT_DYANMIC_CAST_CONVERT(Generic6DofConstraint, constraint);
			DCOBJECT_DYANMIC_CAST_CONVERT(HingeConstraint, constraint);
			DCOBJECT_DYANMIC_CAST_CONVERT(Point2PointConstraint, constraint);
			DCOBJECT_DYANMIC_CAST_CONVERT(SliderConstraint, constraint);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKConstraint* DCConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCConstraint*)obj)->constraint;
	}
	return NULL;
}
