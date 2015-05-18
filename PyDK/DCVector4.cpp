#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

struct DCVector4
{
	PyObject_HEAD
	DKVector4 vector4;
};

static PyObject* DCVector4New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCVector4* self = (DCVector4*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->vector4) DKVector4();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCVector4Init(DCVector4 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCVector4Converter(args, &self->vector4))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCVector4Converter, &self->vector4))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->vector4 = DKVector4(0, 0, 0, 0);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be four floating points (x,y,z,w) or empty");
	return -1;
}

static void DCVector4Dealloc(DCVector4* self)
{
	self->vector4.~DKVector4();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCVector4Repr(DCVector4* self)
{
	DKString str = DKString::Format("<%s object (x:%.3f, y:%.3f, z:%.3f, w:%.3f)>",
		Py_TYPE(self)->tp_name, self->vector4.x, self->vector4.y, self->vector4.z, self->vector4.w);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCVector4RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKVector4* vec1 = DCVector4ToObject(obj1);
	DKVector4* vec2 = DCVector4ToObject(obj2);
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

static PyObject* DCVector4Length(DCVector4* self, PyObject*)
{
	return PyFloat_FromDouble(self->vector4.Length());
}

static PyObject* DCVector4LengthSq(DCVector4* self, PyObject*)
{
	return PyFloat_FromDouble(self->vector4.LengthSq());
}

static PyObject* DCVector4Normalize(DCVector4* self)
{
	self->vector4.Normalize();
	Py_RETURN_NONE;
}

static PyObject* DCVector4Dot(DCVector4* self, PyObject* args)
{
	DKVector4 vec;
	if (PyArg_ParseTuple(args, "O&", &DCVector4Converter, &vec))
	{
		float d = DKVector4::Dot(self->vector4, vec);
		return PyFloat_FromDouble(d);
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Vector4 object.");
	return NULL;
}

static PyObject* DCVector4Cross(DCVector4* self, PyObject* args)
{
	DKVector4 vec1;
	DKVector4 vec2;
	if (PyArg_ParseTuple(args, "O&O&", &DCVector4Converter, &vec1, &DCVector4Converter, &vec2))
	{
		DKVector4 vec = DKVector4::Cross(self->vector4, vec1, vec2);
		return DCVector4FromObject(&vec);
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be two Vector4 objects.");
	return NULL;
}

static PyObject* DCVector4Transform(DCVector4* self, PyObject* args)
{
	PyObject* obj;
	if (PyArg_ParseTuple(args, "O", &obj))
	{
		DKMatrix4* mat = DCMatrix4ToObject(obj);
		if (mat)
		{
			self->vector4.Transform(*mat);
			Py_RETURN_NONE;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Matrix4 objects.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "length", (PyCFunction)&DCVector4Length, METH_NOARGS },
	{ "lengthSq", (PyCFunction)&DCVector4LengthSq, METH_NOARGS },
	{ "normalize", (PyCFunction)&DCVector4Normalize, METH_NOARGS },
	{ "dot", (PyCFunction)&DCVector4Dot, METH_VARARGS, "dot-product" },
	{ "cross", (PyCFunction)&DCVector4Cross, METH_VARARGS, "cross-product" },
	{ "transform", (PyCFunction)&DCVector4Transform, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCVector4GetAttr(DCVector4* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->vector4);
	return PyFloat_FromDouble(*attr);
}

static int DCVector4SetAttr(DCVector4* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->vector4);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCVector4Tuple(DCVector4* self, void*)
{
	return Py_BuildValue("ffff", self->vector4.x, self->vector4.y, self->vector4.z, self->vector4.w);
}

static int DCVector4SetTuple(DCVector4* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffff",
		&self->vector4.x, &self->vector4.y, &self->vector4.z, &self->vector4.w))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of four floating point numbers (x, y, z, w).");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "x", (getter)&DCVector4GetAttr, (setter)&DCVector4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector4, x)) },
	{ "y", (getter)&DCVector4GetAttr, (setter)&DCVector4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector4, y)) },
	{ "z", (getter)&DCVector4GetAttr, (setter)&DCVector4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector4, z)) },
	{ "w", (getter)&DCVector4GetAttr, (setter)&DCVector4SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector4, w)) },
	{ "tuple", (getter)&DCVector4Tuple, (setter)&DCVector4SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCVector4UnaryOperand(PyObject* obj, bool(*fn)(const DKVector4&, DKVector4&))
{
	DKVector4* v = DCVector4ToObject(obj);
	if (v)
	{
		DKVector4 res;
		if (fn(*v, res))
			return DCVector4FromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector4BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector4&, const DKVector4&, DKVector4&))
{
	DKVector4* p = DCVector4ToObject(lhs);
	DKVector4 v;
	if (p && DCVector4Converter(rhs, &v))
	{
		DKVector4 res;
		if (fn(*p, v, res))
			return DCVector4FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector4BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector4&, const DKMatrix4&, DKVector4&))
{
	DKVector4* p = DCVector4ToObject(lhs);
	DKMatrix4 m;
	if (p && DCMatrix4Converter(rhs, &m))
	{
		DKVector4 res;
		if (fn(*p, m, res))
			return DCVector4FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector4BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector4&, double, DKVector4&))
{
	DKVector4* vec = DCVector4ToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKVector4 res;
		if (fn(*vec, d, res))
			return DCVector4FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCVector4Add(PyObject* lhs, PyObject* rhs)
{
	return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v1, const DKVector4& v2, DKVector4& r)
	{
		r = v1 + v2;
		return true;
	});
}

static PyObject* DCVector4Subtract(PyObject* lhs, PyObject* rhs)
{
	return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v1, const DKVector4& v2, DKVector4& r)
	{
		r = v1 - v2;
		return true;
	});
}

static PyObject* DCVector4Multiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector4TypeObject()))
		return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v1, const DKVector4& v2, DKVector4& r)
	{
		r = v1 * v2;
		return true;
	});
	if (PyObject_TypeCheck(rhs, DCMatrix4TypeObject()))
		return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v, const DKMatrix4& m, DKVector4& r)
	{
		r = v * m;
		return true;
	});
	return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v, double d, DKVector4& r)
	{
		r = v * d;
		return true;
	});
}

static PyObject* DCVector4Negative(PyObject* obj)
{
	return DCVector4UnaryOperand(obj, [](const DKVector4& v, DKVector4& r)
	{
		r = DKVector4(-v.x, -v.y, -v.z, -v.w);
		return true;
	});
}

static PyObject* DCVector4Positive(PyObject* obj)
{
	return DCVector4UnaryOperand(obj, [](const DKVector4& v, DKVector4& r)
	{
		r = v;
		return true;
	});
}

static PyObject* DCVector4Absolute(PyObject* obj)
{
	return DCVector4UnaryOperand(obj, [](const DKVector4& v, DKVector4& r)
	{
		float x = v.x < 0 ? -v.x : v.x;
		float y = v.y < 0 ? -v.y : v.y;
		float z = v.z < 0 ? -v.z : v.z;
		float w = v.w < 0 ? -v.w : v.w;
		r = DKVector4(x, y, z, w);
		return true;
	});
}

static PyObject* DCVector4FloorDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector4TypeObject()))
		return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v1, const DKVector4& v2, DKVector4& r)
	{
		if (v2.x != 0.0f && v2.y != 0.0f && v2.z != 0.0f && v2.w != 0.0f)
		{
			r = v1 / v2;
			r.x = floor(r.x);
			r.y = floor(r.y);
			r.z = floor(r.z);
			r.w = floor(r.w);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v, double d, DKVector4& r)
	{
		if (d != 0.0)
		{
			r = v / d;
			r.x = floor(r.x);
			r.y = floor(r.y);
			r.z = floor(r.z);
			r.w = floor(r.w);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyObject* DCVector4TrueDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector4TypeObject()))
		return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v1, const DKVector4& v2, DKVector4& r)
	{
		if (v2.x != 0.0f && v2.y != 0.0f && v2.z != 0.0f && v2.w != 0.0f)
		{
			r = v1 / v2;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	return DCVector4BinaryOperand(lhs, rhs, [](const DKVector4& v, double d, DKVector4& r)
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
	(binaryfunc)&DCVector4Add,			/* nb_add */
	(binaryfunc)&DCVector4Subtract,		/* nb_subtract */
	(binaryfunc)&DCVector4Multiply,		/* nb_multiply */
	0,									/* nb_remainder */
	0,									/* nb_divmod */
	0,									/* nb_power */
	(unaryfunc)&DCVector4Negative,		/* nb_negative */
	(unaryfunc)&DCVector4Positive,		/* nb_positive */
	(unaryfunc)&DCVector4Absolute,		/* nb_absolute */
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
	(binaryfunc)&DCVector4FloorDivide,	/* nb_floor_divide */
	(binaryfunc)&DCVector4TrueDivide,	/* nb_true_divide */
	0,									/* nb_inplace_floor_divide */
	0,									/* nb_inplace_true_divide */
	0									/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Vector4",					/* tp_name */
	sizeof(DCVector4),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCVector4Dealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCVector4Repr,						/* tp_repr */
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
	(richcmpfunc)&DCVector4RichCompare,				/* tp_richcompare */
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
	(initproc)&DCVector4Init,						/* tp_init */
	0,												/* tp_alloc */
	&DCVector4New,									/* tp_new */
};

PyTypeObject* DCVector4TypeObject(void)
{
	return &objectType;
}

PyObject* DCVector4FromObject(DKVector4* vector4)
{
	if (vector4)
	{
		PyObject* args = Py_BuildValue("ffff", vector4->x, vector4->y, vector4->z, vector4->w);
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

DKVector4* DCVector4ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCVector4*)obj)->vector4;
	}
	return NULL;
}

int DCVector4Converter(PyObject* obj, DKVector4* p)
{
	DKVector4* tmp = DCVector4ToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		if (PyArg_ParseTuple(obj, "ffff", &p->x, &p->y, &p->z, &p->w))
			return true;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Vector4 object or four floats tuple.");
	return false;
}
