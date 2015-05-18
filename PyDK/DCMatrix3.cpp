#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCMatrix3
{
	PyObject_HEAD
	DKMatrix3 matrix3;
};

static PyObject* DCMatrix3New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCMatrix3* self = (DCMatrix3*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->matrix3) DKMatrix3();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCMatrix3Init(DCMatrix3 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	// (vec3, vec3, vec3), (float x 9)
	if (numArgs > 1 && DCMatrix3Converter(args, &self->matrix3))
	{
		return 0;
	}
	// matrix3
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCMatrix3Converter, &self->matrix3))
	{
		return 0;
	}
	// default
	else if (numArgs == 0)
	{
		self->matrix3.Identity();
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be three Vector3 or nine floating points or empty.");
	return -1;
}

static void DCMatrix3Dealloc(DCMatrix3* self)
{
	self->matrix3.~DKMatrix3();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCMatrix3Repr(DCMatrix3* self)
{
	DKString str = DKString::Format(
		"<%s object\n"
		"  (%.3f, %.3f, %.3f)\n"
		"  (%.3f, %.3f, %.3f)\n"
		"  (%.3f, %.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		self->matrix3.m[0][0], self->matrix3.m[0][1], self->matrix3.m[0][2],
		self->matrix3.m[1][0], self->matrix3.m[1][1], self->matrix3.m[1][2],
		self->matrix3.m[2][0], self->matrix3.m[2][1], self->matrix3.m[2][2]);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCMatrix3RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKMatrix3* m1 = DCMatrix3ToObject(obj1);
	DKMatrix3* m2 = DCMatrix3ToObject(obj2);
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

static PyObject* DCMatrix3IsIdentity(DCMatrix3* self)
{
	if (self->matrix3.IsIdentity()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCMatrix3IsDiagonal(DCMatrix3* self)
{
	if (self->matrix3.IsDiagonal()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCMatrix3Determinant(DCMatrix3* self)
{
	return PyFloat_FromDouble(self->matrix3.Determinant());
}

static PyObject* DCMatrix3Inverse(DCMatrix3* self)
{
	self->matrix3.Inverse();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix3Transpose(DCMatrix3* self)
{
	self->matrix3.Transpose();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix3Zero(DCMatrix3* self)
{
	self->matrix3.Zero();
	Py_RETURN_NONE;
}

static PyObject* DCMatrix3Identity(DCMatrix3* self)
{
	self->matrix3.Identity();
	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "isIdentity", (PyCFunction)&DCMatrix3IsIdentity, METH_NOARGS },
	{ "isDiagonal", (PyCFunction)&DCMatrix3IsDiagonal, METH_NOARGS },
	{ "determinant", (PyCFunction)&DCMatrix3Determinant, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCMatrix3Inverse, METH_NOARGS },
	{ "transpose", (PyCFunction)&DCMatrix3Transpose, METH_NOARGS },
	{ "zero", (PyCFunction)&DCMatrix3Zero, METH_NOARGS },
	{ "identity", (PyCFunction)&DCMatrix3Identity, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCMatrix3GetAttr(DCMatrix3* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->matrix3);
	return PyFloat_FromDouble(*attr);
}

static int DCMatrix3SetAttr(DCMatrix3* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->matrix3);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCMatrix3RowTuple(DCMatrix3* self, void* closure)
{
	uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
	return Py_BuildValue("fff", self->matrix3.m[idx][0], self->matrix3.m[idx][1], self->matrix3.m[idx][2]);
}

static int DCMatrix3SetRowTuple(DCMatrix3* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3 vec;
	if (DCVector3Converter(value, &vec))
	{
		uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
		self->matrix3.m[idx][0] = vec.x;
		self->matrix3.m[idx][1] = vec.y;
		self->matrix3.m[idx][2] = vec.z;
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Vector3 or three floating point numbers.");
	return -1;
}

static PyObject* DCMatrix3ColumnTuple(DCMatrix3* self, void* closure)
{
	uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
	return Py_BuildValue("fff", self->matrix3.m[0][idx], self->matrix3.m[1][idx], self->matrix3.m[2][idx]);
}

static int DCMatrix3SetColumnTuple(DCMatrix3* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3 vec;
	if (DCVector3Converter(value, &vec))
	{
		uintptr_t idx = reinterpret_cast<uintptr_t>(closure);
		self->matrix3.m[0][idx] = vec.x;
		self->matrix3.m[1][idx] = vec.y;
		self->matrix3.m[2][idx] = vec.z;
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Vector3 or three floating point numbers.");
	return -1;
}

static PyObject* DCMatrix3Tuple(DCMatrix3* self, void*)
{
	return Py_BuildValue("fffffffff",
		self->matrix3.m[0][0], self->matrix3.m[0][1], self->matrix3.m[0][2],
		self->matrix3.m[1][0], self->matrix3.m[1][1], self->matrix3.m[1][2],
		self->matrix3.m[2][0], self->matrix3.m[2][1], self->matrix3.m[2][2]);
}

static int DCMatrix3SetTuple(DCMatrix3* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "fffffffff",
		&self->matrix3.m[0][0], &self->matrix3.m[0][1], &self->matrix3.m[0][2],
		&self->matrix3.m[1][0], &self->matrix3.m[1][1], &self->matrix3.m[1][2],
		&self->matrix3.m[2][0], &self->matrix3.m[2][1], &self->matrix3.m[2][2]))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of nine floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "m11", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _11)) },
	{ "m12", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _12)) },
	{ "m13", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _13)) },
	{ "m21", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _21)) },
	{ "m22", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _22)) },
	{ "m23", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _23)) },
	{ "m31", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _31)) },
	{ "m32", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _32)) },
	{ "m33", (getter)&DCMatrix3GetAttr, (setter)&DCMatrix3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKMatrix3, _33)) },
	{ "row1", (getter)&DCMatrix3RowTuple, (setter)&DCMatrix3SetRowTuple, 0, reinterpret_cast<void*>(0) },
	{ "row2", (getter)&DCMatrix3RowTuple, (setter)&DCMatrix3SetRowTuple, 0, reinterpret_cast<void*>(1) },
	{ "row3", (getter)&DCMatrix3RowTuple, (setter)&DCMatrix3SetRowTuple, 0, reinterpret_cast<void*>(2) },
	{ "column1", (getter)&DCMatrix3ColumnTuple, (setter)&DCMatrix3SetColumnTuple, 0, reinterpret_cast<void*>(0) },
	{ "column2", (getter)&DCMatrix3ColumnTuple, (setter)&DCMatrix3SetColumnTuple, 0, reinterpret_cast<void*>(1) },
	{ "column3", (getter)&DCMatrix3ColumnTuple, (setter)&DCMatrix3SetColumnTuple, 0, reinterpret_cast<void*>(2) },
	{ "tuple", (getter)&DCMatrix3Tuple, (setter)&DCMatrix3SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCMatrix3UnaryOperand(PyObject* obj, bool(*fn)(const DKMatrix3&, DKMatrix3&))
{
	DKMatrix3* mat = DCMatrix3ToObject(obj);
	if (mat)
	{
		DKMatrix3 res;
		if (fn(*mat, res))
			return DCMatrix3FromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCMatrix3BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKMatrix3&, const DKMatrix3&, DKMatrix3&))
{
	DKMatrix3* p = DCMatrix3ToObject(lhs);
	DKMatrix3 mat;
	if (p && DCMatrix3Converter(rhs, &mat))
	{
		DKMatrix3 res;
		if (fn(*p, mat, res))
			return DCMatrix3FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCMatrix3BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKMatrix3&, double, DKMatrix3&))
{
	DKMatrix3* mat = DCMatrix3ToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKMatrix3 res;
		if (fn(*mat, d, res))
			return DCMatrix3FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCMatrix3Add(PyObject* lhs, PyObject* rhs)
{
	return DCMatrix3BinaryOperand(lhs, rhs, [](const DKMatrix3& m1, const DKMatrix3& m2, DKMatrix3& r)
	{
		r = m1 + m2;
		return true;
	});
}

static PyObject* DCMatrix3Subtract(PyObject* lhs, PyObject* rhs)
{
	return DCMatrix3BinaryOperand(lhs, rhs, [](const DKMatrix3& m1, const DKMatrix3& m2, DKMatrix3& r)
	{
		r = m1 - m2;
		return true;
	});
}

static PyObject* DCMatrix3Multiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCMatrix3TypeObject()))
		return DCMatrix3BinaryOperand(lhs, rhs, [](const DKMatrix3& m1, const DKMatrix3& m2, DKMatrix3& r)
	{
		r = m1 * m2;
		return true;
	});
	return DCMatrix3BinaryOperand(lhs, rhs, [](const DKMatrix3& m, double d, DKMatrix3& r)
	{
		r = m * d;
		return true;
	});
}

static PyObject* DCMatrix3Negative(PyObject* obj)
{
	return DCMatrix3UnaryOperand(obj, [](const DKMatrix3& m, DKMatrix3& r)
	{
		r = m * -1.0f;
		return true;
	});
}

static PyObject* DCMatrix3Positive(PyObject* obj)
{
	return DCMatrix3UnaryOperand(obj, [](const DKMatrix3& m, DKMatrix3& r)
	{
		r = m;
		return true;
	});
}

static PyObject* DCMatrix3Absolute(PyObject* obj)
{
	return DCMatrix3UnaryOperand(obj, [](const DKMatrix3& m, DKMatrix3& r)
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

static PyObject* DCMatrix3FloorDivide(PyObject *lhs, PyObject *rhs)
{
	return DCMatrix3BinaryOperand(lhs, rhs, [](const DKMatrix3& m, double d, DKMatrix3& r)
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

static PyObject* DCMatrix3TrueDivide(PyObject *lhs, PyObject *rhs)
{
	return DCMatrix3BinaryOperand(lhs, rhs, [](const DKMatrix3& m, double d, DKMatrix3& r)
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
	(binaryfunc)&DCMatrix3Add,						/* nb_add */
	(binaryfunc)&DCMatrix3Subtract,					/* nb_subtract */
	(binaryfunc)&DCMatrix3Multiply,					/* nb_multiply */
	0,												/* nb_remainder */
	0,												/* nb_divmod */
	0,												/* nb_power */
	(unaryfunc)&DCMatrix3Negative,					/* nb_negative */
	(unaryfunc)&DCMatrix3Positive,					/* nb_positive */
	(unaryfunc)&DCMatrix3Absolute,					/* nb_absolute */
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
	(binaryfunc)&DCMatrix3FloorDivide,				/* nb_floor_divide */
	(binaryfunc)&DCMatrix3TrueDivide,				/* nb_true_divide */
	0,												/* nb_inplace_floor_divide */
	0,												/* nb_inplace_true_divide */
	0												/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Matrix3",					/* tp_name */
	sizeof(DCMatrix3),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCMatrix3Dealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCMatrix3Repr,						/* tp_repr */
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
	(richcmpfunc)&DCMatrix3RichCompare,				/* tp_richcompare */
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
	(initproc)&DCMatrix3Init,						/* tp_init */
	0,												/* tp_alloc */
	&DCMatrix3New,									/* tp_new */
};

PyTypeObject* DCMatrix3TypeObject(void)
{
	return &objectType;
}

PyObject* DCMatrix3FromObject(DKMatrix3* matrix3)
{
	if (matrix3)
	{
		PyObject* args = Py_BuildValue("fffffffff",
			matrix3->m[0][0], matrix3->m[0][1], matrix3->m[0][2],
			matrix3->m[1][0], matrix3->m[1][1], matrix3->m[1][2],
			matrix3->m[2][0], matrix3->m[2][1], matrix3->m[2][2]);
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

DKMatrix3* DCMatrix3ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCMatrix3*)obj)->matrix3;
	}
	return NULL;
}

int DCMatrix3Converter(PyObject* obj, DKMatrix3* p)
{
	DKMatrix3* tmp = DCMatrix3ToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		Py_ssize_t numArgs = PyTuple_GET_SIZE(obj);
		if (numArgs == 9)
		{
			if (PyArg_ParseTuple(obj, "fffffffff",
				&p->m[0][0], &p->m[0][1], &p->m[0][2],
				&p->m[1][0], &p->m[1][1], &p->m[1][2],
				&p->m[2][0], &p->m[2][1], &p->m[2][2]))
				return true;
		}
		else if (numArgs == 3)
		{
			DKVector3 row[3];
			if (PyArg_ParseTuple(obj, "O&O&O&",
				&DCVector3Converter, &row[0],
				&DCVector3Converter, &row[1],
				&DCVector3Converter, &row[2]))
			{
				*p = DKMatrix3(row[0], row[1], row[2]);
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Matrix3 object or nine floats tuple.");
	return false;
}
