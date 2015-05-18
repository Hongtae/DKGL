#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCLinearTransform2
{
	PyObject_HEAD
	DKLinearTransform2 linearTransform2;
};

static PyObject* DCLinearTransform2New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCLinearTransform2* self = (DCLinearTransform2*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->linearTransform2) DKLinearTransform2();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCLinearTransform2Init(DCLinearTransform2 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	if (numArgs > 1 && DCLinearTransform2Converter(args, &self->linearTransform2))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCLinearTransform2Converter, &self->linearTransform2))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->linearTransform2.Identity();
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Matrix2 or two Vector2s or empty.");
	return -1;
}

static void DCLinearTransform2Dealloc(DCLinearTransform2* self)
{
	self->linearTransform2.~DKLinearTransform2();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCLinearTransform2Repr(DCLinearTransform2* self)
{
	const DKMatrix2& mat = self->linearTransform2.Matrix2();

	DKString str = DKString::Format(
		"<%s object\n"
		"  (%.3f, %.3f)\n"
		"  (%.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		mat.m[0][0], mat.m[0][1],
		mat.m[1][0], mat.m[1][1]);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCLinearTransform2RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKLinearTransform2* t1 = DCLinearTransform2ToObject(obj1);
	DKLinearTransform2* t2 = DCLinearTransform2ToObject(obj2);
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

static PyObject* DCLinearTransform2IsIdentity(DCLinearTransform2* self, PyObject*)
{
	if (self->linearTransform2.IsIdentity()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCLinearTransform2IsDiagonal(DCLinearTransform2* self, PyObject*)
{
	if (self->linearTransform2.IsDiagonal()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCLinearTransform2Inverse(DCLinearTransform2* self, PyObject*)
{
	DKLinearTransform2 mat = self->linearTransform2;
	mat.Inverse();
	return DCLinearTransform2FromObject(&mat);
}

static PyObject* DCLinearTransform2Matrix2(DCLinearTransform2* self, PyObject*)
{
	return DCMatrix2FromObject(&self->linearTransform2.matrix2);
}

static PyObject* DCLinearTransform2Scale(DCLinearTransform2* self, PyObject* args)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	DKVector2 scale;

	if (numArgs > 1 && DCVector2Converter(args, &scale))
	{
		self->linearTransform2.Scale(scale);
		Py_RETURN_NONE;
	}
	else if (numArgs == 1)
	{
		if (PyArg_ParseTuple(args, "O&", &DCVector2Converter, &scale))
		{
			self->linearTransform2.Scale(scale);
			Py_RETURN_NONE;
		}
		PyErr_Clear();
		float s;
		if (PyArg_ParseTuple(args, "f", &s))
		{
			self->linearTransform2.Scale(s);
			Py_RETURN_NONE;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Vector2 or two floats (one float for uniform-scale).");
	return NULL;
}

template <DKLinearTransform2& (DKLinearTransform2::*fn)(float)>
static PyObject* SetFloat(DCLinearTransform2* self, PyObject* args)
{
	float f = 0.0f;
	if (!PyArg_ParseTuple(args, "f", &f))
		return NULL;

	(self->linearTransform2.*fn)(f);
	Py_RETURN_NONE;
}

template <DKLinearTransform2& (DKLinearTransform2::*fn)(void)>
static PyObject* CallVFunc(DCLinearTransform2* self, PyObject*)
{
	(self->linearTransform2.*fn)();
	Py_RETURN_NONE;
}

static PyObject* DCLinearTransform2Multiply(DCLinearTransform2* self, PyObject* args)
{
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKMatrix2* mat2 = DCMatrix2ToObject(obj);
	if (mat2)
	{
		self->linearTransform2.Multiply(*mat2);
		Py_RETURN_NONE;
	}
	DKLinearTransform2 trans;
	if (DCLinearTransform2Converter(obj, &trans))
	{
		self->linearTransform2.Multiply(trans);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be LinearTransform2 or Matrix2 object.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "isIdentity", (PyCFunction)&DCLinearTransform2IsIdentity, METH_NOARGS },
	{ "isDiagonal", (PyCFunction)&DCLinearTransform2IsDiagonal, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCLinearTransform2Inverse, METH_NOARGS },
	{ "matrix2", (PyCFunction)&DCLinearTransform2Matrix2, METH_NOARGS },
	{ "identity", (PyCFunction)&CallVFunc<&DKLinearTransform2::Identity>, METH_NOARGS },
	{ "scale", (PyCFunction)&DCLinearTransform2Scale, METH_VARARGS },
	{ "rotate", (PyCFunction)&SetFloat<&DKLinearTransform2::Rotate>, METH_VARARGS },
	{ "squeeze", (PyCFunction)&SetFloat<&DKLinearTransform2::Squeeze>, METH_VARARGS },
	{ "hshear", (PyCFunction)&SetFloat<&DKLinearTransform2::HorizontalShear>, METH_VARARGS },
	{ "vshear", (PyCFunction)&SetFloat<&DKLinearTransform2::VerticalShear>, METH_VARARGS },
	{ "hflip", (PyCFunction)&CallVFunc<&DKLinearTransform2::HorizontalFlip>, METH_VARARGS },
	{ "vflip", (PyCFunction)&CallVFunc<&DKLinearTransform2::VerticalFlip>, METH_VARARGS },
	{ "multiply", (PyCFunction)&DCLinearTransform2Multiply, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCLinearTransform2Tuple(DCLinearTransform2* self, void*)
{
	const DKMatrix2& mat = self->linearTransform2.matrix2;
	return Py_BuildValue("ffff",
		mat.m[0][0], mat.m[0][1],
		mat.m[1][0], mat.m[1][1]);
}

static int DCLinearTransform2SetTuple(DCLinearTransform2* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKMatrix2& mat = self->linearTransform2.matrix2;

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffff",
		&mat.m[0][0], &mat.m[0][1],
		&mat.m[1][0], &mat.m[1][1]))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of four floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "tuple", (getter)&DCLinearTransform2Tuple, (setter)&DCLinearTransform2SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyObject* DCLinearTransform2Mul(PyObject* lhs, PyObject* rhs)
{
	DKLinearTransform2* p = DCLinearTransform2ToObject(lhs);
	DKLinearTransform2 t;
	if (p && DCLinearTransform2Converter(rhs, &t))
	{
		DKLinearTransform2 res = (*p) * t;
		return DCLinearTransform2FromObject(&res);
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyNumberMethods numberMethods = {
	0,												/* nb_add */
	0,												/* nb_subtract */
	(binaryfunc)&DCLinearTransform2Mul,				/* nb_multiply */
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
	PYDK_MODULE_NAME ".LinearTransform2",			/* tp_name */
	sizeof(DCLinearTransform2),						/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCLinearTransform2Dealloc,			/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCLinearTransform2Repr,				/* tp_repr */
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
	(richcmpfunc)&DCLinearTransform2RichCompare,	/* tp_richcompare */
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
	(initproc)&DCLinearTransform2Init,				/* tp_init */
	0,												/* tp_alloc */
	&DCLinearTransform2New,							/* tp_new */
};

PyTypeObject* DCLinearTransform2TypeObject(void)
{
	return &objectType;
}

PyObject* DCLinearTransform2FromObject(DKLinearTransform2* linearTransform2)
{
	if (linearTransform2)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		DCLinearTransform2* self = (DCLinearTransform2*)PyObject_Call(tp, args, kwds);
		if (self)
		{
			self->linearTransform2 = *linearTransform2;
		}
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKLinearTransform2* DCLinearTransform2ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCLinearTransform2*)obj)->linearTransform2;
	}
	return NULL;
}

int DCLinearTransform2Converter(PyObject* obj, DKLinearTransform2* p)
{
	DKLinearTransform2* trans = DCLinearTransform2ToObject(obj);
	if (trans)
	{
		*p = *trans;
		return true;
	}
	DKMatrix2* mat = DCMatrix2ToObject(obj);
	if (mat)
	{
		*p = *mat;
		return true;
	}
	if (obj && PyFloat_Check(obj))
	{
		float rotate = PyFloat_AS_DOUBLE(obj);
		*p = rotate;
		return true;
	}
	if (obj && PyTuple_Check(obj))
	{
		Py_ssize_t numArgs = PyTuple_GET_SIZE(obj);
		if (numArgs == 4)
		{
			DKMatrix2& mat = p->matrix2;
			if (PyArg_ParseTuple(obj, "ffff",
				&mat.m[0][0], &mat.m[0][1],
				&mat.m[1][0], &mat.m[1][1]))
				return true;
		}
		else
		{
			DKVector2 axisX, axisY;

			if (PyArg_ParseTuple(obj, "O&O&",
				&DCVector2Converter, &axisX,
				&DCVector2Converter, &axisY))
			{
				*p = DKLinearTransform2(axisX, axisY);
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be one of following: LinearTransform2, Matrix2, two Vector2s.");
	return false;
}
