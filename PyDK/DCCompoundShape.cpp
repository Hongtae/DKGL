#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCCollisionShape.h"
#include "DCObject.h"

struct DCCompoundShape
{
	DCCollisionShape base;
	DKCompoundShape* shape;
	PyObject* children;

	void UpdateChildren(void)
	{
		size_t num = this->shape->NumberOfShapes();
		PyObject* tuple = PyTuple_New(num);
		for (unsigned int i = 0; i < num; ++i)
		{
			DKCollisionShape* shape = this->shape->ShapeAtIndex(i);
			PyObject* obj = DCCollisionShapeFromObject(shape);
			DKASSERT_DEBUG(DCCollisionShapeToObject(obj) == shape);
			PyTuple_SET_ITEM(tuple, i, obj);
		}
		Py_XDECREF(this->children);
		this->children = tuple;
	}
};

static PyObject* DCCompoundShapeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCCompoundShape* self = (DCCompoundShape*)DCCollisionShapeTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->shape = NULL;
		self->children = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCCompoundShapeInit(DCCompoundShape *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKCompoundShape> shape = NULL;
	if (self->shape == NULL)
	{
		shape = DKOBJECT_NEW DKCompoundShape();
		self->shape = shape;
	}
	self->UpdateChildren();
	self->base.shape = self->shape;
	return DCCollisionShapeTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static int DCCompoundShapeClear(DCCompoundShape* self)
{
	Py_CLEAR(self->children);
	return 0;
}

static int DCCompoundShapeTraverse(DCCompoundShape* self, visitproc visit, void* arg)
{
	Py_VISIT(self->children);
	return 0;
}

static void DCCompoundShapeDealloc(DCCompoundShape* self)
{
	DCCompoundShapeClear(self);
	self->shape = NULL;
	DCCollisionShapeTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCCompoundShapeChildren(DCCompoundShape* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->shape, NULL);

	if (self->children == NULL)
		self->UpdateChildren();

	DKASSERT_DEBUG(self->children);
	Py_INCREF(self->children);
	return self->children;
}

static PyObject* DCCompoundShapeNumberOfChildren(DCCompoundShape* self, PyObject*)
{
	size_t num = 0;
	if (self->shape)
		num = self->shape->NumberOfShapes();
	return PyLong_FromSize_t(num);
}

static PyObject* DCCompoundShapeAddChild(DCCompoundShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	PyObject* shapeObj;
	DKNSTransform trans;

	if (!PyArg_ParseTuple(args, "OO&", &shapeObj, &DCNSTransformConverter, &trans))
		return NULL;

	DKCollisionShape* shape = DCCollisionShapeToObject(shapeObj);
	if (shape == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "first argument must be CollisionShape object.");
		return NULL;
	}

	self->shape->AddShape(shape, trans);
	self->UpdateChildren();
	Py_RETURN_NONE;
}

static PyObject* DCCompoundShapeRemoveChild(DCCompoundShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	PyObject* child;

	if (!PyArg_ParseTuple(args, "O", &child))
		return NULL;

	DKCollisionShape* shape = DCCollisionShapeToObject(child);
	if (shape)
	{
		self->shape->RemoveShape(shape);
		self->UpdateChildren();
		Py_RETURN_NONE;
	}
	else if (PyLong_Check(child))
	{
		size_t num = self->shape->NumberOfShapes();
		long index = PyLong_AsLong(child);
		if (!PyErr_Occurred())
		{
			if (index < 0 || index >= num)
			{
				PyErr_SetString(PyExc_IndexError, "argument must be index of children");
				return NULL;
			}
			self->shape->RemoveShapeAtIndex((unsigned int)index);
			self->UpdateChildren();
			Py_RETURN_NONE;
		}
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be CollisionShape or integer(Index)");
	return NULL;
}

static PyObject* DCCompoundShapeTransformOfChild(DCCompoundShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	PyObject* child;

	if (!PyArg_ParseTuple(args, "O", &child))
		return NULL;

	DKCollisionShape* shape = DCCollisionShapeToObject(child);
	if (shape)
	{
		long index = self->shape->IndexOfShape(shape);
		if (index < 0)
		{
			PyErr_SetString(PyExc_ValueError, "object is not a child.");
			return NULL;
		}
		DKNSTransform trans = self->shape->TransformAtIndex(index);
		return DCNSTransformFromObject(&trans);
	}
	else if (PyLong_Check(child))
	{
		size_t num = self->shape->NumberOfShapes();
		long index = PyLong_AsLong(child);
		if (!PyErr_Occurred())
		{
			if (index < 0 || index >= num)
			{
				PyErr_SetString(PyExc_IndexError, "argument must be index of children");
				return NULL;
			}
			DKNSTransform trans = self->shape->TransformAtIndex(index);
			return DCNSTransformFromObject(&trans);
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be CollisionShape or integer(Index)");
	return NULL;
}

static PyObject* DCCompoundShapeSetTransformOfChild(DCCompoundShape* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->shape, NULL);
	PyObject* child;
	DKNSTransform trans;

	if (!PyArg_ParseTuple(args, "OO&", &child, &DCNSTransformConverter, &trans))
		return NULL;

	DKCollisionShape* shape = DCCollisionShapeToObject(child);
	if (shape)
	{
		long index = self->shape->IndexOfShape(shape);
		if (index < 0)
		{
			PyErr_SetString(PyExc_ValueError, "object is not a child.");
			return NULL;
		}
		self->shape->SetTransformAtIndex(trans, index);
		Py_RETURN_NONE;
	}
	else if (PyLong_Check(child))
	{
		size_t num = self->shape->NumberOfShapes();
		long index = PyLong_AsLong(child);
		if (!PyErr_Occurred())
		{
			if (index < 0 || index >= num)
			{
				PyErr_SetString(PyExc_IndexError, "argument must be index of children");
				return NULL;
			}
			self->shape->SetTransformAtIndex(trans, index);
			Py_RETURN_NONE;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be CollisionShape or integer(Index)");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "children", (PyCFunction)&DCCompoundShapeChildren, METH_NOARGS },
	{ "numberOfChildren", (PyCFunction)&DCCompoundShapeNumberOfChildren, METH_NOARGS },
	{ "addChild", (PyCFunction)&DCCompoundShapeAddChild, METH_VARARGS },
	{ "removeChild", (PyCFunction)&DCCompoundShapeRemoveChild, METH_VARARGS },
	{ "transformOfChild", (PyCFunction)&DCCompoundShapeTransformOfChild, METH_VARARGS },
	{ "setTransformOfChild", (PyCFunction)&DCCompoundShapeSetTransformOfChild, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".CompoundShape",			/* tp_name */
	sizeof(DCCompoundShape),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCCompoundShapeDealloc,		/* tp_dealloc */
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
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCCompoundShapeTraverse,		/* tp_traverse */
	(inquiry)&DCCompoundShapeClear,				/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	0,											/* tp_getset */
	DCCollisionShapeTypeObject(),				/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCCompoundShapeInit,				/* tp_init */
	0,											/* tp_alloc */
	&DCCompoundShapeNew,						/* tp_new */
};

PyTypeObject* DCCompoundShapeTypeObject(void)
{
	return &objectType;
}

PyObject* DCCompoundShapeFromObject(DKCompoundShape* shape)
{
	if (shape)
	{
		DCCompoundShape* self = (DCCompoundShape*)DCObjectFromAddress(shape);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCCompoundShape*)DCObjectCreateDefaultClass(&objectType, args, kwds);
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

DKCompoundShape* DCCompoundShapeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCCompoundShape*)obj)->shape;
	}
	return NULL;
}
