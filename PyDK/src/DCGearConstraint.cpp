#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConstraint.h"
#include "DCObject.h"

struct DCGearConstraint
{
	DCConstraint base;
	DKGearConstraint* constraint;
};

static PyObject* DCGearConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCGearConstraint* self = (DCGearConstraint*)DCConstraintTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCGearConstraintInit(DCGearConstraint *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKGearConstraint> con = NULL;
	if (self->constraint == NULL)
	{
		PyObject* objA;
		PyObject* objB;
		DKVector3 axisA, axisB;
		float ratio = 1.0f;
		char* kwlist[] = { "bodyA", "bodyB", "axisInA", "axisInB", "ratio", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO&O&|f", kwlist,
			&objA, &objB, 
			&DCVector3Converter, &axisA,
			&DCVector3Converter, &axisB,
			&ratio))
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

		con = DKOBJECT_NEW DKGearConstraint(rbA, rbB, axisA, axisB, ratio);
		self->constraint = con;
		DCObjectSetAddress(self->constraint, (PyObject*)self);
	}
	self->base.constraint = self->constraint;
	return DCConstraintTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCGearConstraintDealloc(DCGearConstraint* self)
{
	self->constraint = NULL;
	DCConstraintTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCGearConstraintAxisInA(DCGearConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v = self->constraint->AxisInA();
	return DCVector3FromObject(&v);
}

static PyObject* DCGearConstraintAxisInB(DCGearConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v = self->constraint->AxisInB();
	return DCVector3FromObject(&v);
}

static PyObject* DCGearConstraintSetAxisInA(DCGearConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &v))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	self->constraint->SetAxisInA(v);
	Py_RETURN_NONE;
}

static PyObject* DCGearConstraintSetAxisInB(DCGearConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &v))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	self->constraint->SetAxisInB(v);
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "axisInA", (PyCFunction)&DCGearConstraintAxisInA, METH_NOARGS },
	{ "axisInB", (PyCFunction)&DCGearConstraintAxisInB, METH_NOARGS },
	{ "setAxisInA", (PyCFunction)&DCGearConstraintSetAxisInA, METH_VARARGS },
	{ "setAxisInB", (PyCFunction)&DCGearConstraintSetAxisInB, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCGearConstraintRatio(DCGearConstraint* self, void*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	return PyFloat_FromDouble(self->constraint->Ratio());
}

static int DCGearConstraintSetRatio(DCGearConstraint* self, PyObject* value, void*)
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
	self->constraint->SetRatio(val);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "ratio", (getter)&DCGearConstraintRatio, (setter)&DCGearConstraintSetRatio, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".GearConstraint",			/* tp_name */
	sizeof(DCGearConstraint),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCGearConstraintDealloc,		/* tp_dealloc */
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
	(initproc)&DCGearConstraintInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCGearConstraintNew,						/* tp_new */
};

PyTypeObject* DCGearConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCGearConstraintFromObject(DKGearConstraint* constraint)
{
	if (constraint)
	{
		DCGearConstraint* self = (DCGearConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCGearConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKGearConstraint* DCGearConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCGearConstraint*)obj)->constraint;
	}
	return NULL;
}
