#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

struct DCVector2
{
	PyObject_HEAD
	DKVector2 vector2;
};

static PyObject* DCVector2New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCVector2* self = (DCVector2*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->vector2) DKVector2();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCVector2Init(DCVector2 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCVector2Converter(args, &self->vector2))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCVector2Converter, &self->vector2))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->vector2 = DKVector2(0, 0);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be two floating points (x,y) or empty");
	return -1;
}

static void DCVector2Dealloc(DCVector2* self)
{
	self->vector2.~DKVector2();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCVector2Repr(DCVector2* self)
{
	DKString str = DKString::Format("<%s object (x:%.3f, y:%.3f)>",
		Py_TYPE(self)->tp_name, self->vector2.x, self->vector2.y);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCVector2RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKVector2* vec1 = DCVector2ToObject(obj1);
	DKVector2* vec2 = DCVector2ToObject(obj2);
	if (vec1 && vec2)
	{
		if (op == Py_EQ)
		{
			if (*vec1 == *vec2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
		else if (op == Py_NE)
		{
			if (*vec1 != *vec2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCVector2Length(DCVector2* self, PyObject*)
{
	return PyFloat_FromDouble(self->vector2.Length());
}

static PyObject* DCVector2LengthSq(DCVector2* self, PyObject*)
{
	return PyFloat_FromDouble(self->vector2.LengthSq());
}

static PyObject* DCVector2Normalize(DCVector2* self, PyObject*)
{
	self->vector2.Normalize();
	Py_RETURN_NONE;
}

static PyObject* DCVector2Dot(DCVector2* self, PyObject* args)
{
	DKVector2 vec;
	if (PyArg_ParseTuple(args, "O&", &DCVector2Converter, &vec))
	{
		float d = DKVector2::Dot(self->vector2, vec);
		return PyFloat_FromDouble(d);
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Vector2 object.");
	return NULL;
}

static PyObject* DCVector2Transform(DCVector2* self, PyObject* args)
{
	PyObject* obj;
	if (PyArg_ParseTuple(args, "O", &obj))
	{
		DKMatrix2* mat2 = DCMatrix2ToObject(obj);
		if (mat2)
		{
			self->vector2.Transform(*mat2);
			Py_RETURN_NONE;
		}
		DKMatrix3* mat3 = DCMatrix3ToObject(obj);
		if (mat3)
		{
			self->vector2.Transform(*mat3);
			Py_RETURN_NONE;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Matrix2 or Matrix3 object.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "length", (PyCFunction)&DCVector2Length, METH_NOARGS },
	{ "lengthSq", (PyCFunction)&DCVector2LengthSq, METH_NOARGS },
	{ "normalize", (PyCFunction)&DCVector2Normalize, METH_NOARGS },
	{ "dot", (PyCFunction)&DCVector2Dot, METH_VARARGS, "dot-product" },
	{ "transform", (PyCFunction)&DCVector2Transform, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCVector2GetAttr(DCVector2* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->vector2);
	return PyFloat_FromDouble(*attr);
}

static int DCVector2SetAttr(DCVector2* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->vector2);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCVector2Tuple(DCVector2* self, void*)
{
	return Py_BuildValue("ff", self->vector2.x, self->vector2.y);
}

static int DCVector2SetTuple(DCVector2* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ff", &self->vector2.x, &self->vector2.y))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of two floating point numbers (x, y).");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "x", (getter)&DCVector2GetAttr, (setter)&DCVector2SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector2, x)) },
	{ "y", (getter)&DCVector2GetAttr, (setter)&DCVector2SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector2, y)) },
	{ "tuple", (getter)&DCVector2Tuple, (setter)&DCVector2SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCVector2UnaryOperand(PyObject* obj, bool(*fn)(const DKVector2&, DKVector2&))
{
	DKVector2* v = DCVector2ToObject(obj);
	if (v)
	{
		DKVector2 res;
		if (fn(*v, res))
			return DCVector2FromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector2BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector2&, const DKVector2&, DKVector2&))
{
	DKVector2* p = DCVector2ToObject(lhs);
	DKVector2 v;
	if (p && DCVector2Converter(rhs, &v))
	{
		DKVector2 res;
		if (fn(*p, v, res))
			return DCVector2FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector2BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector2&, const DKMatrix2&, DKVector2&))
{
	DKVector2* p = DCVector2ToObject(lhs);
	DKMatrix2 m;
	if (p && DCMatrix2Converter(rhs, &m))
	{
		DKVector2 res;
		if (fn(*p, m, res))
			return DCVector2FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector2BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector2&, double, DKVector2&))
{
	DKVector2* p = DCVector2ToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKVector2 res;
		if (fn(*p, d, res))
			return DCVector2FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCVector2Add(PyObject* lhs, PyObject* rhs)
{
	return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v1, const DKVector2& v2, DKVector2& r)
	{
		r = v1 + v2;
		return true;
	});
}

static PyObject* DCVector2Subtract(PyObject* lhs, PyObject* rhs)
{
	return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v1, const DKVector2& v2, DKVector2& r)
	{
		r = v1 - v2;
		return true;
	});
}

static PyObject* DCVector2Multiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector2TypeObject()))
		return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v1, const DKVector2& v2, DKVector2& r)
	{
		r = v1 * v2;
		return true;
	});
	if (PyObject_TypeCheck(rhs, DCMatrix2TypeObject()))
		return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v, const DKMatrix2& m, DKVector2& r)
	{
		r = v * m;
		return true;
	});
	return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v, double d, DKVector2& r)
	{
		r = v * d;
		return true;
	});
}

static PyObject* DCVector2Negative(PyObject* obj)
{
	return DCVector2UnaryOperand(obj, [](const DKVector2& v, DKVector2& r)
	{
		r = DKVector2(-v.x, -v.y);
		return true;
	});
}

static PyObject* DCVector2Positive(PyObject* obj)
{
	return DCVector2UnaryOperand(obj, [](const DKVector2& v, DKVector2& r)
	{
		r = v;
		return true;
	});
}

static PyObject* DCVector2Absolute(PyObject* obj)
{
	return DCVector2UnaryOperand(obj, [](const DKVector2& v, DKVector2& r)
	{
		r = DKVector2((v.x < 0 ? -v.x : v.x), (v.y < 0 ? -v.y : v.y));
		return true;
	});
}

static PyObject* DCVector2FloorDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector2TypeObject()))
		return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v1, const DKVector2& v2, DKVector2& r)
	{
		if (v2.x != 0.0f && v2.y != 0.0f)
		{
			r = v1 / v2;
			r.x = floor(r.x);
			r.y = floor(r.y);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v, double d, DKVector2& r)
	{
		if (d != 0.0)
		{
			r = v / d;
			r.x = floor(r.x);
			r.y = floor(r.y);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyObject* DCVector2TrueDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector2TypeObject()))
		return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v1, const DKVector2& v2, DKVector2& r)
	{
		if (v2.x != 0.0f && v2.y != 0.0f)
		{
			r = v1 / v2;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	return DCVector2BinaryOperand(lhs, rhs, [](const DKVector2& v, double d, DKVector2& r)
	{
		if (d != 0.0)
		{
			r = v / d;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyNumberMethods numberMethods = {
	(binaryfunc)&DCVector2Add,			/* nb_add */
	(binaryfunc)&DCVector2Subtract,		/* nb_subtract */
	(binaryfunc)&DCVector2Multiply,		/* nb_multiply */
	0,									/* nb_remainder */
	0,									/* nb_divmod */
	0,									/* nb_power */
	(unaryfunc)&DCVector2Negative,		/* nb_negative */
	(unaryfunc)&DCVector2Positive,		/* nb_positive */
	(unaryfunc)&DCVector2Absolute,		/* nb_absolute */
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
	(binaryfunc)&DCVector2FloorDivide,	/* nb_floor_divide */
	(binaryfunc)&DCVector2TrueDivide,	/* nb_true_divide */
	0,									/* nb_inplace_floor_divide */
	0,									/* nb_inplace_true_divide */
	0									/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Vector2",					/* tp_name */
	sizeof(DCVector2),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCVector2Dealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCVector2Repr,						/* tp_repr */
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
	(richcmpfunc)&DCVector2RichCompare,				/* tp_richcompare */
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
	(initproc)&DCVector2Init,						/* tp_init */
	0,												/* tp_alloc */
	&DCVector2New,									/* tp_new */
};

PyTypeObject* DCVector2TypeObject(void)
{
	return &objectType;
}

PyObject* DCVector2FromObject(DKVector2* vector2)
{
	if (vector2)
	{
		PyObject* args = Py_BuildValue("ff", vector2->x, vector2->y);
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

DKVector2* DCVector2ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCVector2*)obj)->vector2;
	}
	return NULL;
}

int DCVector2Converter(PyObject* obj, DKVector2* p)
{
	DKVector2* tmp = DCVector2ToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		if (PyArg_ParseTuple(obj, "ff", &p->x, &p->y))
			return true;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Vector2 object or two floats tuple.");
	return false;
}
