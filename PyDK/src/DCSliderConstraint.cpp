#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConstraint.h"
#include "DCObject.h"

struct DCSliderConstraint
{
	DCConstraint base;
	DKSliderConstraint* constraint;
};

static PyObject* DCSliderConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCSliderConstraint* self = (DCSliderConstraint*)DCConstraintTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCSliderConstraintInit(DCSliderConstraint *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKSliderConstraint> con = NULL;
	if (self->constraint == NULL)
	{
		con = DKOBJECT_NEW DKSliderConstraint();
		self->constraint = con;
		DCObjectSetAddress(self->constraint, (PyObject*)self);
	}
	self->base.constraint = self->constraint;
	return DCConstraintTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCSliderConstraintDealloc(DCSliderConstraint* self)
{
	self->constraint = NULL;
	DCConstraintTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCSliderConstraintFrameA(DCSliderConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameA();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCSliderConstraintFrameB(DCSliderConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameB();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCSliderConstraintSetFrameA(DCSliderConstraint* self, PyObject* args)
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

static PyObject* DCSliderConstraintSetFrameB(DCSliderConstraint* self, PyObject* args)
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

static PyObject* DCSliderConstraintSetFrames(DCSliderConstraint* self, PyObject* args)
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

static PyObject* DCSliderConstraintLinearLimit(DCSliderConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	float lower = self->constraint->LinearLowerLimit();
	float upper = self->constraint->LinearUpperLimit();
	return Py_BuildValue("ff", lower, upper);
}

static PyObject* DCSliderConstraintSetLinearLimit(DCSliderConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	float lower, upper;
	if (!PyArg_ParseTuple(args, "ff", &lower, &upper))
		return NULL;
	self->constraint->SetLinearLimit(lower, upper);
	Py_RETURN_NONE;
}

static PyObject* DCSliderConstraintAngularLimit(DCSliderConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	float lower = self->constraint->AngularLowerLimit();
	float upper = self->constraint->AngularUpperLimit();
	return Py_BuildValue("ff", lower, upper);
}

static PyObject* DCSliderConstraintSetAngularLimit(DCSliderConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	float lower, upper;
	if (!PyArg_ParseTuple(args, "ff", &lower, &upper))
		return NULL;
	self->constraint->SetAngularLimit(lower, upper);
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "frameA", (PyCFunction)&DCSliderConstraintFrameA, METH_NOARGS },
	{ "frameB", (PyCFunction)&DCSliderConstraintFrameB, METH_NOARGS },
	{ "setFrameA", (PyCFunction)&DCSliderConstraintSetFrameA, METH_VARARGS },
	{ "setFrameB", (PyCFunction)&DCSliderConstraintSetFrameB, METH_VARARGS },
	{ "setFrames", (PyCFunction)&DCSliderConstraintSetFrames, METH_VARARGS },
	{ "linearLimit", (PyCFunction)&DCSliderConstraintLinearLimit, METH_NOARGS },
	{ "setLinearLimit", (PyCFunction)&DCSliderConstraintSetLinearLimit, METH_VARARGS },
	{ "angularLimit", (PyCFunction)&DCSliderConstraintAngularLimit, METH_NOARGS },
	{ "setAngularLimit", (PyCFunction)&DCSliderConstraintSetAngularLimit, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

struct FloatAttr
{
	float (DKSliderConstraint::*getter)()const;
	void (DKSliderConstraint::*setter)(float);
};
static FloatAttr floatAttrs[] = {
	{ &DKSliderConstraint::LinearLowerLimit, &DKSliderConstraint::SetLinearLowerLimit },
	{ &DKSliderConstraint::LinearUpperLimit, &DKSliderConstraint::SetLinearUpperLimit },
	{ &DKSliderConstraint::AngularLowerLimit, &DKSliderConstraint::SetAngularLowerLimit },
	{ &DKSliderConstraint::AngularUpperLimit, &DKSliderConstraint::SetAngularUpperLimit },
};

static PyObject* DCSliderConstraintGetFloatAttr(DCSliderConstraint* self, void* closure)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);

	DKASSERT_DEBUG(offset < (sizeof(floatAttrs) / sizeof(floatAttrs[0])));
	DKASSERT_DEBUG(floatAttrs[offset].getter);

	return PyFloat_FromDouble((self->constraint->*(floatAttrs[offset].getter))());
}

static int DCSliderConstraintSetFloatAttr(DCSliderConstraint* self, PyObject* value, void* closure)
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
	{ "linearLowerLimit", (getter)&DCSliderConstraintGetFloatAttr, (setter)&DCSliderConstraintSetFloatAttr, 0, (void*)0 },
	{ "linearUpperLimit", (getter)&DCSliderConstraintGetFloatAttr, (setter)&DCSliderConstraintSetFloatAttr, 0, (void*)1 },
	{ "angularLowerLimit", (getter)&DCSliderConstraintGetFloatAttr, (setter)&DCSliderConstraintSetFloatAttr, 0, (void*)2 },
	{ "angularUpperLimit", (getter)&DCSliderConstraintGetFloatAttr, (setter)&DCSliderConstraintSetFloatAttr, 0, (void*)3 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".SliderConstraint",		/* tp_name */
	sizeof(DCSliderConstraint),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCSliderConstraintDealloc,		/* tp_dealloc */
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
	(initproc)&DCSliderConstraintInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCSliderConstraintNew,						/* tp_new */
};

PyTypeObject* DCSliderConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCSliderConstraintFromObject(DKSliderConstraint* constraint)
{
	if (constraint)
	{
		DCSliderConstraint* self = (DCSliderConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCSliderConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKSliderConstraint* DCSliderConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCSliderConstraint*)obj)->constraint;
	}
	return NULL;
}
