#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCRect
{
	PyObject_HEAD
	DKRect rect;
};

static PyObject* DCRectNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCRect* self = (DCRect*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->rect) DKRect();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCRectInit(DCRect *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCRectConverter(args, &self->rect))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCRectConverter, &self->rect))
	{
		return 0;
	}
	else if (numArgs == 0)		// (0, 0, -1, -1), -1 또는 -FLT_MAX 일수 있음. (어쨌든 size 가 0 보다 작으면 invalid)
	{
		self->rect = DKRect(0, 0, -1, -1);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be one of following: (Point, Size), (x, y, w, h) or empty");
	return -1;
}

static void DCRectDealloc(DCRect* self)
{
	self->rect.~DKRect();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCRectRepr(DCRect* self)
{
	DKString str = DKString::Format("<%s object (origin:(%.3f, %.3f), size:(%.3f, %.3f))>",
		Py_TYPE(self)->tp_name,
		self->rect.origin.x, self->rect.origin.y,
		self->rect.size.width, self->rect.size.height);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCRectRichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKRect* rc1 = DCRectToObject(obj1);
	DKRect* rc2 = DCRectToObject(obj2);
	if (rc1 && rc2)
	{
		if (op == Py_EQ)
		{
			if (*rc1 == *rc2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
		else if (op == Py_NE)
		{
			if (*rc1 != *rc2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCRectIsInside(DCRect* self, PyObject* args)
{
	auto setTypeError = []()->PyObject*
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be two floating point tuple or Point object.");
		return NULL;
	};

	DKPoint pt;
	Py_ssize_t size = PyTuple_Size(args);
	if (size == 2)
	{
		if (!PyArg_ParseTuple(args, "ff", &pt.x, &pt.y))
			return setTypeError();
	}
	else
	{
		PyObject* obj = NULL;
		if (!PyArg_ParseTuple(args, "O", &obj))
			return setTypeError();

		DKPoint* p = DCPointToObject(obj);
		if (p)
		{
			pt = *p;
		}
		else
		{
			DKVector2* v = DCVector2ToObject(obj);
			if (v)
				pt = DKPoint(v->x, v->y);
			else
				return setTypeError();
		}
	}

	if (self->rect.IsPointInside(pt)) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCRectIntersect(DCRect* self, PyObject* args)
{
	PyObject* obj = NULL;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	// obj 는 DCRect, DCLine, DCTriangle 중 하나.
	// DCRect 만 우선 구현함. (2014-01-20)
	DKRect* rc = DCRectToObject(obj);
	if (rc)
	{
		if (self->rect.Intersect(*rc)) { Py_RETURN_TRUE; }
		else { Py_RETURN_FALSE; }
	}
	PyErr_SetString(PyExc_TypeError, "argument must be one of following type: Rect, Line, Triangle.");
	return NULL;
}

static PyObject* DCRectIntersection(DCRect* self, PyObject* args)
{
	PyObject* obj = NULL;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKRect* rc = DCRectToObject(obj);
	if (rc == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Rect object.");
		return NULL;
	}

	DKRect tmp = DKRect::Intersection(self->rect, *rc);
	return DCRectFromObject(&tmp);
}

static PyObject* DCRectUnion(DCRect* self, PyObject* args)
{
	PyObject* obj = NULL;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKRect* rc = DCRectToObject(obj);
	if (rc == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Rect object.");
		return NULL;
	}

	DKRect tmp = DKRect::Union(self->rect, *rc);
	return DCRectFromObject(&tmp);
}

static PyObject* DCRectIsValid(DCRect* self, PyObject*)
{
	if (self->rect.IsValid())
		Py_RETURN_TRUE;
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "isInside", (PyCFunction)&DCRectIsInside, METH_VARARGS },
	{ "intersect", (PyCFunction)&DCRectIntersect, METH_VARARGS },
	{ "intersection", (PyCFunction)&DCRectIntersection, METH_VARARGS },
	{ "union", (PyCFunction)&DCRectUnion, METH_VARARGS },
	{ "isValid", (PyCFunction)&DCRectIsValid, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCRectGetAttr(DCRect* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->rect);
	return PyFloat_FromDouble(*attr);
}

static int DCRectSetAttr(DCRect* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->rect);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCRectOrigin(DCRect* self, void*)
{
	DKPoint& pt = self->rect.origin;
	return Py_BuildValue("ff", pt.x, pt.y);
}

static int DCRectSetOrigin(DCRect* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKPoint pt;
	if (DCPointConverter(value, &pt))
	{
		self->rect.origin = pt;
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Point object or tow floating point tuple.");
	return -1;
}

static PyObject* DCRectSize(DCRect* self, void*)
{
	DKSize& sz = self->rect.size;
	return Py_BuildValue("ff", sz.width, sz.height);
}

static int DCRectSetSize(DCRect* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKSize sz;
	if (DCSizeConverter(value, &sz))
	{
		self->rect.size = sz;
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Size object or tow floating point tuple.");
	return -1;
}

static PyObject* DCRectCenter(DCRect* self, void*)
{
	DKPoint pt = self->rect.Center();
	return Py_BuildValue("ff", pt.x, pt.y);
}

static int DCRectSetCenter(DCRect* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKPoint pt;
	if (DCPointConverter(value, &pt))
	{
		self->rect.SetCenter(pt);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Point object or tow floating point tuple.");
	return -1;
}

static PyObject* DCRectTuple(DCRect* self, void*)
{
	return Py_BuildValue("ffff",
		self->rect.origin.x, self->rect.origin.y,
		self->rect.size.width, self->rect.size.height);
}

static int DCRectSetTuple(DCRect* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffff",
		&self->rect.origin.x, &self->rect.origin.y,
		&self->rect.size.width, &self->rect.size.height))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of four floating point numbers (x, y, w, h).");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "x", (getter)&DCRectGetAttr, (setter)&DCRectSetAttr, 0, reinterpret_cast<void*>(offsetof(DKRect, origin.x)) },
	{ "y", (getter)&DCRectGetAttr, (setter)&DCRectSetAttr, 0, reinterpret_cast<void*>(offsetof(DKRect, origin.y)) },
	{ "width", (getter)&DCRectGetAttr, (setter)&DCRectSetAttr, 0, reinterpret_cast<void*>(offsetof(DKRect, size.width)) },
	{ "height", (getter)&DCRectGetAttr, (setter)&DCRectSetAttr, 0, reinterpret_cast<void*>(offsetof(DKRect, size.height)) },
	{ "origin", (getter)&DCRectOrigin, (setter)&DCRectSetOrigin, 0, 0 },
	{ "size", (getter)&DCRectSize, (setter)&DCRectSetSize, 0, 0 },
	{ "center", (getter)&DCRectCenter, (setter)&DCRectSetCenter, 0, 0 },
	{ "tuple", (getter)&DCRectTuple, (setter)&DCRectSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Rect",						/* tp_name */
	sizeof(DCRect),									/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCRectDealloc,						/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCRectRepr,							/* tp_repr */
	0,												/* tp_as_number */
	0,												/* tp_as_sequence */
	0,												/* tp_as_mapping */
	0,												/* tp_hash  */
	0,												/* tp_call */
	0,												/* tp_str */
	0,												/* tp_getattro */
	0,												/* tp_setattro */
	0,												/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		/* tp_flags */
	0,												/* tp_doc */
	0,												/* tp_traverse */
	0,												/* tp_clear */
	(richcmpfunc)&DCRectRichCompare,				/* tp_richcompare */
	0,												/* tp_weaklistoffset */
	0,												/* tp_iter */
	0,												/* tp_iternext */
	methods,										/* tp_methods */
	0,												/* tp_members */
	getsets,										/* tp_getset */
	0,												/* tp_base */
	0,												/* tp_dict */
	0,												/* tp_descr_get */
	0,												/* tp_descr_set */
	0,												/* tp_dictoffset */
	(initproc)&DCRectInit,						/* tp_init */
	0,												/* tp_alloc */
	&DCRectNew,									/* tp_new */
};

PyTypeObject* DCRectTypeObject(void)
{
	return &objectType;
}

PyObject* DCRectFromObject(DKRect* rect)
{
	if (rect)
	{
		PyObject* args = Py_BuildValue("ffff", rect->origin.x, rect->origin.y, rect->size.width, rect->size.height);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		PyObject* self = PyObject_Call(tp, args, kwds);
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return self;
	}
	Py_RETURN_NONE;
}

DKRect* DCRectToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCRect*)obj)->rect;
	}
	return NULL;
}

int DCRectConverter(PyObject* obj, DKRect* p)
{
	DKRect* tmp = DCRectToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		Py_ssize_t numArgs = PyTuple_GET_SIZE(obj);
		if (numArgs == 4)
		{
			if (PyArg_ParseTuple(obj, "ffff",
				&p->origin.x, &p->origin.y,
				&p->size.width, &p->size.height))
				return true;
		}
		else if (numArgs == 2)
		{
			DKPoint pt;
			DKSize sz;
			if (PyArg_ParseTuple(obj, "O&O&", &DCPointConverter, &pt, &DCSizeConverter, &sz))
			{
				p->origin = pt;
				p->size = sz;
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Rect object or Point, Size object tuple or four floats tuple.");
	return false;
}
