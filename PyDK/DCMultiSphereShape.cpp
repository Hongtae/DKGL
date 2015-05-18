#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCConvexShape.h"
#include "DCObject.h"

struct DCMultiSphereShape
{
	DCConvexShape base;
	DKMultiSphereShape* shape;
};

static PyObject* DCMultiSphereShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCMultiSphereShape* self = (DCMultiSphereShape*)DCConvexShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCMultiSphereShapeInit(DCMultiSphereShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKMultiSphereShape> shape = NULL;
	if (self->shape == NULL)
	{
		DKASSERT_DEBUG(PyTuple_Check(args));
		size_t numItems = PyTuple_GET_SIZE(args);

		DKArray<DKVector3> centers;
		DKArray<float> radii;
		centers.Reserve(numItems);
		radii.Reserve(numItems);

		for (size_t i = 0; i < numItems; ++i)
		{
			PyObject* obj = PyTuple_GET_ITEM(args, i);
			PyObject* sphere = PySequence_Tuple(obj);
			if (sphere == NULL)
			{
				PyErr_SetString(PyExc_TypeError, "argument must be sequence object.");
				return -1;
			}

			DKVector3 v;
			float r;
			if (!PyArg_ParseTuple(sphere, "O&f", &DCVector3Converter, &v, &r))
			{
				PyErr_Clear();
				PyErr_SetString(PyExc_TypeError, "each spere should have center:Vector3, radius:float");
			}

			Py_DECREF(sphere);

			if (PyErr_Occurred())
				return -1;

			centers.Add(v);
			radii.Add(r);
		}
		DKASSERT_DEBUG(centers.Count() == radii.Count());
		if (centers.Count() > 0)
		{
			shape = DKOBJECT_NEW DKMultiSphereShape(centers, radii, centers.Count());
			self->shape = shape;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "argument must have one or more tuples to describe sphere.");
			return -1;
		}
	}

	self->base.shape = self->shape;
	return DCConvexShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCMultiSphereShapeDealloc(DCMultiSphereShape* self)
{
	self->shape = NULL;
	DCConvexShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCMultiSphereShapeNumberOfSpheres(DCMultiSphereShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	return PyLong_FromSize_t(self->shape->NumberOfSpheres());
}

static PyObject* DCMultiSphereShapeGetSphere(DCMultiSphereShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	unsigned int index;
	if (!PyArg_ParseTuple(args, "I", &index))
		return NULL;

	size_t numSpheres = self->shape->NumberOfSpheres();
	if (index >= numSpheres)
	{
		PyErr_SetString(PyExc_IndexError, "argument is out of range.");
		return NULL;
	}
	DKVector3 center = self->shape->CenterOfSphereAtIndex(index);
	float radius = self->shape->RadiusOfSphereAtIndex(index);
	return Py_BuildValue("Nf", DCVector3FromObject(&center), radius);
}

static PyMethodDef methods[] = {
	{ "numberOfSpheres", (PyCFunction)&DCMultiSphereShapeNumberOfSpheres, METH_NOARGS },
	{ "getSphere", (PyCFunction)&DCMultiSphereShapeGetSphere, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".MultiSphereShape",		/* tp_name */
	sizeof(DCMultiSphereShape),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCMultiSphereShapeDealloc,		/* tp_dealloc */
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
	0,											/* tp_as_shape */
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
	DCConvexShapeTypeObject(),					/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCMultiSphereShapeInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCMultiSphereShapeNew,						/* tp_new */
};

PyTypeObject* DCMultiSphereShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCMultiSphereShapeFromObject(DKMultiSphereShape* shape)
{
	if (shape)
	{
		DCMultiSphereShape* self = (DCMultiSphereShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCMultiSphereShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->shape = shape;
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKMultiSphereShape* DCMultiSphereShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCMultiSphereShape*)obj)->shape;
	}
	return NULL;
}
