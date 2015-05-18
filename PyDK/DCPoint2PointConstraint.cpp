#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConstraint.h"
#include "DCObject.h"

struct DCPoint2PointConstraint
{
	DCConstraint base;
	DKPoint2PointConstraint* constraint;
};

static PyObject* DCPoint2PointConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCPoint2PointConstraint* self = (DCPoint2PointConstraint*)DCConstraintTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCPoint2PointConstraintInit(DCPoint2PointConstraint *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKPoint2PointConstraint> con = NULL;
	if (self->constraint == NULL)
	{
		PyObject* objA;
		PyObject* objB;
		DKVector3 pivotA, pivotB;
		char* kwlist[] = { "bodyA", "bodyB", "pivotA", "pivotB", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO&O&", kwlist,
			&objA, &objB, &DCVector3Converter, &pivotA, &DCVector3Converter, &pivotB))
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

		con = DKOBJECT_NEW DKPoint2PointConstraint(rbA, rbB, pivotA, pivotB);
		self->constraint = con;
		DCObjectSetAddress(self->constraint, (PyObject*)self);
	}
	self->base.constraint = self->constraint;
	return DCConstraintTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCPoint2PointConstraintDealloc(DCPoint2PointConstraint* self)
{
	self->constraint = NULL;
	DCConstraintTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCPoint2PointConstraintPivotA(DCPoint2PointConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v = self->constraint->PivotInA();
	return DCVector3FromObject(&v);
}

static PyObject* DCPoint2PointConstraintPivotB(DCPoint2PointConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v = self->constraint->PivotInB();
	return DCVector3FromObject(&v);
}

static PyObject* DCPoint2PointConstraintSetPivotA(DCPoint2PointConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &v))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	self->constraint->SetPivotInA(v);
	Py_RETURN_NONE;
}

static PyObject* DCPoint2PointConstraintSetPivotB(DCPoint2PointConstraint* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKVector3 v;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &v))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
		return NULL;
	}
	self->constraint->SetPivotInB(v);
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "pivotA", (PyCFunction)&DCPoint2PointConstraintPivotA, METH_NOARGS },
	{ "pivotB", (PyCFunction)&DCPoint2PointConstraintPivotB, METH_NOARGS },
	{ "setPivotA", (PyCFunction)&DCPoint2PointConstraintSetPivotA, METH_VARARGS },
	{ "setPivotB", (PyCFunction)&DCPoint2PointConstraintSetPivotB, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Point2PointConstraint",	/* tp_name */
	sizeof(DCPoint2PointConstraint),			/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCPoint2PointConstraintDealloc,/* tp_dealloc */
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
	0,											/* tp_getset */
	DCConstraintTypeObject(),					/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCPoint2PointConstraintInit,		/* tp_init */
	0,											/* tp_alloc */
	&DCPoint2PointConstraintNew,				/* tp_new */
};

PyTypeObject* DCPoint2PointConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCPoint2PointConstraintFromObject(DKPoint2PointConstraint* constraint)
{
	if (constraint)
	{
		DCPoint2PointConstraint* self = (DCPoint2PointConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCPoint2PointConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKPoint2PointConstraint* DCPoint2PointConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCPoint2PointConstraint*)obj)->constraint;
	}
	return NULL;
}
