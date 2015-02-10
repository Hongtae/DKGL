#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConstraint.h"
#include "DCObject.h"

struct DCConeTwistConstraint
{
	DCConstraint base;
	DKConeTwistConstraint* constraint;
};

static PyObject* DCConeTwistConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCConeTwistConstraint* self = (DCConeTwistConstraint*)DCConstraintTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCConeTwistConstraintInit(DCConeTwistConstraint *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKConeTwistConstraint> con = NULL;
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

		con = DKOBJECT_NEW DKConeTwistConstraint(rbA, rbB, ta, tb);
		self->constraint = con;
		DCObjectSetAddress(self->constraint, (PyObject*)self);
	}
	self->base.constraint = self->constraint;
	return DCConstraintTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCConeTwistConstraintDealloc(DCConeTwistConstraint* self)
{
	self->constraint = NULL;
	DCConstraintTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCConeTwistConstraintFrameA(DCConeTwistConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameA();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCConeTwistConstraintFrameB(DCConeTwistConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameB();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCConeTwistConstraintSetFrameA(DCConeTwistConstraint* self, PyObject* args)
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

static PyObject* DCConeTwistConstraintSetFrameB(DCConeTwistConstraint* self, PyObject* args)
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

static PyObject* DCConeTwistConstraintSetFrames(DCConeTwistConstraint* self, PyObject* args)
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

static PyObject* DCConeTwistConstraintSetLimit(DCConeTwistConstraint* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	float swingSpan1;
	float swingSpan2;
	float twistSpan;
	float softness = 1.0f;
	float biasFactor = 0.3f;
	float relaxationFactor = 1.0f;
	char* kwlist[] = { "swingSpan1", "swingSpan2", "twistSpan", "softness", "biasFactor", "relaxationFactor", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "fff|fff", kwlist,
		&swingSpan1, &swingSpan2, &twistSpan, &softness, &biasFactor, &relaxationFactor))
		return NULL;

	self->constraint->SetLimit(swingSpan1, swingSpan2, twistSpan, softness, biasFactor, relaxationFactor);
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "frameA", (PyCFunction)&DCConeTwistConstraintFrameA, METH_NOARGS },
	{ "frameB", (PyCFunction)&DCConeTwistConstraintFrameB, METH_NOARGS },
	{ "setFrameA", (PyCFunction)&DCConeTwistConstraintSetFrameA, METH_VARARGS },
	{ "setFrameB", (PyCFunction)&DCConeTwistConstraintSetFrameB, METH_VARARGS },
	{ "setFrames", (PyCFunction)&DCConeTwistConstraintSetFrames, METH_VARARGS },
	{ "setLimit", (PyCFunction)&DCConeTwistConstraintSetLimit, METH_VARARGS | METH_KEYWORDS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

struct FloatAttr
{
	float (DKConeTwistConstraint::*getter)(void) const;
	void (DKConeTwistConstraint::*setter)(float);
};
static FloatAttr floatAttrs[] = {
	{ &DKConeTwistConstraint::SwingSpan1, &DKConeTwistConstraint::SetSwingSpan1 },
	{ &DKConeTwistConstraint::SwingSpan2, &DKConeTwistConstraint::SetSwingSpan2 },
	{ &DKConeTwistConstraint::TwistSpan, &DKConeTwistConstraint::SetTwistSpan },
	{ &DKConeTwistConstraint::Softness, &DKConeTwistConstraint::SetSoftness },
	{ &DKConeTwistConstraint::BiasFactor, &DKConeTwistConstraint::SetBiasFactor },
	{ &DKConeTwistConstraint::RelaxationFactor, &DKConeTwistConstraint::SetRelaxationFactor },
	{ &DKConeTwistConstraint::Damping, &DKConeTwistConstraint::SetDamping },
	{ &DKConeTwistConstraint::TwistAngle, NULL },
};

static PyObject* DCConeTwistConstraintGetFloatAttr(DCConeTwistConstraint* self, void* closure)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);

	DKASSERT_DEBUG(offset < (sizeof(floatAttrs) / sizeof(floatAttrs[0])));
	DKASSERT_DEBUG(floatAttrs[offset].getter);

	float v = (self->constraint->*(floatAttrs[offset].getter))();
	return PyFloat_FromDouble(v);
}

static int DCConeTwistConstraintSetFloatAttr(DCConeTwistConstraint* self, PyObject* value, void* closure)
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

	(self->constraint->*(floatAttrs[offset].setter))(val);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "swingSpan1", (getter)&DCConeTwistConstraintGetFloatAttr, (setter)&DCConeTwistConstraintSetFloatAttr, 0, (void*)0 },
	{ "swingSpan2", (getter)&DCConeTwistConstraintGetFloatAttr, (setter)&DCConeTwistConstraintSetFloatAttr, 0, (void*)1 },
	{ "twistSpan", (getter)&DCConeTwistConstraintGetFloatAttr, (setter)&DCConeTwistConstraintSetFloatAttr, 0, (void*)2 },
	{ "softness", (getter)&DCConeTwistConstraintGetFloatAttr, (setter)&DCConeTwistConstraintSetFloatAttr, 0, (void*)3 },
	{ "biasFactor", (getter)&DCConeTwistConstraintGetFloatAttr, (setter)&DCConeTwistConstraintSetFloatAttr, 0, (void*)4 },
	{ "relaxationFactor", (getter)&DCConeTwistConstraintGetFloatAttr, (setter)&DCConeTwistConstraintSetFloatAttr, 0, (void*)5 },
	{ "damping", (getter)&DCConeTwistConstraintGetFloatAttr, (setter)&DCConeTwistConstraintSetFloatAttr, 0, (void*)6 },
	{ "twistAngle", (getter)&DCConeTwistConstraintGetFloatAttr, (setter)&DCConeTwistConstraintSetFloatAttr, 0, (void*)7 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ConeTwistConstraint",	/* tp_name */
	sizeof(DCConeTwistConstraint),				/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCConeTwistConstraintDealloc,	/* tp_dealloc */
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
	(initproc)&DCConeTwistConstraintInit,		/* tp_init */
	0,											/* tp_alloc */
	&DCConeTwistConstraintNew,					/* tp_new */
};

PyTypeObject* DCConeTwistConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCConeTwistConstraintFromObject(DKConeTwistConstraint* constraint)
{
	if (constraint)
	{
		DCConeTwistConstraint* self = (DCConeTwistConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCConeTwistConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKConeTwistConstraint* DCConeTwistConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCConeTwistConstraint*)obj)->constraint;
	}
	return NULL;
}
