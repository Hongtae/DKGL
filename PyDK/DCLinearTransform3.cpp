#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCLinearTransform3
{
	PyObject_HEAD
	DKLinearTransform3 linearTransform3;
};

static PyObject* DCLinearTransform3New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCLinearTransform3* self = (DCLinearTransform3*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->linearTransform3) DKLinearTransform3();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCLinearTransform3Init(DCLinearTransform3 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	if (numArgs > 1 && DCLinearTransform3Converter(args, &self->linearTransform3))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCLinearTransform3Converter, &self->linearTransform3))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->linearTransform3.Identity();
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Matrix3 or Quaternion or three Vector3s or empty.");
	return -1;
}

static void DCLinearTransform3Dealloc(DCLinearTransform3* self)
{
	self->linearTransform3.~DKLinearTransform3();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCLinearTransform3Repr(DCLinearTransform3* self)
{
	const DKMatrix3& mat = self->linearTransform3.Matrix3();

	DKString str = DKString::Format(
		"<%s object\n"
		"  (%.3f, %.3f, %.3f)\n"
		"  (%.3f, %.3f, %.3f)\n"
		"  (%.3f, %.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		mat.m[0][0], mat.m[0][1], mat.m[0][2],
		mat.m[1][0], mat.m[1][1], mat.m[1][2],
		mat.m[2][0], mat.m[2][1], mat.m[2][2]);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCLinearTransform3RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKLinearTransform3* t1 = DCLinearTransform3ToObject(obj1);
	DKLinearTransform3* t2 = DCLinearTransform3ToObject(obj2);
	if (t1 && t2)
	{
		if (op == Py_EQ)
		{
			if (*t1 == *t2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
		else if (op == Py_NE)
		{
			if (*t1 != *t2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCLinearTransform3IsIdentity(DCLinearTransform3* self, PyObject*)
{
	if (self->linearTransform3.IsIdentity()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCLinearTransform3IsDiagonal(DCLinearTransform3* self, PyObject*)
{
	if (self->linearTransform3.IsDiagonal()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCLinearTransform3Inverse(DCLinearTransform3* self, PyObject*)
{
	DKLinearTransform3 mat = self->linearTransform3;
	mat.Inverse();
	return DCLinearTransform3FromObject(&mat);
}

static PyObject* DCLinearTransform3Matrix3(DCLinearTransform3* self, PyObject*)
{
	return DCMatrix3FromObject(&self->linearTransform3.matrix3);
}

static PyObject* DCLinearTransform3Rotation(DCLinearTransform3* self, PyObject*)
{
	DKQuaternion quat = self->linearTransform3.Rotation();
	return DCQuaternionFromObject(&quat);
}

static PyObject* DCLinearTransform3Identity(DCLinearTransform3* self, PyObject*)
{
	self->linearTransform3.Identity();
	Py_RETURN_NONE;
}

static PyObject* DCLinearTransform3Scale(DCLinearTransform3* self, PyObject* args)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	DKVector3 scale;

	if (numArgs > 1 && DCVector3Converter(args, &scale))
	{
		self->linearTransform3.Scale(scale);
		Py_RETURN_NONE;
	}
	else if (numArgs == 1)
	{
		if (PyArg_ParseTuple(args, "O&", &DCVector3Converter, &scale))
		{
			self->linearTransform3.Scale(scale);
			Py_RETURN_NONE;
		}
		PyErr_Clear();
		float s;
		if (PyArg_ParseTuple(args, "f", &s))
		{
			self->linearTransform3.Scale(s);
			Py_RETURN_NONE;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Vector3 or three floats (one float for uniform-scale).");
	return NULL;
}

static PyObject* DCLinearTransform3Rotate(DCLinearTransform3* self, PyObject* args)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	DKQuaternion quat;
	if (numArgs > 1 && DCQuaternionConverter(args, &quat))
	{
		self->linearTransform3.Rotate(quat);
		Py_RETURN_NONE;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCQuaternionConverter, &quat))
	{
		self->linearTransform3.Rotate(quat);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Quaternion object.");
	return NULL;
}

template <DKLinearTransform3& (DKLinearTransform3::*fn)(float)>
static PyObject* SetFloat(DCLinearTransform3* self, PyObject* args)
{
	float f = 0.0f;
	if (!PyArg_ParseTuple(args, "f", &f))
		return NULL;

	(self->linearTransform3.*fn)(f);
	Py_RETURN_NONE;
}

static PyObject* DCLinearTransform3Multiply(DCLinearTransform3* self, PyObject* args)
{
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKMatrix3* mat3 = DCMatrix3ToObject(obj);
	if (mat3)
	{
		self->linearTransform3.Multiply(*mat3);
		Py_RETURN_NONE;
	}
	DKLinearTransform3 trans;
	if (DCLinearTransform3Converter(obj, &trans))
	{
		self->linearTransform3.Multiply(trans);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be LinearTransform3 or Matrix3 object.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "isIdentity", (PyCFunction)&DCLinearTransform3IsIdentity, METH_NOARGS },
	{ "isDiagonal", (PyCFunction)&DCLinearTransform3IsDiagonal, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCLinearTransform3Inverse, METH_NOARGS },
	{ "matrix3", (PyCFunction)&DCLinearTransform3Matrix3, METH_NOARGS },
	{ "rotation", (PyCFunction)&DCLinearTransform3Rotation, METH_NOARGS },
	{ "identity", (PyCFunction)&DCLinearTransform3Identity, METH_NOARGS },
	{ "scale", (PyCFunction)&DCLinearTransform3Scale, METH_VARARGS },
	{ "rotate", (PyCFunction)&DCLinearTransform3Rotate, METH_VARARGS },
	{ "rotateX", (PyCFunction)&SetFloat<&DKLinearTransform3::RotateX>, METH_VARARGS },
	{ "rotateY", (PyCFunction)&SetFloat<&DKLinearTransform3::RotateY>, METH_VARARGS },
	{ "rotateZ", (PyCFunction)&SetFloat<&DKLinearTransform3::RotateZ>, METH_VARARGS },
	{ "multiply", (PyCFunction)&DCLinearTransform3Multiply, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCLinearTransform3Tuple(DCLinearTransform3* self, void*)
{
	const DKMatrix3& mat = self->linearTransform3.matrix3;
	return Py_BuildValue("fffffffff",
		mat.m[0][0], mat.m[0][1], mat.m[0][2],
		mat.m[1][0], mat.m[1][1], mat.m[1][2],
		mat.m[2][0], mat.m[2][1], mat.m[2][2]);
}

static int DCLinearTransform3SetTuple(DCLinearTransform3* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKMatrix3& mat = self->linearTransform3.matrix3;

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "fffffffff",
		&mat.m[0][0], &mat.m[0][1], &mat.m[0][2],
		&mat.m[1][0], &mat.m[1][1], &mat.m[1][2],
		&mat.m[2][0], &mat.m[2][1], &mat.m[2][2]))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of nine floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "tuple", (getter)&DCLinearTransform3Tuple, (setter)&DCLinearTransform3SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyObject* DCLinearTransform3Multiply(PyObject* lhs, PyObject* rhs)
{
	DKLinearTransform3* p = DCLinearTransform3ToObject(lhs);
	DKLinearTransform3 t;
	if (p && DCLinearTransform3Converter(rhs, &t))
	{
		DKLinearTransform3 res = (*p) * t;
		return DCLinearTransform3FromObject(&res);
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyNumberMethods numberMethods = {
	0,												/* nb_add */
	0,												/* nb_subtract */
	(binaryfunc)&DCLinearTransform3Multiply,		/* nb_multiply */
	0,												/* nb_remainder */
	0,												/* nb_divmod */
	0,												/* nb_power */
	0,												/* nb_negative */
	0,												/* nb_positive */
	0,												/* nb_absolute */
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
	0,												/* nb_floor_divide */
	0,												/* nb_true_divide */
	0,												/* nb_inplace_floor_divide */
	0,												/* nb_inplace_true_divide */
	0												/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".LinearTransform3",			/* tp_name */
	sizeof(DCLinearTransform3),						/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCLinearTransform3Dealloc,			/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCLinearTransform3Repr,				/* tp_repr */
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
	(richcmpfunc)&DCLinearTransform3RichCompare,	/* tp_richcompare */
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
	(initproc)&DCLinearTransform3Init,				/* tp_init */
	0,												/* tp_alloc */
	&DCLinearTransform3New,							/* tp_new */
};

PyTypeObject* DCLinearTransform3TypeObject(void)
{
	return &objectType;
}

PyObject* DCLinearTransform3FromObject(DKLinearTransform3* linearTransform3)
{
	if (linearTransform3)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		DCLinearTransform3* self = (DCLinearTransform3*)PyObject_Call(tp, args, kwds);
		if (self)
		{
			self->linearTransform3 = *linearTransform3;
		}
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKLinearTransform3* DCLinearTransform3ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCLinearTransform3*)obj)->linearTransform3;
	}
	return NULL;
}

int DCLinearTransform3Converter(PyObject* obj, DKLinearTransform3* p)
{
	DKLinearTransform3* trans = DCLinearTransform3ToObject(obj);
	if (trans)
	{
		*p = *trans;
		return true;
	}
	DKMatrix3* mat = DCMatrix3ToObject(obj);
	if (mat)
	{
		*p = *mat;
		return true;
	}
	DKQuaternion* quat = DCQuaternionToObject(obj);
	if (quat)
	{
		*p = *quat;
		return true;
	}
	if (obj && PyTuple_Check(obj))
	{
		Py_ssize_t numArgs = PyTuple_GET_SIZE(obj);
		if (numArgs == 9)
		{
			DKMatrix3& mat = p->matrix3;
			if (PyArg_ParseTuple(obj, "fffffffff",
				&mat.m[0][0], &mat.m[0][1], &mat.m[0][2],
				&mat.m[1][0], &mat.m[1][1], &mat.m[1][2],
				&mat.m[2][0], &mat.m[2][1], &mat.m[2][2]))
				return true;
		}
		else
		{
			DKVector3 left;
			DKVector3 up;
			DKVector3 forward;

			if (PyArg_ParseTuple(obj, "O&O&O&",
				&DCVector3Converter, &left,
				&DCVector3Converter, &up,
				&DCVector3Converter, &forward))
			{
				*p = DKLinearTransform3(left, up, forward);
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be one of following: LinearTransform3, Matrix3, Quaternion, three Vector3s.");
	return false;
}
