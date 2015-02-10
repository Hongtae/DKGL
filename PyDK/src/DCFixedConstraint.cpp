#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConstraint.h"
#include "DCObject.h"

struct DCFixedConstraint
{
	DCConstraint base;
	DKFixedConstraint* constraint;
};

static PyObject* DCFixedConstraintNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCFixedConstraint* self = (DCFixedConstraint*)DCConstraintTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->constraint = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCFixedConstraintInit(DCFixedConstraint *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKFixedConstraint> con = NULL;
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

		con = DKOBJECT_NEW DKFixedConstraint(rbA, rbB, ta, tb);
		self->constraint = con;
		DCObjectSetAddress(self->constraint, (PyObject*)self);
	}
	self->base.constraint = self->constraint;
	return DCConstraintTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCFixedConstraintDealloc(DCFixedConstraint* self)
{
	self->constraint = NULL;
	DCConstraintTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCFixedConstraintFrameA(DCFixedConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameA();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCFixedConstraintFrameB(DCFixedConstraint* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->constraint, NULL);
	DKNSTransform t = self->constraint->FrameB();
	return DCNSTransformFromObject(&t);
}

static PyObject* DCFixedConstraintSetFrameA(DCFixedConstraint* self, PyObject* args)
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

static PyObject* DCFixedConstraintSetFrameB(DCFixedConstraint* self, PyObject* args)
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

static PyObject* DCFixedConstraintSetFrames(DCFixedConstraint* self, PyObject* args)
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

static PyMethodDef methods[] = {
	{ "frameA", (PyCFunction)&DCFixedConstraintFrameA, METH_NOARGS },
	{ "frameB", (PyCFunction)&DCFixedConstraintFrameB, METH_NOARGS },
	{ "setFrameA", (PyCFunction)&DCFixedConstraintSetFrameA, METH_VARARGS },
	{ "setFrameB", (PyCFunction)&DCFixedConstraintSetFrameB, METH_VARARGS },
	{ "setFrames", (PyCFunction)&DCFixedConstraintSetFrames, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".FixedConstraint",		/* tp_name */
	sizeof(DCFixedConstraint),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCFixedConstraintDealloc,		/* tp_dealloc */
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
	(initproc)&DCFixedConstraintInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCFixedConstraintNew,						/* tp_new */
};

PyTypeObject* DCFixedConstraintTypeObject(void)
{
	return &objectType;
}

PyObject* DCFixedConstraintFromObject(DKFixedConstraint* constraint)
{
	if (constraint)
	{
		DCFixedConstraint* self = (DCFixedConstraint*)DCObjectFromAddress(constraint);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCFixedConstraint*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKFixedConstraint* DCFixedConstraintToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCFixedConstraint*)obj)->constraint;
	}
	return NULL;
}
