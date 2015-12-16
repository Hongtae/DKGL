#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCGeneric6DofConstraint.h"
#include "DCObject.h"

static PyObject* DCGeneric6DofConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCGeneric6DofConstraint* self = (DCGeneric6DofConstraint*)DCConstraintTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCGeneric6DofConstraintInit(DCGeneric6DofConstraint *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKGeneric6DofConstraint> con = NULL;
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

		con = DKOBJECT_NEW DKGeneric6DofConstraint(rbA, rbB, ta, tb);
		self->constraint = con;
		DCObjectSetAddress(self->constraint, (PyObject*)self);
	}
	self->base.constraint = self->constraint;
	return DCConstraintTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCGeneric6DofConstraintDealloc(DCGeneric6DofConstraint* self)
{
	self->constraint = NULL;
	DCConstraintTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCGeneric6DofConstraintFrameA(DCGeneric6DofConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameA();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCGeneric6DofConstraintFrameB(DCGeneric6DofConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameB();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCGeneric6DofConstraintSetFrameA(DCGeneric6DofConstraint* self, PyObject* args)
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

static PyObject* DCGeneric6DofConstraintSetFrameB(DCGeneric6DofConstraint* self, PyObject* args)
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

static PyObject* DCGeneric6DofConstraintSetFrames(DCGeneric6DofConstraint* self, PyObject* args)
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

static PyObject* DCGeneric6DofConstraintSetLimit(DCGeneric6DofConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int axis;
	float lower, upper;
	if (!PyArg_ParseTuple(args, "iff", &axis, &lower, &upper))
		return NULL;
	if (axis < 1 || axis > 6)	// from DKConstraint::ParamAxis::LinearX(1) to AngularZ(6).
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "first argument is out of range.");
		return NULL;
	}
	self->constraint->SetLimit((DKConstraint::ParamAxis)axis, lower, upper);
	Py_RETURN_NONE;
}

struct AxisAttr
{
	PyObject* (*getter)(DKGeneric6DofConstraint*, DKConstraint::ParamAxis);
};
AxisAttr axisAttr[] = {
	[](DKGeneric6DofConstraint* c, DKConstraint::ParamAxis axis) /* LowerLimit */
	{
		return PyFloat_FromDouble(c->LowerLimit(axis));
	},
	[](DKGeneric6DofConstraint* c, DKConstraint::ParamAxis axis) /* UpperLimit */
	{
		return PyFloat_FromDouble(c->UpperLimit(axis));
	},
	[](DKGeneric6DofConstraint* c, DKConstraint::ParamAxis axis) /* Axis */
	{
		DKVector3 v = c->Axis(axis);
		return DCVector3FromObject(&v);
	},
	[](DKGeneric6DofConstraint* c, DKConstraint::ParamAxis axis) /* Angle */
	{
		return PyFloat_FromDouble(c->Angle(axis));
	},
	[](DKGeneric6DofConstraint* c, DKConstraint::ParamAxis axis) /* RelativePivotPosition */
	{
		return PyFloat_FromDouble(c->RelativePivotPosition(axis));
	},
	[](DKGeneric6DofConstraint* c, DKConstraint::ParamAxis axis) /* State */
	{
		return PyLong_FromLong((long)c->State(axis));
	},
};

template <int index>
static PyObject* DCGeneric6DofConstraintGetAxisAttr(DCGeneric6DofConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	int axis;
	if (!PyArg_ParseTuple(args, "i", &axis))
		return NULL;
	if (axis < 1 || axis > 6)	// from DKConstraint::ParamAxis::LinearX(1) to AngularZ(6).
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "first argument is out of range.");
		return NULL;
	}
	static_assert(index >= 0 && index < (sizeof(axisAttr) / sizeof(axisAttr[0])), "index out of range");
	return axisAttr[index].getter(self->constraint, (DKConstraint::ParamAxis)axis);
}

template <DKVector3 (DKGeneric6DofConstraint::*fn)(void) const>
static PyObject* GetVector3(DCGeneric6DofConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v = (self->constraint->*fn)();
	return DCVector3FromObject(&v);
}

template <void (DKGeneric6DofConstraint::*fn)(const DKVector3&)>
static PyObject* SetVector3(DCGeneric6DofConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &v))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	(self->constraint->*fn)(v);
	Py_RETURN_NONE;
}

static PyMethodDef methods[] =
{
	{ "frameA", (PyCFunction)&DCGeneric6DofConstraintFrameA, METH_NOARGS },
	{ "frameB", (PyCFunction)&DCGeneric6DofConstraintFrameB, METH_NOARGS },
	{ "setFrameA", (PyCFunction)&DCGeneric6DofConstraintSetFrameA, METH_VARARGS },
	{ "setFrameB", (PyCFunction)&DCGeneric6DofConstraintSetFrameB, METH_VARARGS },
	{ "setFrames", (PyCFunction)&DCGeneric6DofConstraintSetFrames, METH_VARARGS },
	{ "setLimit", (PyCFunction)&DCGeneric6DofConstraintSetLimit, METH_VARARGS },

	{ "lowerLimit", (PyCFunction)&DCGeneric6DofConstraintGetAxisAttr<0>, METH_VARARGS },
	{ "upperLimit", (PyCFunction)&DCGeneric6DofConstraintGetAxisAttr<1>, METH_VARARGS },
	{ "axis", (PyCFunction)&DCGeneric6DofConstraintGetAxisAttr<2>, METH_VARARGS },
	{ "angle", (PyCFunction)&DCGeneric6DofConstraintGetAxisAttr<3>, METH_VARARGS },
	{ "relativePivotPosition", (PyCFunction)&DCGeneric6DofConstraintGetAxisAttr<4>, METH_VARARGS },
	{ "axisState", (PyCFunction)&DCGeneric6DofConstraintGetAxisAttr<5>, METH_VARARGS },

	{ "linearLowerLimit", (PyCFunction)&GetVector3<&DKGeneric6DofConstraint::LinearLowerLimit>, METH_NOARGS },
	{ "setLinearLowerLimit", (PyCFunction)&SetVector3<&DKGeneric6DofConstraint::SetLinearLowerLimit>, METH_VARARGS },
	{ "linearUpperLimit", (PyCFunction)&GetVector3<&DKGeneric6DofConstraint::LinearUpperLimit>, METH_NOARGS },
	{ "setLinearUpperLimit", (PyCFunction)&SetVector3<&DKGeneric6DofConstraint::SetLinearUpperLimit>, METH_VARARGS },
	{ "angularLowerLimit", (PyCFunction)&GetVector3<&DKGeneric6DofConstraint::AngularLowerLimit>, METH_NOARGS },
	{ "setAngularLowerLimit", (PyCFunction)&SetVector3<&DKGeneric6DofConstraint::SetAngularLowerLimit>, METH_VARARGS },
	{ "angularUpperLimit", (PyCFunction)&GetVector3<&DKGeneric6DofConstraint::AngularUpperLimit>, METH_NOARGS },
	{ "setAngularUpperLimit", (PyCFunction)&SetVector3<&DKGeneric6DofConstraint::SetAngularUpperLimit>, METH_VARARGS },

	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

template <DKConstraint::ParamAxis axis>
struct AxisLimitAttr
{
	static float GetLower(DKGeneric6DofConstraint* c)
	{
		return c->LowerLimit(axis);
	};
	static float GetUpper(DKGeneric6DofConstraint* c)
	{
		return c->UpperLimit(axis);
	};
	static void SetLower(DKGeneric6DofConstraint* c, float lower)
	{
		c->SetLimit(axis, lower, GetUpper(c));
	};
	static void SetUpper(DKGeneric6DofConstraint* c, float upper)
	{
		c->SetLimit(axis, GetLower(c), upper);
	};
};

struct FloatAttr
{
	float(*getter)(DKGeneric6DofConstraint*);
	void(*setter)(DKGeneric6DofConstraint*, float);
};
static FloatAttr floatAttrs[] =
{
	{ &AxisLimitAttr<DKConstraint::ParamAxis::LinearX>::GetLower, &AxisLimitAttr<DKConstraint::ParamAxis::LinearX>::SetLower },
	{ &AxisLimitAttr<DKConstraint::ParamAxis::LinearX>::GetUpper, &AxisLimitAttr<DKConstraint::ParamAxis::LinearX>::SetUpper },

	{ &AxisLimitAttr<DKConstraint::ParamAxis::LinearY>::GetLower, &AxisLimitAttr<DKConstraint::ParamAxis::LinearY>::SetLower },
	{ &AxisLimitAttr<DKConstraint::ParamAxis::LinearY>::GetUpper, &AxisLimitAttr<DKConstraint::ParamAxis::LinearY>::SetUpper },

	{ &AxisLimitAttr<DKConstraint::ParamAxis::LinearZ>::GetLower, &AxisLimitAttr<DKConstraint::ParamAxis::LinearZ>::SetLower },
	{ &AxisLimitAttr<DKConstraint::ParamAxis::LinearZ>::GetUpper, &AxisLimitAttr<DKConstraint::ParamAxis::LinearZ>::SetUpper },

	{ &AxisLimitAttr<DKConstraint::ParamAxis::AngularX>::GetLower, &AxisLimitAttr<DKConstraint::ParamAxis::AngularX>::SetLower },
	{ &AxisLimitAttr<DKConstraint::ParamAxis::AngularX>::GetUpper, &AxisLimitAttr<DKConstraint::ParamAxis::AngularX>::SetUpper },

	{ &AxisLimitAttr<DKConstraint::ParamAxis::AngularY>::GetLower, &AxisLimitAttr<DKConstraint::ParamAxis::AngularY>::SetLower },
	{ &AxisLimitAttr<DKConstraint::ParamAxis::AngularY>::GetUpper, &AxisLimitAttr<DKConstraint::ParamAxis::AngularY>::SetUpper },

	{ &AxisLimitAttr<DKConstraint::ParamAxis::AngularZ>::GetLower, &AxisLimitAttr<DKConstraint::ParamAxis::AngularZ>::SetLower },
	{ &AxisLimitAttr<DKConstraint::ParamAxis::AngularZ>::GetUpper, &AxisLimitAttr<DKConstraint::ParamAxis::AngularZ>::SetUpper },
};

static PyObject* DCGeneric6DofConstraintGetFloatAttr(DCGeneric6DofConstraint* self, void* closure)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);

	DKASSERT_DEBUG(offset < (sizeof(floatAttrs) / sizeof(floatAttrs[0])));
	DKASSERT_DEBUG(floatAttrs[offset].getter);

	return PyFloat_FromDouble( floatAttrs[offset].getter(self->constraint) );
}

static int DCGeneric6DofConstraintSetFloatAttr(DCGeneric6DofConstraint* self, PyObject* value, void* closure)
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

static PyGetSetDef getsets[] = {
	{ "linearXLowerLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)0 },
	{ "linearXUpperLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)1 },
	{ "linearYLowerLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)2 },
	{ "linearYUpperLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)3 },
	{ "linearZLowerLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)4 },
	{ "linearZUpperLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)5 },
	{ "angularXLowerLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)6 },
	{ "angularXUpperLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)7 },
	{ "angularYLowerLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)8 },
	{ "angularYUpperLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)9 },
	{ "angularZLowerLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)10 },
	{ "angularZUpperLimit", (getter)&DCGeneric6DofConstraintGetFloatAttr, (setter)&DCGeneric6DofConstraintSetFloatAttr, 0, (void*)11 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Generic6DofConstraint",	/* tp_name */
	sizeof(DCGeneric6DofConstraint),			/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCGeneric6DofConstraintDealloc,/* tp_dealloc */
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
	(initproc)&DCGeneric6DofConstraintInit,		/* tp_init */
	0,											/* tp_alloc */
	&DCGeneric6DofConstraintNew,				/* tp_new */
};

PyTypeObject* DCGeneric6DofConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCGeneric6DofConstraintFromObject(DKGeneric6DofConstraint* constraint)
{
	if (constraint)
	{
		DCGeneric6DofConstraint* self = (DCGeneric6DofConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(Generic6DofSpringConstraint, constraint);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCGeneric6DofConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKGeneric6DofConstraint* DCGeneric6DofConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCGeneric6DofConstraint*)obj)->constraint;
	}
	return NULL;
}
