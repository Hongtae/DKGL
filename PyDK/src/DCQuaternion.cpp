#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCQuaternion
{
	PyObject_HEAD
	DKQuaternion quaternion;
};

static PyObject* DCQuaternionNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCQuaternion* self = (DCQuaternion*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->quaternion) DKQuaternion();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCQuaternionInit(DCQuaternion *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCQuaternionConverter(args, &self->quaternion))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCQuaternionConverter, &self->quaternion))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->quaternion = DKQuaternion(0, 0, 0, 1);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be four floating points (x,y,z,w) or empty");
	return -1;
}

static void DCQuaternionDealloc(DCQuaternion* self)
{
	self->quaternion.~DKQuaternion();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCQuaternionRepr(DCQuaternion* self)
{
	DKString str = DKString::Format("<%s object (x:%.3f, y:%.3f, z:%.3f, w:%.3f)>",
		Py_TYPE(self)->tp_name, self->quaternion.x, self->quaternion.y, self->quaternion.z, self->quaternion.w);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCQuaternionRichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKQuaternion* vec1 = DCQuaternionToObject(obj1);
	DKQuaternion* vec2 = DCQuaternionToObject(obj2);
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

static PyObject* DCQuaternionLength(DCQuaternion* self, PyObject*)
{
	return PyFloat_FromDouble(self->quaternion.Length());
}

static PyObject* DCQuaternionLengthSq(DCQuaternion* self, PyObject*)
{
	return PyFloat_FromDouble(self->quaternion.LengthSq());
}

static PyObject* DCQuaternionNormalize(DCQuaternion* self, PyObject*)
{
	DKQuaternion v(self->quaternion);
	v.Normalize();
	return DCQuaternionFromObject(&v);
}

static PyObject* DCQuaternionDot(DCQuaternion* self, PyObject* args)
{
	PyObject* obj = NULL;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKQuaternion* quat = DCQuaternionToObject(obj);
	if (quat == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Quaternion object.");
		return NULL;
	}
	float d = DKQuaternion::Dot(self->quaternion, *quat);
	return PyFloat_FromDouble(d);
}

static PyObject* DCQuaternionSlerp(DCQuaternion* self, PyObject* args)
{
	DKQuaternion quat;
	float t;
	if (!PyArg_ParseTuple(args, "O&f", &DCQuaternionConverter, &quat, &t))
		return NULL;

	DKQuaternion q = DKQuaternion::Slerp(self->quaternion, quat, t);
	return DCQuaternionFromObject(&q);
}

static PyObject* DCQuaternionAxis(DCQuaternion* self, PyObject*)
{
	DKVector3 axis = self->quaternion.Axis();
	return DCVector3FromObject(&axis);
}

static PyObject* DCQuaternionAxisAngle(DCQuaternion* self, PyObject*)
{
	float angle = 0.0f;
	DKVector3 axis = self->quaternion.AxisAngle(&angle);
	PyObject* tuple = PyTuple_New(2);
	PyTuple_SET_ITEM(tuple, 0, DCVector3FromObject(&axis));
	PyTuple_SET_ITEM(tuple, 1, PyFloat_FromDouble(angle));
	return tuple;
}

static PyObject* DCQuaternionAngle(DCQuaternion* self, PyObject*)
{
	return PyFloat_FromDouble(self->quaternion.Angle());
}

static PyObject* DCQuaternionInverse(DCQuaternion* self, PyObject*)
{
	self->quaternion.Inverse();
	Py_RETURN_NONE;
}

static PyObject* DCQuaternionConjugate(DCQuaternion* self, PyObject*)
{
	self->quaternion.Conjugate();
	Py_RETURN_NONE;
}

static PyObject* DCQuaternionRoll(DCQuaternion* self, PyObject*)
{
	return PyFloat_FromDouble(self->quaternion.Roll());
}

static PyObject* DCQuaternionPitch(DCQuaternion* self, PyObject*)
{
	return PyFloat_FromDouble(self->quaternion.Pitch());
}

static PyObject* DCQuaternionYaw(DCQuaternion* self, PyObject*)
{
	return PyFloat_FromDouble(self->quaternion.Yaw());
}

static PyObject* DCQuaternionMatrix3(DCQuaternion* self, PyObject*)
{
	DKMatrix3 m = self->quaternion.Matrix3();
	return DCMatrix3FromObject(&m);
}

static PyObject* DCQuaternionMatrix4(DCQuaternion* self, PyObject*)
{
	DKMatrix4 m = self->quaternion.Matrix4();
	return DCMatrix4FromObject(&m);
}

static PyObject* DCQuaternionFromEuler(PyObject* cls, PyObject* args, PyObject* kwds)
{
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;

	char* kwlist[] = { "pitch", "yaw", "roll", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "fff", kwlist, &pitch, &yaw, &roll))
		return NULL;

	DKQuaternion quat(pitch, yaw, roll);
	return PyObject_CallFunction(cls, "ffff", quat.x, quat.y, quat.z, quat.w);
}

static PyObject* DCQuaternionFromAxisAngle(PyObject* cls, PyObject* args, PyObject* kwds)
{
	DKVector3 axis;
	float angle;
	char* kwlist[] = { "axis", "angle", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O&f", kwlist, &DCVector3Converter, &axis, &angle))
		return NULL;

	DKQuaternion quat(axis, angle);
	return PyObject_CallFunction(cls, "ffff", quat.x, quat.y, quat.z, quat.w);
}

static PyObject* DCQuaternionFromRotate(PyObject* cls, PyObject* args, PyObject* kwds)
{
	DKVector3 from;
	DKVector3 to;
	float t = 1.0;
	char* kwlist[] = { "from", "to", "t", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O&O&|f", kwlist,
		&DCVector3Converter, &from,
		&DCVector3Converter, &to,
		t))
		return NULL;

	DKQuaternion quat(from, to, t);
	return PyObject_CallFunction(cls, "ffff", quat.x, quat.y, quat.z, quat.w);
}

static PyMethodDef methods[] = {
	{ "length", (PyCFunction)&DCQuaternionLength, METH_NOARGS },
	{ "lengthSq", (PyCFunction)&DCQuaternionLengthSq, METH_NOARGS },
	{ "normalize", (PyCFunction)&DCQuaternionNormalize, METH_NOARGS },
	{ "dot", (PyCFunction)&DCQuaternionDot, METH_VARARGS, "dot-product" },
	{ "slerp", (PyCFunction)&DCQuaternionSlerp, METH_VARARGS, "slerp(quat, t:float)" },
	{ "axis", (PyCFunction)&DCQuaternionAxis, METH_NOARGS },
	{ "axisAngle", (PyCFunction)&DCQuaternionAxisAngle, METH_NOARGS, "(axis, angle)" },
	{ "angle", (PyCFunction)&DCQuaternionAngle, METH_NOARGS },
	{ "roll", (PyCFunction)&DCQuaternionRoll, METH_NOARGS },
	{ "pitch", (PyCFunction)&DCQuaternionPitch, METH_NOARGS },
	{ "yaw", (PyCFunction)&DCQuaternionYaw, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCQuaternionInverse, METH_NOARGS },
	{ "conjugate", (PyCFunction)&DCQuaternionConjugate, METH_NOARGS },
	{ "matrix3", (PyCFunction)&DCQuaternionMatrix3, METH_NOARGS },
	{ "matrix4", (PyCFunction)&DCQuaternionMatrix4, METH_NOARGS },

	{ "quaternionEuler", (PyCFunction)&DCQuaternionFromEuler, METH_VARARGS | METH_KEYWORDS | METH_CLASS, "create quaternion from pitch, yaw, roll." },
	{ "quaternionAxisAngle", (PyCFunction)&DCQuaternionFromAxisAngle, METH_VARARGS | METH_KEYWORDS | METH_CLASS, "create quaternion from axis, angle." },
	{ "quaternionRotate", (PyCFunction)&DCQuaternionFromRotate, METH_VARARGS | METH_KEYWORDS | METH_CLASS, "create quaternion from vector3 rotate in time (from, to, t=1.0)." },

	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCQuaternionGetAttr(DCQuaternion* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->quaternion);
	return PyFloat_FromDouble(*attr);
}

static int DCQuaternionSetAttr(DCQuaternion* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->quaternion);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCQuaternionTuple(DCQuaternion* self, void*)
{
	return Py_BuildValue("ffff", self->quaternion.x, self->quaternion.y, self->quaternion.z, self->quaternion.w);
}

static int DCQuaternionSetTuple(DCQuaternion* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffff",
		&self->quaternion.x, &self->quaternion.y, &self->quaternion.z, &self->quaternion.w))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of four floating point numbers (x, y, z, w).");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "x", (getter)&DCQuaternionGetAttr, (setter)&DCQuaternionSetAttr, 0, reinterpret_cast<void*>(offsetof(DKQuaternion, x)) },
	{ "y", (getter)&DCQuaternionGetAttr, (setter)&DCQuaternionSetAttr, 0, reinterpret_cast<void*>(offsetof(DKQuaternion, y)) },
	{ "z", (getter)&DCQuaternionGetAttr, (setter)&DCQuaternionSetAttr, 0, reinterpret_cast<void*>(offsetof(DKQuaternion, z)) },
	{ "w", (getter)&DCQuaternionGetAttr, (setter)&DCQuaternionSetAttr, 0, reinterpret_cast<void*>(offsetof(DKQuaternion, w)) },
	{ "tuple", (getter)&DCQuaternionTuple, (setter)&DCQuaternionSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCQuaternionUnaryOperand(PyObject* obj, bool(*fn)(const DKQuaternion&, DKQuaternion&))
{
	DKQuaternion* q = DCQuaternionToObject(obj);
	if (q)
	{
		DKQuaternion res;
		if (fn(*q, res))
			return DCQuaternionFromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCQuaternionBinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKQuaternion&, const DKQuaternion&, DKQuaternion&))
{
	DKQuaternion* p = DCQuaternionToObject(lhs);
	DKQuaternion quat;
	if (p && DCQuaternionConverter(rhs, &quat))
	{
		DKQuaternion res;
		if (fn(*p, quat, res))
			return DCQuaternionFromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCQuaternionBinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKQuaternion&, double, DKQuaternion&))
{
	DKQuaternion* quat = DCQuaternionToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKQuaternion res;
		if (fn(*quat, d, res))
			return DCQuaternionFromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCQuaternionAdd(PyObject* lhs, PyObject* rhs)
{
	return DCQuaternionBinaryOperand(lhs, rhs, [](const DKQuaternion& q1, const DKQuaternion& q2, DKQuaternion& r)
	{
		r = q1 + q2;
		return true;
	});
}

static PyObject* DCQuaternionSubtract(PyObject* lhs, PyObject* rhs)
{
	return DCQuaternionBinaryOperand(lhs, rhs, [](const DKQuaternion& q1, const DKQuaternion& q2, DKQuaternion& r)
	{
		r = q1 - q2;
		return true;
	});
}

static PyObject* DCQuaternionMultiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCQuaternionTypeObject()))
		return DCQuaternionBinaryOperand(lhs, rhs, [](const DKQuaternion& q1, const DKQuaternion& q2, DKQuaternion& r)
	{
		r = q1 * q2;
		return true;
	});
	return DCQuaternionBinaryOperand(lhs, rhs, [](const DKQuaternion& q, double d, DKQuaternion& r)
	{
		r = q * d;
		return true;
	});
}

static PyObject* DCQuaternionNegative(PyObject* obj)
{
	return DCQuaternionUnaryOperand(obj, [](const DKQuaternion& q, DKQuaternion& r)
	{
		r = DKQuaternion(-q.x, -q.y, -q.z, -q.w);
		return true;
	});
}

static PyObject* DCQuaternionPositive(PyObject* obj)
{
	return DCQuaternionUnaryOperand(obj, [](const DKQuaternion& q, DKQuaternion& r)
	{
		r = q;
		return true;
	});
}

static PyObject* DCQuaternionAbsolute(PyObject* obj)
{
	return DCQuaternionUnaryOperand(obj, [](const DKQuaternion& q, DKQuaternion& r)
	{
		float x = q.x < 0 ? -q.x : q.x;
		float y = q.y < 0 ? -q.y : q.y;
		float z = q.z < 0 ? -q.z : q.z;
		float w = q.w < 0 ? -q.w : q.w;
		r = DKQuaternion(x, y, z, w);
		return true;
	});
}

static PyObject* DCQuaternionFloorDivide(PyObject *lhs, PyObject *rhs)
{
	return DCQuaternionBinaryOperand(lhs, rhs, [](const DKQuaternion& q, double d, DKQuaternion& r)
	{
		if (d != 0.0)
		{
			r = DKQuaternion(floor(q.x / d), floor(q.y / d), floor(q.z / d), floor(q.w / d));
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyObject* DCQuaternionTrueDivide(PyObject *lhs, PyObject *rhs)
{
	return DCQuaternionBinaryOperand(lhs, rhs, [](const DKQuaternion& q, double d, DKQuaternion& r)
	{
		if (d != 0.0)
		{
			r = q / d;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyNumberMethods numberMethods = {
	(binaryfunc)&DCQuaternionAdd,			/* nb_add */
	(binaryfunc)&DCQuaternionSubtract,		/* nb_subtract */
	(binaryfunc)&DCQuaternionMultiply,		/* nb_multiply */
	0,										/* nb_remainder */
	0,										/* nb_divmod */
	0,										/* nb_power */
	(unaryfunc)&DCQuaternionNegative,		/* nb_negative */
	(unaryfunc)&DCQuaternionPositive,		/* nb_positive */
	(unaryfunc)&DCQuaternionAbsolute,		/* nb_absolute */
	0,										/* nb_bool */
	0,										/* nb_invert */
	0,										/* nb_lshift */
	0,										/* nb_rshift */
	0,										/* nb_and */
	0,										/* nb_xor */
	0,										/* nb_or */
	0,										/* nb_int */
	0,										/* nb_reserved */
	0,										/* nb_float */
	0,										/* nb_inplace_add */
	0,										/* nb_inplace_subtract */
	0,										/* nb_inplace_multiply */
	0,										/* nb_inplace_remainder */
	0,										/* nb_inplace_power */
	0,										/* nb_inplace_lshift */
	0,										/* nb_inplace_rshift */
	0,										/* nb_inplace_and */
	0,										/* nb_inplace_xor */
	0,										/* nb_inplace_or */
	(binaryfunc)&DCQuaternionFloorDivide,	/* nb_floor_divide */
	(binaryfunc)&DCQuaternionTrueDivide,	/* nb_true_divide */
	0,										/* nb_inplace_floor_divide */
	0,										/* nb_inplace_true_divide */
	0										/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Quaternion",					/* tp_name */
	sizeof(DCQuaternion),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCQuaternionDealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCQuaternionRepr,						/* tp_repr */
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
	(richcmpfunc)&DCQuaternionRichCompare,				/* tp_richcompare */
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
	(initproc)&DCQuaternionInit,						/* tp_init */
	0,												/* tp_alloc */
	&DCQuaternionNew,									/* tp_new */
};

PyTypeObject* DCQuaternionTypeObject(void)
{
	return &objectType;
}

PyObject* DCQuaternionFromObject(DKQuaternion* quaternion)
{
	if (quaternion)
	{
		PyObject* args = Py_BuildValue("ffff", quaternion->x, quaternion->y, quaternion->z, quaternion->w);
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

DKQuaternion* DCQuaternionToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCQuaternion*)obj)->quaternion;
	}
	return NULL;
}

int DCQuaternionConverter(PyObject* obj, DKQuaternion* p)
{
	DKQuaternion* tmp = DCQuaternionToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	DKVector4* vec = DCVector4ToObject(obj);
	if (vec)
	{
		*p = DKQuaternion(vec->x, vec->y, vec->z, vec->w);
		return true;
	}
	if (obj && PyTuple_Check(obj))
	{
		Py_ssize_t numArgs = PyTuple_GET_SIZE(obj);
		if (numArgs == 4 && PyArg_ParseTuple(obj, "ffff", &p->x, &p->y, &p->z, &p->w))
			return true;
		else if (numArgs == 2)	// axis, angle
		{
			DKVector3 axis;
			float angle;
			if (PyArg_ParseTuple(obj, "O&f", &DCVector3Converter, &axis, &angle))
			{
				*p = DKQuaternion(axis, angle);
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Quaternion object or four floats tuple.");
	return false;
}
