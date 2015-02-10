#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCMatrix4
{
	PyObject_HEAD
	DKMatrix4 matrix4;
};

static PyObject* DCMatrix4New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCMatrix4* self = (DCMatrix4*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->matrix4) DKMatrix4();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCMatrix4Init(DCMatrix4 *self, PyObject *args, PyObject *kwds)
{
	// (vec4, vec4, vec4, vec4) - row1, row2, row3, row4
	// (float x 16)
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	// (row1,row2,row3,row4) or (float x 16)
	if (numArgs > 1 && DCMatrix4Converter(args, &self->matrix4))
	{
		return 0;
	}
	// matrix4
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCMatrix4Converter, &self->matrix4))
	{
		return 0;
	}
	// default
	else if (numArgs == 0)
	{
		self->matrix4.Identity();
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be four vector4 or sixteen floating points or empty.");
	return -1;
}

static void DCMatrix4Dealloc(DCMatrix4* self)
{
	self->matrix4.~DKMatrix4();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCMatrix4Repr(DCMatrix4* self)
{
	DKString str = DKString::Format(
		"<%s object\n"
		"  (%.3f, %.3f, %.3f, %.3f)\n"
		"  (%.3f, %.3f, %.3f, %.3f)\n"
		"  (%.3f, %.3f, %.3f, %.3f)\n"
		"  (%.3f, %.3f, %.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		self->matrix4.m[0][0], self->matrix4.m[0][1], self->matrix4.m[0][2], self->matrix4.m[0][3],
		self->matrix4.m[1][0], self->matrix4.m[1][1], self->matrix4.m[1][2], self->matrix4.m[1][3],
		self->matrix4.m[2][0], self->matrix4.m[2][1], self->matrix4.m[2][2], self->matrix4.m[2][3],
		self->matrix4.m[3][0], self->matrix4.m[3][1], self->matrix4.m[3][2], self->matrix4.m[3][3]);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCMatrix4RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKMatrix4* m1 = DCMatrix4ToObject(obj1);
	DKMatrix4* m2 = DCMatrix4ToObject(obj2);
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

static PyObject* DCMatrix4IsIdentity(DCMatrix4* self, PyObject*)
{
	if (self->matrix4.IsIdentity()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCMatrix4IsDiagonal(DCMatrix4* self, PyObject*)
{
	if (self->matrix4.IsDiagonal()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCMatrix4Determinant(DCMatrix4* self, PyObject*)
{
	return PyFloat_FromDouble(self->matrix4.Determinant());
}

static PyObject* DCMatrix4Inverse(DCMatrix4* self, PyObject*)
{
	self->matrix4.Inverse();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix4Transpose(DCMatrix4* self, PyObject*)
{
	self->matrix4.Transpose();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix4Zero(DCMatrix4* self, PyObject*)
{
	self->matrix4.Zero();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix4Identity(DCMatrix4* self, PyObject*)
{
	self->matrix4.Identity();
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "isIdentity", (PyCFunction)&DCMatrix4IsIdentity, METH_NOARGS },
	{ "isDiagonal", (PyCFunction)&DCMatrix4IsDiagonal, METH_NOARGS },
	{ "determinant", (PyCFunction)&DCMatrix4Determinant, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCMatrix4Inverse, METH_NOARGS },
	{ "transpose", (PyCFunction)&DCMatrix4Transpose, METH_NOARGS },
	{ "zero", (PyCFunction)&DCMatrix4Zero, METH_NOARGS },
	{ "identity", (PyCFunction)&DCMatrix4Identity, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCMatrix4GetAttr(DCMatrix4* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->matrix4);
	return PyFloat_FromDouble(*attr);
}

static int DCMatrix4SetAttr(DCMatrix4* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->matrix4);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCMatrix4RowTuple(DCMatrix4* self, void* closure)
{
	uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
	return Py_BuildValue("ffff",
		self->matrix4.m[idx][0], self->matrix4.m[idx][1],
		self->matrix4.m[idx][2], self->matrix4.m[idx][3]);
}

static int DCMatrix4SetRowTuple(DCMatrix4* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector4 vec;
	if (DCVector4Converter(value, &vec))
	{
		uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
		self->matrix4.m[idx][0] = vec.x;
		self->matrix4.m[idx][1] = vec.y;
		self->matrix4.m[idx][2] = vec.z;
		self->matrix4.m[idx][3] = vec.w;
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Vector4 or four floating point numbers.");
	return -1;
}

static PyObject* DCMatrix4ColumnTuple(DCMatrix4* self, void* closure)
{
	uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
	return Py_BuildValue("ffff",
		self->matrix4.m[0][idx], self->matrix4.m[1][idx],
		self->matrix4.m[2][idx], self->matrix4.m[3][idx]);
}

static int DCMatrix4SetColumnTuple(DCMatrix4* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector4 vec;
	if (DCVector4Converter(value, &vec))
	{
		uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
		self->matrix4.m[0][idx] = vec.x;
		self->matrix4.m[1][idx] = vec.y;
		self->matrix4.m[2][idx] = vec.z;
		self->matrix4.m[3][idx] = vec.w;
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Vector4 or four floating point numbers.");
	return -1;
}

static PyObject* DCMatrix4Tuple(DCMatrix4* self, void*)
{
	return Py_BuildValue("ffffffffffffffff",
		self->matrix4.m[0][0], self->matrix4.m[0][1], self->matrix4.m[0][2], self->matrix4.m[0][3],
		self->matrix4.m[1][0], self->matrix4.m[1][1], self->matrix4.m[1][2], self->matrix4.m[1][3],
		self->matrix4.m[2][0], self->matrix4.m[2][1], self->matrix4.m[2][2], self->matrix4.m[2][3],
		self->matrix4.m[3][0], self->matrix4.m[3][1], self->matrix4.m[3][2], self->matrix4.m[3][3]);
}

static int DCMatrix4SetTuple(DCMatrix4* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffffffffffffffff",
		&self->matrix4.m[0][0], &self->matrix4.m[0][1], &self->matrix4.m[0][2], &self->matrix4.m[0][3],
		&self->matrix4.m[1][0], &self->matrix4.m[1][1], &self->matrix4.m[1][2], &self->matrix4.m[1][3],
		&self->matrix4.m[2][0], &self->matrix4.m[2][1], &self->matrix4.m[2][2], &self->matrix4.m[2][3],
		&self->matrix4.m[3][0], &self->matrix4.m[3][1], &self->matrix4.m[3][2], &self->matrix4.m[3][3]))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of nine floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "m11", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _11)) },
	{ "m12", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _12)) },
	{ "m13", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _13)) },
	{ "m14", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _14)) },
	{ "m21", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _21)) },
	{ "m22", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _22)) },
	{ "m23", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _23)) },
	{ "m24", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _24)) },
	{ "m31", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _31)) },
	{ "m32", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _32)) },
	{ "m33", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _33)) },
	{ "m34", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _34)) },
	{ "m41", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _41)) },
	{ "m42", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _42)) },
	{ "m43", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _43)) },
	{ "m44", (getter)&DCMatrix4GetAttr, (setter)&DCMatrix4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix4, _44)) },
	{ "row1", (getter)&DCMatrix4RowTuple, (setter)&DCMatrix4SetRowTuple, 0, reinterpret_cast<void*>(0) },
	{ "row2", (getter)&DCMatrix4RowTuple, (setter)&DCMatrix4SetRowTuple, 0, reinterpret_cast<void*>(1) },
	{ "row3", (getter)&DCMatrix4RowTuple, (setter)&DCMatrix4SetRowTuple, 0, reinterpret_cast<void*>(2) },
	{ "row4", (getter)&DCMatrix4RowTuple, (setter)&DCMatrix4SetRowTuple, 0, reinterpret_cast<void*>(3) },
	{ "column1", (getter)&DCMatrix4ColumnTuple, (setter)&DCMatrix4SetColumnTuple, 0, reinterpret_cast<void*>(0) },
	{ "column2", (getter)&DCMatrix4ColumnTuple, (setter)&DCMatrix4SetColumnTuple, 0, reinterpret_cast<void*>(1) },
	{ "column3", (getter)&DCMatrix4ColumnTuple, (setter)&DCMatrix4SetColumnTuple, 0, reinterpret_cast<void*>(2) },
	{ "column4", (getter)&DCMatrix4ColumnTuple, (setter)&DCMatrix4SetColumnTuple, 0, reinterpret_cast<void*>(3) },
	{ "tuple", (getter)&DCMatrix4Tuple, (setter)&DCMatrix4SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCMatrix4UnaryOperand(PyObject* obj, bool(*fn)(const DKMatrix4&, DKMatrix4&))
{
	DKMatrix4* mat = DCMatrix4ToObject(obj);
	if (mat)
	{
		DKMatrix4 res;
		if (fn(*mat, res))
			return DCMatrix4FromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCMatrix4BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKMatrix4&, const DKMatrix4&, DKMatrix4&))
{
	DKMatrix4* p = DCMatrix4ToObject(lhs);
	DKMatrix4 mat;
	if (p && DCMatrix4Converter(rhs, &mat))
	{
		DKMatrix4 res;
		if (fn(*p, mat, res))
			return DCMatrix4FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCMatrix4BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKMatrix4&, double, DKMatrix4&))
{
	DKMatrix4* mat = DCMatrix4ToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKMatrix4 res;
		if (fn(*mat, d, res))
			return DCMatrix4FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCMatrix4Add(PyObject* lhs, PyObject* rhs)
{
	return DCMatrix4BinaryOperand(lhs, rhs, [](const DKMatrix4& m1, const DKMatrix4& m2, DKMatrix4& r)
	{
		r = m1 + m2;
		return true;
	});
}

static PyObject* DCMatrix4Subtract(PyObject* lhs, PyObject* rhs)
{
	return DCMatrix4BinaryOperand(lhs, rhs, [](const DKMatrix4& m1, const DKMatrix4& m2, DKMatrix4& r)
	{
		r = m1 - m2;
		return true;
	});
}

static PyObject* DCMatrix4Multiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCMatrix4TypeObject()))
		return DCMatrix4BinaryOperand(lhs, rhs, [](const DKMatrix4& m1, const DKMatrix4& m2, DKMatrix4& r)
	{
		r = m1 * m2;
		return true;
	});
	return DCMatrix4BinaryOperand(lhs, rhs, [](const DKMatrix4& m, double d, DKMatrix4& r)
	{
		r = m * d;
		return true;
	});
}

static PyObject* DCMatrix4Negative(PyObject* obj)
{
	return DCMatrix4UnaryOperand(obj, [](const DKMatrix4& m, DKMatrix4& r)
	{
		r = m * -1.0f;
		return true;
	});
}

static PyObject* DCMatrix4Positive(PyObject* obj)
{
	return DCMatrix4UnaryOperand(obj, [](const DKMatrix4& m, DKMatrix4& r)
	{
		r = m;
		return true;
	});
}

static PyObject* DCMatrix4Absolute(PyObject* obj)
{
	return DCMatrix4UnaryOperand(obj, [](const DKMatrix4& m, DKMatrix4& r)
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

static PyObject* DCMatrix4FloorDivide(PyObject *lhs, PyObject *rhs)
{
	return DCMatrix4BinaryOperand(lhs, rhs, [](const DKMatrix4& m, double d, DKMatrix4& r)
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

static PyObject* DCMatrix4TrueDivide(PyObject *lhs, PyObject *rhs)
{
	return DCMatrix4BinaryOperand(lhs, rhs, [](const DKMatrix4& m, double d, DKMatrix4& r)
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
	(binaryfunc)&DCMatrix4Add,						/* nb_add */
	(binaryfunc)&DCMatrix4Subtract,					/* nb_subtract */
	(binaryfunc)&DCMatrix4Multiply,					/* nb_multiply */
	0,												/* nb_remainder */
	0,												/* nb_divmod */
	0,												/* nb_power */
	(unaryfunc)&DCMatrix4Negative,					/* nb_negative */
	(unaryfunc)&DCMatrix4Positive,					/* nb_positive */
	(unaryfunc)&DCMatrix4Absolute,					/* nb_absolute */
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
	(binaryfunc)&DCMatrix4FloorDivide,				/* nb_floor_divide */
	(binaryfunc)&DCMatrix4TrueDivide,				/* nb_true_divide */
	0,												/* nb_inplace_floor_divide */
	0,												/* nb_inplace_true_divide */
	0												/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Matrix4",					/* tp_name */
	sizeof(DCMatrix4),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCMatrix4Dealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCMatrix4Repr,						/* tp_repr */
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
	(richcmpfunc)&DCMatrix4RichCompare,				/* tp_richcompare */
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
	(initproc)&DCMatrix4Init,						/* tp_init */
	0,												/* tp_alloc */
	&DCMatrix4New,									/* tp_new */
};

PyTypeObject* DCMatrix4TypeObject(void)
{
	return &objectType;
}

PyObject* DCMatrix4FromObject(DKMatrix4* matrix4)
{
	if (matrix4)
	{
		PyObject* args = Py_BuildValue("ffffffffffffffff",
			matrix4->m[0][0], matrix4->m[0][1], matrix4->m[0][2], matrix4->m[0][3],
			matrix4->m[1][0], matrix4->m[1][1], matrix4->m[1][2], matrix4->m[1][3],
			matrix4->m[2][0], matrix4->m[2][1], matrix4->m[2][2], matrix4->m[2][3],
			matrix4->m[3][0], matrix4->m[3][1], matrix4->m[3][2], matrix4->m[3][3]);
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

DKMatrix4* DCMatrix4ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCMatrix4*)obj)->matrix4;
	}
	return NULL;
}

int DCMatrix4Converter(PyObject* obj, DKMatrix4* p)
{
	DKMatrix4* tmp = DCMatrix4ToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		Py_ssize_t numArgs = PyTuple_GET_SIZE(obj);
		if (numArgs == 16)
		{
			if (PyArg_ParseTuple(obj, "ffffffffffffffff",
				&p->m[0][0], &p->m[0][1], &p->m[0][2], &p->m[0][3],
				&p->m[1][0], &p->m[1][1], &p->m[1][2], &p->m[1][3],
				&p->m[2][0], &p->m[2][1], &p->m[2][2], &p->m[2][3],
				&p->m[3][0], &p->m[3][1], &p->m[3][2], &p->m[3][3]))
				return true;
		}
		else if (numArgs == 4)
		{
			DKVector4 row[4];
			if (PyArg_ParseTuple(obj, "O&O&O&O&",
				&DCVector4Converter, &row[0],
				&DCVector4Converter, &row[1],
				&DCVector4Converter, &row[2],
				&DCVector4Converter, &row[3]))
			{
				*p = DKMatrix4(row[0], row[1], row[2], row[3]);
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Matrix4 object or sixteen floats tuple.");
	return false;
}
