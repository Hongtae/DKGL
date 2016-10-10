#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKGL;
using namespace DKGL;

struct DCVector3
{
	PyObject_HEAD
	DKVector3 vector3;
};

static PyObject* DCVector3New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCVector3* self = (DCVector3*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->vector3) DKVector3();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCVector3Init(DCVector3 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCVector3Converter(args, &self->vector3))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCVector3Converter, &self->vector3))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->vector3 = DKVector3(0, 0, 0);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be three floating points (x,y,z) or empty");
	return -1;
}

static void DCVector3Dealloc(DCVector3* self)
{
	self->vector3.~DKVector3();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCVector3Repr(DCVector3* self)
{
	DKString str = DKString::Format("<%s object (x:%.3f, y:%.3f, z:%.3f)>",
		Py_TYPE(self)->tp_name, self->vector3.x, self->vector3.y, self->vector3.z);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCVector3RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKVector3* vec1 = DCVector3ToObject(obj1);
	DKVector3* vec2 = DCVector3ToObject(obj2);
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

static PyObject* DCVector3Length(DCVector3* self, PyObject*)
{
	return PyFloat_FromDouble(self->vector3.Length());
}

static PyObject* DCVector3LengthSq(DCVector3* self, PyObject*)
{
	return PyFloat_FromDouble(self->vector3.LengthSq());
}

static PyObject* DCVector3Normalize(DCVector3* self)
{
	self->vector3.Normalize();
	Py_RETURN_NONE;
}

static PyObject* DCVector3Dot(DCVector3* self, PyObject* args)
{
	DKVector3 vec;
	if (PyArg_ParseTuple(args, "O&", &DCVector3Converter, &vec))
	{
		float d = DKVector3::Dot(self->vector3, vec);
		return PyFloat_FromDouble(d);
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
	return NULL;
}

static PyObject* DCVector3Cross(DCVector3* self, PyObject* args)
{
	DKVector3 vec;
	if (PyArg_ParseTuple(args, "O&", &DCVector3Converter, &vec))
	{
		DKVector3 c = DKVector3::Cross(self->vector3, vec);
		return DCVector3FromObject(&c);
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Vector3 object.");
	return NULL;
}

static PyObject* DCVector3Transform(DCVector3* self, PyObject* args)
{
	PyObject* obj;
	if (PyArg_ParseTuple(args, "O", &obj))
	{
		DKMatrix3* mat3 = DCMatrix3ToObject(obj);
		if (mat3)
		{
			self->vector3.Transform(*mat3);
			Py_RETURN_NONE;
		}
		DKMatrix4* mat4 = DCMatrix4ToObject(obj);
		if (mat4)
		{
			self->vector3.Transform(*mat4);
			Py_RETURN_NONE;
		}
		DKQuaternion* quat = DCQuaternionToObject(obj);
		if (quat)
		{
			self->vector3.Rotate(*quat);
			Py_RETURN_NONE;
		}
		DKNSTransform* nst = DCNSTransformToObject(obj);
		if (nst)
		{
			self->vector3 *= (*nst);
			Py_RETURN_NONE;
		}
		DKUSTransform* ust = DCUSTransformToObject(obj);
		if (ust)
		{
			self->vector3 *= (*ust);
			Py_RETURN_NONE;
		}
		DKTransformUnit* tu = DCTransformUnitToObject(obj);
		if (tu)
		{
			self->vector3 *= (*tu);
			Py_RETURN_NONE;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be one of following: Matrix3, Matrix4, Quaternion, Transform object.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "length", (PyCFunction)&DCVector3Length, METH_NOARGS },
	{ "lengthSq", (PyCFunction)&DCVector3LengthSq, METH_NOARGS },
	{ "normalize", (PyCFunction)&DCVector3Normalize, METH_NOARGS },
	{ "dot", (PyCFunction)&DCVector3Dot, METH_VARARGS, "dot-product" },
	{ "cross", (PyCFunction)&DCVector3Cross, METH_VARARGS, "cross-product" },
	{ "transform", (PyCFunction)&DCVector3Transform, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCVector3GetAttr(DCVector3* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->vector3);
	return PyFloat_FromDouble(*attr);
}

static int DCVector3SetAttr(DCVector3* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->vector3);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCVector3Tuple(DCVector3* self, void*)
{
	return Py_BuildValue("fff", self->vector3.x, self->vector3.y, self->vector3.z);
}

static int DCVector3SetTuple(DCVector3* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "fff",
		&self->vector3.x, &self->vector3.y, &self->vector3.z))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of three floating point numbers (x, y, z).");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "x", (getter)&DCVector3GetAttr, (setter)&DCVector3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector3, x)) },
	{ "y", (getter)&DCVector3GetAttr, (setter)&DCVector3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector3, y)) },
	{ "z", (getter)&DCVector3GetAttr, (setter)&DCVector3SetAttr, 0, reinterpret_cast<void*>(offsetof(DKVector3, z)) },
	{ "tuple", (getter)&DCVector3Tuple, (setter)&DCVector3SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCVector3UnaryOperand(PyObject* obj, bool(*fn)(const DKVector3&, DKVector3&))
{
	DKVector3* v = DCVector3ToObject(obj);
	if (v)
	{
		DKVector3 res;
		if (fn(*v, res))
			return DCVector3FromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector3BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector3&, const DKVector3&, DKVector3&))
{
	DKVector3* p = DCVector3ToObject(lhs);
	DKVector3 v;
	if (p && DCVector3Converter(rhs, &v))
	{
		DKVector3 res;
		if (fn(*p, v, res))
			return DCVector3FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector3BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector3&, const DKMatrix3&, DKVector3&))
{
	DKVector3* p = DCVector3ToObject(lhs);
	DKMatrix3 m;
	if (p && DCMatrix3Converter(rhs, &m))
	{
		DKVector3 res;
		if (fn(*p, m, res))
			return DCVector3FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCVector3BinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKVector3&, double, DKVector3&))
{
	DKVector3* vec = DCVector3ToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKVector3 res;
		if (fn(*vec, d, res))
			return DCVector3FromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCVector3Add(PyObject* lhs, PyObject* rhs)
{
	return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v1, const DKVector3& v2, DKVector3& r)
	{
		r = v1 + v2;
		return true;
	});
}

static PyObject* DCVector3Subtract(PyObject* lhs, PyObject* rhs)
{
	return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v1, const DKVector3& v2, DKVector3& r)
	{
		r = v1 - v2;
		return true;
	});
}

static PyObject* DCVector3Multiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector3TypeObject()))
		return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v1, const DKVector3& v2, DKVector3& r)
	{
		r = v1 * v2;
		return true;
	});
	if (PyObject_TypeCheck(rhs, DCMatrix3TypeObject()))
		return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v, const DKMatrix3& m, DKVector3& r)
	{
		r = v * m;
		return true;
	});
	return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v, double d, DKVector3& r)
	{
		r = v * d;
		return true;
	});
}

static PyObject* DCVector3Negative(PyObject* obj)
{
	return DCVector3UnaryOperand(obj, [](const DKVector3& v, DKVector3& r)
	{
		r = DKVector3(-v.x, -v.y, -v.z);
		return true;
	});
}

static PyObject* DCVector3Positive(PyObject* obj)
{
	return DCVector3UnaryOperand(obj, [](const DKVector3& v, DKVector3& r)
	{
		r = v;
		return true;
	});
}

static PyObject* DCVector3Absolute(PyObject* obj)
{
	return DCVector3UnaryOperand(obj, [](const DKVector3& v, DKVector3& r)
	{
		float x = v.x < 0 ? -v.x : v.x;
		float y = v.y < 0 ? -v.y : v.y;
		float z = v.z < 0 ? -v.z : v.z;
		r = DKVector3(x, y, z);
		return true;
	});
}

static PyObject* DCVector3FloorDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector3TypeObject()))
		return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v1, const DKVector3& v2, DKVector3& r)
	{
		if (v2.x != 0.0f && v2.y != 0.0f && v2.z != 0.0f)
		{
			r = v1 / v2;
			r.x = floor(r.x);
			r.y = floor(r.y);
			r.z = floor(r.z);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v, double d, DKVector3& r)
	{
		if (d != 0.0)
		{
			r = v / d;
			r.x = floor(r.x);
			r.y = floor(r.y);
			r.z = floor(r.z);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyObject* DCVector3TrueDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector3TypeObject()))
		return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v1, const DKVector3& v2, DKVector3& r)
	{
		if (v2.x != 0.0f && v2.y != 0.0f && v2.z != 0.0f)
		{
			r = v1 / v2;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	return DCVector3BinaryOperand(lhs, rhs, [](const DKVector3& v, double d, DKVector3& r)
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
	(binaryfunc)&DCVector3Add,			/* nb_add */
	(binaryfunc)&DCVector3Subtract,		/* nb_subtract */
	(binaryfunc)&DCVector3Multiply,		/* nb_multiply */
	0,									/* nb_remainder */
	0,									/* nb_divmod */
	0,									/* nb_power */
	(unaryfunc)&DCVector3Negative,		/* nb_negative */
	(unaryfunc)&DCVector3Positive,		/* nb_positive */
	(unaryfunc)&DCVector3Absolute,		/* nb_absolute */
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
	(binaryfunc)&DCVector3FloorDivide,	/* nb_floor_divide */
	(binaryfunc)&DCVector3TrueDivide,	/* nb_true_divide */
	0,									/* nb_inplace_floor_divide */
	0,									/* nb_inplace_true_divide */
	0									/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Vector3",					/* tp_name */
	sizeof(DCVector3),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCVector3Dealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCVector3Repr,						/* tp_repr */
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
	(richcmpfunc)&DCVector3RichCompare,				/* tp_richcompare */
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
	(initproc)&DCVector3Init,						/* tp_init */
	0,												/* tp_alloc */
	&DCVector3New,									/* tp_new */
};

PyTypeObject* DCVector3TypeObject(void)
{
	return &objectType;
}

PyObject* DCVector3FromObject(DKVector3* vector3)
{
	if (vector3)
	{
		PyObject* args = Py_BuildValue("fff", vector3->x, vector3->y, vector3->z);
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

DKVector3* DCVector3ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCVector3*)obj)->vector3;
	}
	return NULL;
}

int DCVector3Converter(PyObject* obj, DKVector3* p)
{
	DKVector3* tmp = DCVector3ToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		if (PyArg_ParseTuple(obj, "fff", &p->x, &p->y, &p->z))
			return true;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Vector3 object or three floats tuple.");
	return false;
}
