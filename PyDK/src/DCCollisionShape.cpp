#include <Python.h>
#include <structmember.h>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include "DCCollisionShape.h"
#include "DCObject.h"


static PyObject* DCCollisionShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCCollisionShape* self = (DCCollisionShape*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->shape) DKObject<DKCollisionShape>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCCollisionShapeInit(DCCollisionShape *self, PyObject *args, PyObject *kwds)
{
	if (self->shape == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid object");
		return -1;
	}

	DCObjectSetAddress(self->shape, (PyObject*)self);
	return 0;
}

static void DCCollisionShapeDealloc(DCCollisionShape* self)
{
	if (self->shape)
	{
		DCObjectSetAddress(self->shape, NULL);
		self->shape = NULL;
	}
	self->shape.~DKObject<DKCollisionShape>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCCollisionShapeAABB(DCCollisionShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);

	DKNSTransform trans;
	if (!PyArg_ParseTuple(args, "O&", &DCNSTransformConverter, &trans))
		return NULL;

	DKAABox aabb = self->shape->AABB(trans);
	return Py_BuildValue("NN",
		DCVector3FromObject(&aabb.positionMin),
		DCVector3FromObject(&aabb.positionMax));
}

static PyObject* DCCollisionShapeBoundingSphere(DCCollisionShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);

	DKSphere bs = self->shape->BoundingSphere();
	return Py_BuildValue("Nf", DCVector3FromObject(&bs.center), bs.radius);
}

static PyObject* DCCollisionShapeCalculateLocalInertia(DCCollisionShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	float mass;
	if (!PyArg_ParseTuple(args, "f", &mass))
		return NULL;

	DKVector3 inertia = self->shape->CalculateLocalInertia(mass);
	return DCVector3FromObject(&inertia);
}

static PyMethodDef methods[] = {
	{ "aabb", (PyCFunction)&DCCollisionShapeAABB, METH_VARARGS },
	{ "boundingSphere", (PyCFunction)&DCCollisionShapeBoundingSphere, METH_NOARGS },
	{ "calculateLocalInertia", (PyCFunction)&DCCollisionShapeCalculateLocalInertia, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCCollisionShapeLocalScale(DCCollisionShape* self, void*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);

	DKVector3 scale = self->shape->LocalScale();
	return Py_BuildValue("fff", scale.x, scale.y, scale.z);
}

static int DCCollisionShapeSetLocalScale(DCCollisionShape* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->shape, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3 scale;
	if (!DCVector3Converter(value, &scale))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be three floats or Vector3.");
		return -1;
	}
	self->shape->SetLocalScale(scale);
	return 0;
}

static PyObject* DCCollisionShapeMargin(DCCollisionShape* self, void*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);

	return PyFloat_FromDouble(self->shape->Margin());
}

static int DCCollisionShapeSetMargin(DCCollisionShape* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->shape, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double margin = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be float number.");
		return -1;
	}
	self->shape->SetMargin(margin);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "localScale", (getter)&DCCollisionShapeLocalScale, (setter)&DCCollisionShapeSetLocalScale, 0, 0 },
	{ "margin", (getter)&DCCollisionShapeMargin, (setter)&DCCollisionShapeSetMargin, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".CollisionShape",			/* tp_name */
	sizeof(DCCollisionShape),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCCollisionShapeDealloc,		/* tp_dealloc */
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
	0,											/* tp_as_buffer */
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
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCCollisionShapeInit,			/* tp_init */
	0,											/* tp_alloc */
	&DCCollisionShapeNew,						/* tp_new */
};

PyTypeObject* DCCollisionShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCCollisionShapeFromObject(DKCollisionShape* shape)
{
	if (shape)
	{
		DCCollisionShape* self = (DCCollisionShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(CompoundShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(ConcaveShape, shape);
			DCOBJECT_DYANMIC_CAST_CONVERT(ConvexShape, shape);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCCollisionShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKCollisionShape* DCCollisionShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCCollisionShape*)obj)->shape;
	}
	return NULL;
}
