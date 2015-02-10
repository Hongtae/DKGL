#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCGeneric6DofConstraint.h"
#include "DCObject.h"

struct DCGeneric6DofSpringConstraint
{
	DCGeneric6DofConstraint base;
	DKGeneric6DofSpringConstraint* constraint;
};

static PyObject* DCGeneric6DofSpringConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCGeneric6DofSpringConstraint* self = (DCGeneric6DofSpringConstraint*)DCGeneric6DofConstraintTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCGeneric6DofSpringConstraintInit(DCGeneric6DofSpringConstraint *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKGeneric6DofSpringConstraint> con = NULL;
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

		con = DKOBJECT_NEW DKGeneric6DofSpringConstraint(rbA, rbB, ta, tb);
		self->constraint = con;
		DCObjectSetAddress(self->constraint, (PyObject*)self);
	}
	self->base.constraint = self->constraint;
	return DCGeneric6DofConstraintTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCGeneric6DofSpringConstraintDealloc(DCGeneric6DofSpringConstraint* self)
{
	self->constraint = NULL;
	DCGeneric6DofConstraintTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCGeneric6DofSpringConstraintEnableSpring(DCGeneric6DofSpringConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int axis;
	int enable;
	if (!PyArg_ParseTuple(args, "ip", &axis, &enable))
		return NULL;
	if (axis < 1 || axis > 6)	// DKConstraint::ParamAxis::LinearX(1) 부터 AngularZ(6) 까지.
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "first argument is out of range.");
		return NULL;
	}
	self->constraint->EnableSpring((DKConstraint::ParamAxis)axis, enable != 0);
	Py_RETURN_NONE;
}

static PyObject* DCGeneric6DofSpringConstraintIsSpringEnabled(DCGeneric6DofSpringConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int axis;
	if (!PyArg_ParseTuple(args, "i", &axis))
		return NULL;
	if (axis < 1 || axis > 6)	// DKConstraint::ParamAxis::LinearX(1) 부터 AngularZ(6) 까지.
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "first argument is out of range.");
		return NULL;
	}
	return PyBool_FromLong(self->constraint->IsSpringEnabled((DKConstraint::ParamAxis)axis));
}

template <float (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis) const>
static PyObject* GetAxisFloat(DCGeneric6DofSpringConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int axis;
	if (!PyArg_ParseTuple(args, "i", &axis))
		return NULL;
	if (axis < 1 || axis > 6)	// DKConstraint::ParamAxis::LinearX(1) 부터 AngularZ(6) 까지.
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "first argument is out of range.");
		return NULL;
	}
	return PyFloat_FromDouble((self->constraint->*fn)((DKConstraint::ParamAxis)axis));
}

template <void (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis, float)>
static PyObject* SetAxisFloat(DCGeneric6DofSpringConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int axis;
	float value;
	if (!PyArg_ParseTuple(args, "if", &axis, &value))
		return NULL;
	if (axis < 1 || axis > 6)	// DKConstraint::ParamAxis::LinearX(1) 부터 AngularZ(6) 까지.
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "first argument is out of range.");
		return NULL;
	}
	(self->constraint->*fn)((DKConstraint::ParamAxis)axis, value);
	Py_RETURN_NONE;
}

static PyObject* DCGeneric6DofSpringConstraintSetEquilibriumPoint(DCGeneric6DofSpringConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	PyObject* axisObj = NULL;
	PyObject* valueObj = NULL;
	if (!PyArg_ParseTuple(args, "|OO", &axisObj, &valueObj))
		return NULL;
	if (axisObj)
	{
		long axis = PyLong_AsLong(axisObj);
		if (PyErr_Occurred())
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "optional first argument must be integer.");
		}
		if (axis < 1 || axis > 6)	// DKConstraint::ParamAxis::LinearX(1) 부터 AngularZ(6) 까지.
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "first argument is out of range.");
			return NULL;
		}
		if (valueObj)
		{
			double val = PyFloat_AsDouble(valueObj);
			if (PyErr_Occurred())
			{
				PyErr_Clear();
				PyErr_SetString(PyExc_TypeError, "optional second argument must be float.");
				return NULL;
			}
			self->constraint->SetEquilibriumPoint((DKConstraint::ParamAxis)axis, val);
		}
		else
		{
			self->constraint->SetEquilibriumPoint((DKConstraint::ParamAxis)axis);
		}
	}
	else
	{
		self->constraint->SetEquilibriumPoint();
	}
	Py_RETURN_NONE;
}

static PyMethodDef methods[] =
{
	{ "enableSpring", (PyCFunction)&DCGeneric6DofSpringConstraintEnableSpring, METH_VARARGS },
	{ "isSpringEnabled", (PyCFunction)&DCGeneric6DofSpringConstraintIsSpringEnabled, METH_VARARGS },
	{ "stiffness", (PyCFunction)&GetAxisFloat<&DKGeneric6DofSpringConstraint::Stiffness>, METH_VARARGS },
	{ "setStiffness", (PyCFunction)&SetAxisFloat<&DKGeneric6DofSpringConstraint::SetStiffness>, METH_VARARGS },
	{ "damping", (PyCFunction)&GetAxisFloat<&DKGeneric6DofSpringConstraint::Damping>, METH_VARARGS },
	{ "setDamping", (PyCFunction)&SetAxisFloat<&DKGeneric6DofSpringConstraint::SetDamping>, METH_VARARGS },
	{ "equilibriumPoint", (PyCFunction)&GetAxisFloat<&DKGeneric6DofSpringConstraint::EquilibriumPoint>, METH_VARARGS },
	{ "setEquilibriumPoint", (PyCFunction)&DCGeneric6DofSpringConstraintSetEquilibriumPoint, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

template <float (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis) const,
	DKConstraint::ParamAxis axis1, DKConstraint::ParamAxis axis2, DKConstraint::ParamAxis axis3>
	static PyObject* GetAxisFloat3Attr(DCGeneric6DofSpringConstraint* self)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return Py_BuildValue("fff",
		(self->constraint->*fn)(axis1),
		(self->constraint->*fn)(axis2),
		(self->constraint->*fn)(axis3));
}

template <bool (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis) const,
	DKConstraint::ParamAxis axis1, DKConstraint::ParamAxis axis2, DKConstraint::ParamAxis axis3>
	static PyObject* GetAxisBool3Attr(DCGeneric6DofSpringConstraint* self)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return Py_BuildValue("OOO",
		(self->constraint->*fn)(axis1) ? Py_True : Py_False,
		(self->constraint->*fn)(axis2) ? Py_True : Py_False,
		(self->constraint->*fn)(axis3) ? Py_True : Py_False);
}

template <void (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis, float),
	DKConstraint::ParamAxis axis1, DKConstraint::ParamAxis axis2, DKConstraint::ParamAxis axis3>
	static int SetAxisFloat3Attr(DCGeneric6DofSpringConstraint* self, PyObject* value)
{
	DCOBJECT_VALIDATE(self->constraint, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	float v1, v2, v3;
	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "fff", &v1, &v2, &v3))
	{
		(self->constraint->*fn)(axis1, v1);
		(self->constraint->*fn)(axis2, v2);
		(self->constraint->*fn)(axis3, v3);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be three float objects.");
	return -1;
}

template <void (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis, bool),
	DKConstraint::ParamAxis axis1, DKConstraint::ParamAxis axis2, DKConstraint::ParamAxis axis3>
	static int SetAxisBool3Attr(DCGeneric6DofSpringConstraint* self, PyObject* value)
{
	DCOBJECT_VALIDATE(self->constraint, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int v1, v2, v3;
	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ppp", &v1, &v2, &v3))
	{
		(self->constraint->*fn)(axis1, v1 != 0);
		(self->constraint->*fn)(axis2, v2 != 0);
		(self->constraint->*fn)(axis3, v3 != 0);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be three boolean objects.");
	return -1;
}

template <float (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis) const>
static PyObject* GetLinearAxisFloat3Attr(DCGeneric6DofSpringConstraint* self, void*)
{
	return GetAxisFloat3Attr<fn, DKConstraint::ParamAxis::LinearX, DKConstraint::ParamAxis::LinearY, DKConstraint::ParamAxis::LinearZ>(self);
}

template <float (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis) const>
static PyObject* GetAngularAxisFloat3Attr(DCGeneric6DofSpringConstraint* self, void*)
{
	return GetAxisFloat3Attr<fn, DKConstraint::ParamAxis::AngularX, DKConstraint::ParamAxis::AngularY, DKConstraint::ParamAxis::AngularZ>(self);
}

template <bool (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis) const>
static PyObject* GetLinearAxisBool3Attr(DCGeneric6DofSpringConstraint* self, void*)
{
	return GetAxisBool3Attr<fn, DKConstraint::ParamAxis::LinearX, DKConstraint::ParamAxis::LinearY, DKConstraint::ParamAxis::LinearZ>(self);
}

template <bool (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis) const>
static PyObject* GetAngularAxisBool3Attr(DCGeneric6DofSpringConstraint* self, void*)
{
	return GetAxisBool3Attr<fn, DKConstraint::ParamAxis::AngularX, DKConstraint::ParamAxis::AngularY, DKConstraint::ParamAxis::AngularZ>(self);
}

template <void (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis, float)>
static int SetLinearAxisFloat3Attr(DCGeneric6DofSpringConstraint* self, PyObject* value, void*)
{
	return SetAxisFloat3Attr<fn, DKConstraint::ParamAxis::LinearX, DKConstraint::ParamAxis::LinearY, DKConstraint::ParamAxis::LinearZ>(self, value);
}

template <void (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis, float)>
static int SetAngularAxisFloat3Attr(DCGeneric6DofSpringConstraint* self, PyObject* value, void*)
{
	return SetAxisFloat3Attr<fn, DKConstraint::ParamAxis::AngularX, DKConstraint::ParamAxis::AngularY, DKConstraint::ParamAxis::AngularZ>(self, value);
}

template <void (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis, bool)>
static int SetLinearAxisBool3Attr(DCGeneric6DofSpringConstraint* self, PyObject* value)
{
	return SetAxisBool3Attr<fn, DKConstraint::ParamAxis::LinearX, DKConstraint::ParamAxis::LinearY, DKConstraint::ParamAxis::LinearZ>(self, value);
}

template <void (DKGeneric6DofSpringConstraint::*fn)(DKConstraint::ParamAxis, bool)>
static int SetAngularAxisBool3Attr(DCGeneric6DofSpringConstraint* self, PyObject* value)
{
	return SetAxisBool3Attr<fn, DKConstraint::ParamAxis::AngularX, DKConstraint::ParamAxis::AngularY, DKConstraint::ParamAxis::AngularZ>(self, value);
}

static PyGetSetDef getsets[] =
{
	/* springEnabled */
	{ "linearSpringEnabled", (getter)&GetLinearAxisBool3Attr<&DKGeneric6DofSpringConstraint::IsSpringEnabled>, (setter)&SetLinearAxisBool3Attr<&DKGeneric6DofSpringConstraint::EnableSpring>, 0, 0 },
	{ "angularSpringEnabled", (getter)&GetAngularAxisBool3Attr<&DKGeneric6DofSpringConstraint::IsSpringEnabled>, (setter)&SetAngularAxisBool3Attr<&DKGeneric6DofSpringConstraint::EnableSpring>, 0, 0 },

	/* stiffness */
	{ "linearStiffness", (getter)&GetLinearAxisFloat3Attr<&DKGeneric6DofSpringConstraint::Stiffness>, (setter)&SetLinearAxisFloat3Attr<&DKGeneric6DofSpringConstraint::SetStiffness>, 0, 0 },
	{ "angularStiffness", (getter)&GetAngularAxisFloat3Attr<&DKGeneric6DofSpringConstraint::Stiffness>, (setter)&SetAngularAxisFloat3Attr<&DKGeneric6DofSpringConstraint::SetStiffness>, 0, 0 },

	/* damping */
	{ "linearDamping", (getter)&GetLinearAxisFloat3Attr<&DKGeneric6DofSpringConstraint::Damping>, (setter)&SetLinearAxisFloat3Attr<&DKGeneric6DofSpringConstraint::SetDamping>, 0, 0 },
	{ "angularDamping", (getter)&GetAngularAxisFloat3Attr<&DKGeneric6DofSpringConstraint::Damping>, (setter)&SetAngularAxisFloat3Attr<&DKGeneric6DofSpringConstraint::SetDamping>, 0, 0 },

	/* equilibriumPoint */
	{ "linearEquilibriumPoint", (getter)&GetLinearAxisFloat3Attr<&DKGeneric6DofSpringConstraint::EquilibriumPoint>, (setter)&SetLinearAxisFloat3Attr<&DKGeneric6DofSpringConstraint::SetEquilibriumPoint>, 0, 0 },
	{ "angularEquilibriumPoint", (getter)&GetAngularAxisFloat3Attr<&DKGeneric6DofSpringConstraint::EquilibriumPoint>, (setter)&SetAngularAxisFloat3Attr<&DKGeneric6DofSpringConstraint::SetEquilibriumPoint>, 0, 0 },

	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Generic6DofSpringConstraint",	/* tp_name */
	sizeof(DCGeneric6DofSpringConstraint),				/* tp_basicsize */
	0,													/* tp_itemsize */
	(destructor)&DCGeneric6DofSpringConstraintDealloc,	/* tp_dealloc */
	0,													/* tp_print */
	0,													/* tp_getattr */
	0,													/* tp_setattr */
	0,													/* tp_reserved */
	0,													/* tp_repr */
	0,													/* tp_as_number */
	0,													/* tp_as_sequence */
	0,													/* tp_as_mapping */
	0,													/* tp_hash  */
	0,													/* tp_call */
	0,													/* tp_str */
	0,													/* tp_getattro */
	0,													/* tp_setattro */
	0,													/* tp_as_constraint */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,			/* tp_flags */
	0,													/* tp_doc */
	0,													/* tp_traverse */
	0,													/* tp_clear */
	0,													/* tp_richcompare */
	0,													/* tp_weaklistoffset */
	0,													/* tp_iter */
	0,													/* tp_iternext */
	methods,											/* tp_methods */
	0,													/* tp_members */
	getsets,											/* tp_getset */
	DCConstraintTypeObject(),							/* tp_base */
	0,													/* tp_dict */
	0,													/* tp_descr_get */
	0,													/* tp_descr_set */
	0,													/* tp_dictoffset */
	(initproc)&DCGeneric6DofSpringConstraintInit,		/* tp_init */
	0,													/* tp_alloc */
	&DCGeneric6DofSpringConstraintNew,					/* tp_new */
};

PyTypeObject* DCGeneric6DofSpringConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCGeneric6DofSpringConstraintFromObject(DKGeneric6DofSpringConstraint* constraint)
{
	if (constraint)
	{
		DCGeneric6DofSpringConstraint* self = (DCGeneric6DofSpringConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCGeneric6DofSpringConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKGeneric6DofSpringConstraint* DCGeneric6DofSpringConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCGeneric6DofSpringConstraint*)obj)->constraint;
	}
	return NULL;
}
