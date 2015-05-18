#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCPoint
{
	PyObject_HEAD
	DKPoint point;
};

static PyObject* DCPointNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCPoint* self = (DCPoint*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->point) DKPoint();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCPointInit(DCPoint *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCPointConverter(args, &self->point))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCPointConverter, &self->point))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->point = DKPoint(0, 0);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be two floating point (x,y) or empty");
	return -1;
}

static void DCPointDealloc(DCPoint* self)
{
	self->point.~DKPoint();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCPointRepr(DCPoint* self)
{
	DKString str = DKString::Format("<%s object (x:%.3f, y:%.3f)>",
		Py_TYPE(self)->tp_name, self->point.x, self->point.y);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCPointRichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKPoint* pt1 = DCPointToObject(obj1);
	DKPoint* pt2 = DCPointToObject(obj2);
	if (pt1 && pt2)
	{
		if (op == Py_EQ)
		{
			if (*pt1 == *pt2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
		else if (op == Py_NE)
		{
			if (*pt1 != *pt2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCPointVector(DCPoint* self, PyObject*)
{
	DKVector2 vec = self->point.Vector();
	return DCVector2FromObject(&vec);
}

static PyMethodDef methods[] = {
	{ "vector", (PyCFunction)&DCPointVector, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCPointGetAttr(DCPoint* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->point);
	return PyFloat_FromDouble(*attr);
}

static int DCPointSetAttr(DCPoint* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->point);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCPointTuple(DCPoint* self, void*)
{
	return Py_BuildValue("ff", self->point.x, self->point.y);
}

static int DCPointSetTuple(DCPoint* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ff", &self->point.x, &self->point.y))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of two floating point numbers (x, y).");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "x", (getter)&DCPointGetAttr, (setter)&DCPointSetAttr, 0, reinterpret_cast<void*>(offsetof(DKPoint, x)) },
	{ "y", (getter)&DCPointGetAttr, (setter)&DCPointSetAttr, 0, reinterpret_cast<void*>(offsetof(DKPoint, y)) },
	{ "tuple", (getter)&DCPointTuple, (setter)&DCPointSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCPointUnaryOperand(PyObject* obj, bool(*fn)(const DKPoint&, DKPoint&))
{
	DKPoint* pt = DCPointToObject(obj);
	if (pt)
	{
		DKPoint res;
		if (fn(*pt, res))
			return DCPointFromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCPointBinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKPoint&, const DKPoint&, DKPoint&))
{
	DKPoint* p = DCPointToObject(lhs);
	DKPoint pt;
	if (p && DCPointConverter(rhs, &pt))
	{
		DKPoint res;
		if (fn(*p, pt, res))
			return DCPointFromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCPointBinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKPoint&, double, DKPoint&))
{
	DKPoint* pt = DCPointToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKPoint res;
		if (fn(*pt, d, res))
			return DCPointFromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCPointAdd(PyObject* lhs, PyObject* rhs)
{
	return DCPointBinaryOperand(lhs, rhs, [](const DKPoint& pt1, const DKPoint& pt2, DKPoint& r)
	{
		r = pt1 + pt2;
		return true;
	});
}

static PyObject* DCPointSubtract(PyObject* lhs, PyObject* rhs)
{
	return DCPointBinaryOperand(lhs, rhs, [](const DKPoint& pt1, const DKPoint& pt2, DKPoint& r)
	{
		r = pt1 - pt2;
		return true;
	});
}

static PyObject* DCPointMultiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCPointTypeObject()))
		return DCPointBinaryOperand(lhs, rhs, [](const DKPoint& pt1, const DKPoint& pt2, DKPoint& r)
	{
		r = pt1 * pt2;
		return true;
	});
	return DCPointBinaryOperand(lhs, rhs, [](const DKPoint& pt, double d, DKPoint& r)
	{
		r = pt * d;
		return true;
	});
}

static PyObject* DCPointNegative(PyObject* obj)
{
	return DCPointUnaryOperand(obj, [](const DKPoint& pt, DKPoint& r)
	{
		r = DKPoint(-pt.x, -pt.y);
		return true;
	});
}

static PyObject* DCPointPositive(PyObject* obj)
{
	return DCPointUnaryOperand(obj, [](const DKPoint& pt, DKPoint& r)
	{
		r = pt;
		return true;
	});
}

static PyObject* DCPointAbsolute(PyObject* obj)
{
	return DCPointUnaryOperand(obj, [](const DKPoint& pt, DKPoint& r)
	{
		r = DKPoint((pt.x < 0 ? -pt.x : pt.x), (pt.y < 0 ? -pt.y : pt.y));
		return true;
	});
}

static PyObject* DCPointFloorDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCPointTypeObject()))
		return DCPointBinaryOperand(lhs, rhs, [](const DKPoint& pt1, const DKPoint& pt2, DKPoint& r)
	{
		if (pt2.x != 0 && pt2.y != 0)
		{
			r = pt1 / pt2;
			r.x = floor(r.x);
			r.y = floor(r.y);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	
	return DCPointBinaryOperand(lhs, rhs, [](const DKPoint& pt, double d, DKPoint& r)
	{
		if (d != 0.0)
		{
			r = pt / d;
			r.x = floor(r.x);
			r.y = floor(r.y);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyObject* DCPointTrueDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCPointTypeObject()))
		return DCPointBinaryOperand(lhs, rhs, [](const DKPoint& pt1, const DKPoint& pt2, DKPoint& r)
	{
		if (pt2.x != 0 && pt2.y != 0)
		{
			r = pt1 / pt2;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});

	return DCPointBinaryOperand(lhs, rhs, [](const DKPoint& pt, double d, DKPoint& r)
	{
		if (d != 0.0)
		{
			r = pt / d;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyNumberMethods numberMethods = {
	(binaryfunc)&DCPointAdd,			/* nb_add */
	(binaryfunc)&DCPointSubtract,		/* nb_subtract */
	(binaryfunc)&DCPointMultiply,		/* nb_multiply */
	0,									/* nb_remainder */
	0,									/* nb_divmod */
	0,									/* nb_power */
	(unaryfunc)&DCPointNegative,		/* nb_negative */
	(unaryfunc)&DCPointPositive,		/* nb_positive */
	(unaryfunc)&DCPointAbsolute,		/* nb_absolute */
	0,									/* nb_bool */
	0,									/* nb_invert */
	0,									/* nb_lshift */
	0,									/* nb_rshift */
	0,									/* nb_and */
	0,									/* nb_xor */
	0,									/* nb_or */
	0,									/* nb_int */
	0,									/* nb_reserved */
	0,									/* nb_float */
	0,									/* nb_inplace_add */
	0,									/* nb_inplace_subtract */
	0,									/* nb_inplace_multiply */
	0,									/* nb_inplace_remainder */
	0,									/* nb_inplace_power */
	0,									/* nb_inplace_lshift */
	0,									/* nb_inplace_rshift */
	0,									/* nb_inplace_and */
	0,									/* nb_inplace_xor */
	0,									/* nb_inplace_or */
	(binaryfunc)&DCPointFloorDivide,	/* nb_floor_divide */
	(binaryfunc)&DCPointTrueDivide,		/* nb_true_divide */
	0,									/* nb_inplace_floor_divide */
	0,									/* nb_inplace_true_divide */
	0									/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Point",						/* tp_name */
	sizeof(DCPoint),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCPointDealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCPointRepr,							/* tp_repr */
	&numberMethods,									/* tp_as_number */
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
	(richcmpfunc)&DCPointRichCompare,				/* tp_richcompare */
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
	(initproc)&DCPointInit,							/* tp_init */
	0,												/* tp_alloc */
	&DCPointNew,									/* tp_new */
};

PyTypeObject* DCPointTypeObject(void)
{
	return &objectType;
}

PyObject* DCPointFromObject(DKPoint* point)
{
	if (point)
	{
		PyObject* args = Py_BuildValue("ff", point->x, point->y);
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

DKPoint* DCPointToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCPoint*)obj)->point;
	}
	return NULL;
}

int DCPointConverter(PyObject* obj, DKPoint* p)
{
	DKPoint* tmp = DCPointToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	DKVector2* vec = DCVector2ToObject(obj);
	if (vec)
	{
		*p = *vec;
		return true;
	}
	if (obj && PyTuple_Check(obj))
	{
		if (PyArg_ParseTuple(obj, "ff", &p->x, &p->y))
			return true;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Point object or two floats tuple.");
	return false;
}
