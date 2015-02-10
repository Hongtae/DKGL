#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCMatrix2
{
	PyObject_HEAD
	DKMatrix2 matrix2;
};

static PyObject* DCMatrix2New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCMatrix2* self = (DCMatrix2*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->matrix2) DKMatrix2();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCMatrix2Init(DCMatrix2 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	// (vec2, vec2) or (float x 4)
	if (numArgs > 1 && DCMatrix2Converter(args, &self->matrix2))
	{
		return 0;
	}
	// matrix2
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCMatrix2Converter, &self->matrix2))
	{
		return 0;
	}
	// default
	else if (numArgs == 0)
	{
		self->matrix2.Identity();
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be two vector2 or four floating points or empty.");
	return -1;
}

static void DCMatrix2Dealloc(DCMatrix2* self)
{
	self->matrix2.~DKMatrix2();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCMatrix2Repr(DCMatrix2* self)
{
	DKString str = DKString::Format(
		"<%s object\n"
		"  (%.3f, %.3f)\n"
		"  (%.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		self->matrix2.m[0][0], self->matrix2.m[0][1],
		self->matrix2.m[1][0], self->matrix2.m[1][1]);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCMatrix2RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKMatrix2* m1 = DCMatrix2ToObject(obj1);
	DKMatrix2* m2 = DCMatrix2ToObject(obj2);
	if (m1 && m2)
	{
		if (op == Py_EQ)
		{
			if (*m1 == *m2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
		else if (op == Py_NE)
		{
			if (*m1 != *m2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCMatrix2IsIdentity(DCMatrix2* self, PyObject*)
{
	if (self->matrix2.IsIdentity()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCMatrix2IsDiagonal(DCMatrix2* self, PyObject*)
{
	if (self->matrix2.IsDiagonal()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCMatrix2Determinant(DCMatrix2* self, PyObject*)
{
	return PyFloat_FromDouble(self->matrix2.Determinant());
}

static PyObject* DCMatrix2Inverse(DCMatrix2* self, PyObject*)
{
	self->matrix2.Inverse();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix2Transpose(DCMatrix2* self, PyObject*)
{
	self->matrix2.Transpose();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix2Zero(DCMatrix2* self, PyObject*)
{
	self->matrix2.Zero();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix2Identity(DCMatrix2* self, PyObject*)
{
	self->matrix2.Identity();
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "isIdentity", (PyCFunction)&DCMatrix2IsIdentity, METH_NOARGS },
	{ "isDiagonal", (PyCFunction)&DCMatrix2IsDiagonal, METH_NOARGS },
	{ "determinant", (PyCFunction)&DCMatrix2Determinant, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCMatrix2Inverse, METH_NOARGS },
	{ "transpose", (PyCFunction)&DCMatrix2Transpose, METH_NOARGS },
	{ "zero", (PyCFunction)&DCMatrix2Zero, METH_NOARGS },
	{ "identity", (PyCFunction)&DCMatrix2Identity, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCMatrix2GetAttr(DCMatrix2* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->matrix2);
	return PyFloat_FromDouble(*attr);
}

static int DCMatrix2SetAttr(DCMatrix2* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->matrix2);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCMatrix2RowTuple(DCMatrix2* self, void* closure)
{
	uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
	return Py_BuildValue("ff", self->matrix2.m[idx][0], self->matrix2.m[idx][1]);
}

static int DCMatrix2SetRowTuple(DCMatrix2* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector2 vec;
	if (DCVector2Converter(value, &vec))
	{
		uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
		self->matrix2.m[idx][0] = vec.x;
		self->matrix2.m[idx][1] = vec.y;
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Vector2 or two floating point numbers.");
	return -1;
}

static PyObject* DCMatrix2ColumnTuple(DCMatrix2* self, void* closure)
{
	uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
	return Py_BuildValue("ff", self->matrix2.m[0][idx], self->matrix2.m[1][idx]);
}

static int DCMatrix2SetColumnTuple(DCMatrix2* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector2 vec;
	if (DCVector2Converter(value, &vec))
	{
		uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
		self->matrix2.m[0][idx] = vec.x;
		self->matrix2.m[1][idx] = vec.y;
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Vector2 or two floating point numbers.");
	return -1;
}

static PyObject* DCMatrix2Tuple(DCMatrix2* self, void*)
{
	return Py_BuildValue("ffff",
		self->matrix2.m[0][1], self->matrix2.m[0][1],
		self->matrix2.m[1][1], self->matrix2.m[1][1]);
}

static int DCMatrix2SetTuple(DCMatrix2* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffff",
		&self->matrix2.m[0][0], &self->matrix2.m[0][1],
		&self->matrix2.m[1][0], &self->matrix2.m[1][1]))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of nine floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "m11", (getter)&DCMatrix2GetAttr, (setter)&DCMatrix2SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix2, _11)) },
	{ "m12", (getter)&DCMatrix2GetAttr, (setter)&DCMatrix2SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix2, _12)) },
	{ "m21", (getter)&DCMatrix2GetAttr, (setter)&DCMatrix2SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix2, _21)) },
	{ "m22", (getter)&DCMatrix2GetAttr, (setter)&DCMatrix2SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix2, _22)) },
	{ "row1", (getter)&DCMatrix2RowTuple, (setter)&DCMatrix2SetRowTuple, 0, reinterpret_cast<void*>(0) },
	{ "row2", (getter)&DCMatrix2RowTuple, (setter)&DCMatrix2SetRowTuple, 0, reinterpret_cast<void*>(1) },
	{ "column1", (getter)&DCMatrix2ColumnTuple, (setter)&DCMatrix2SetColumnTuple, 0, reinterpret_cast<void*>(0) },
	{ "column2", (getter)&DCMatrix2ColumnTuple, (setter)&DCMatrix2SetColumnTuple, 0, reinterpret_cast<void*>(1) },
	{ "tuple", (getter)&DCMatrix2Tuple, (setter)&DCMatrix2SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCMatrix2UnaryOperand(PyObject* obj, bool(*fn)(const DKMatrix2&, DKMatrix2&))
{
	DKMatrix2* mat = DCMatrix2ToObject(obj);
	if (mat)
	{
		DKMatrix2 res;
		if (fn(*mat, res))
			return DCMatrix2FromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCMatrix2BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKMatrix2&, const DKMatrix2&, DKMatrix2&))
{
	DKMatrix2* p = DCMatrix2ToObject(lhs);
	DKMatrix2 mat;
	if (p && DCMatrix2Converter(rhs, &mat))
	{
		DKMatrix2 res;
		if (fn(*p, mat, res))
			return DCMatrix2FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCMatrix2BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKMatrix2&, double, DKMatrix2&))
{
	DKMatrix2* mat = DCMatrix2ToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKMatrix2 res;
		if (fn(*mat, d, res))
			return DCMatrix2FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCMatrix2Add(PyObject* lhs, PyObject* rhs)
{
	return DCMatrix2BinaryOperand(lhs, rhs, [](const DKMatrix2& m1, const DKMatrix2& m2, DKMatrix2& r)
	{
		r = m1 + m2;
		return true;
	});
}

static PyObject* DCMatrix2Subtract(PyObject* lhs, PyObject* rhs)
{
	return DCMatrix2BinaryOperand(lhs, rhs, [](const DKMatrix2& m1, const DKMatrix2& m2, DKMatrix2& r)
	{
		r = m1 - m2;
		return true;
	});
}

static PyObject* DCMatrix2Multiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCMatrix2TypeObject()))
		return DCMatrix2BinaryOperand(lhs, rhs, [](const DKMatrix2& m1, const DKMatrix2& m2, DKMatrix2& r)
	{
		r = m1 * m2;
		return true;
	});
	return DCMatrix2BinaryOperand(lhs, rhs, [](const DKMatrix2& m, double d, DKMatrix2& r)
	{
		r = m * d;
		return true;
	});
}

static PyObject* DCMatrix2Negative(PyObject* obj)
{
	return DCMatrix2UnaryOperand(obj, [](const DKMatrix2& m, DKMatrix2& r)
	{
		r = m * -1.0f;
		return true;
	});
}

static PyObject* DCMatrix2Positive(PyObject* obj)
{
	return DCMatrix2UnaryOperand(obj, [](const DKMatrix2& m, DKMatrix2& r)
	{
		r = m;
		return true;
	});
}

static PyObject* DCMatrix2Absolute(PyObject* obj)
{
	return DCMatrix2UnaryOperand(obj, [](const DKMatrix2& m, DKMatrix2& r)
	{
		r = m;
		for (float& f : r.val)
		{
			if (f < 0)
				f = -f;
		}
		return true;
	});
}

static PyObject* DCMatrix2FloorDivide(PyObject *lhs, PyObject *rhs)
{
	return DCMatrix2BinaryOperand(lhs, rhs, [](const DKMatrix2& m, double d, DKMatrix2& r)
	{
		if (d != 0.0)
		{
			r = m / d;
			for (float& f : r.val)
				f = floor(f);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyObject* DCMatrix2TrueDivide(PyObject *lhs, PyObject *rhs)
{
	return DCMatrix2BinaryOperand(lhs, rhs, [](const DKMatrix2& m, double d, DKMatrix2& r)
	{
		if (d != 0.0)
		{
			r = m / d;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyNumberMethods numberMethods = {
	(binaryfunc)&DCMatrix2Add,						/* nb_add */
	(binaryfunc)&DCMatrix2Subtract,					/* nb_subtract */
	(binaryfunc)&DCMatrix2Multiply,					/* nb_multiply */
	0,												/* nb_remainder */
	0,												/* nb_divmod */
	0,												/* nb_power */
	(unaryfunc)&DCMatrix2Negative,					/* nb_negative */
	(unaryfunc)&DCMatrix2Positive,					/* nb_positive */
	(unaryfunc)&DCMatrix2Absolute,					/* nb_absolute */
	0,												/* nb_bool */
	0,												/* nb_invert */
	0,												/* nb_lshift */
	0,												/* nb_rshift */
	0,												/* nb_and */
	0,												/* nb_xor */
	0,												/* nb_or */
	0,												/* nb_int */
	0,												/* nb_reserved */
	0,												/* nb_float */
	0,												/* nb_inplace_add */
	0,												/* nb_inplace_subtract */
	0,												/* nb_inplace_multiply */
	0,												/* nb_inplace_remainder */
	0,												/* nb_inplace_power */
	0,												/* nb_inplace_lshift */
	0,												/* nb_inplace_rshift */
	0,												/* nb_inplace_and */
	0,												/* nb_inplace_xor */
	0,												/* nb_inplace_or */
	(binaryfunc)&DCMatrix2FloorDivide,				/* nb_floor_divide */
	(binaryfunc)&DCMatrix2TrueDivide,				/* nb_true_divide */
	0,												/* nb_inplace_floor_divide */
	0,												/* nb_inplace_true_divide */
	0												/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Matrix2",					/* tp_name */
	sizeof(DCMatrix2),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCMatrix2Dealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCMatrix2Repr,						/* tp_repr */
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
	(richcmpfunc)&DCMatrix2RichCompare,				/* tp_richcompare */
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
	(initproc)&DCMatrix2Init,						/* tp_init */
	0,												/* tp_alloc */
	&DCMatrix2New,									/* tp_new */
};

PyTypeObject* DCMatrix2TypeObject(void)
{
	return &objectType;
}

PyObject* DCMatrix2FromObject(DKMatrix2* matrix2)
{
	if (matrix2)
	{
		PyObject* args = Py_BuildValue("ffff",
			matrix2->m[0][0], matrix2->m[0][1],
			matrix2->m[1][0], matrix2->m[1][1]);
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

DKMatrix2* DCMatrix2ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCMatrix2*)obj)->matrix2;
	}
	return NULL;
}

int DCMatrix2Converter(PyObject* obj, DKMatrix2* p)
{
	DKMatrix2* tmp = DCMatrix2ToObject(obj);
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
				&p->m[0][0], &p->m[0][1],
				&p->m[1][0], &p->m[1][1]))
				return true;
		}
		else if (numArgs == 2)
		{
			DKVector2 row[2];
			if (PyArg_ParseTuple(obj, "O&O&",
				&DCVector2Converter, &row[0],
				&DCVector2Converter, &row[1]))
			{
				*p = DKMatrix2(row[0], row[1]);
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Matrix2 object or four floats tuple.");
	return false;
}
