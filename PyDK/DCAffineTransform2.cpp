#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCAffineTransform2
{
	PyObject_HEAD
	DKAffineTransform2 affineTransform2;
};

static PyObject* DCAffineTransform2New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCAffineTransform2* self = (DCAffineTransform2*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->affineTransform2) DKAffineTransform2();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCAffineTransform2Init(DCAffineTransform2 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	if (numArgs > 1 && DCAffineTransform2Converter(args, &self->affineTransform2))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCAffineTransform2Converter, &self->affineTransform2))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->affineTransform2.Identity();
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Matrix3 or LinearTransform2 with Vector2 or three Vector2s or empty.");
	return -1;
}

static void DCAffineTransform2Dealloc(DCAffineTransform2* self)
{
	self->affineTransform2.~DKAffineTransform2();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCAffineTransform2Repr(DCAffineTransform2* self)
{
	const DKMatrix2& mat = self->affineTransform2.matrix2;
	const DKVector2& pos = self->affineTransform2.translation;

	DKString str = DKString::Format(
		"<%s object\n"
		"Matrix2: (%.3f, %.3f)\n"
		"         (%.3f, %.3f)\n"
		" origin: (%.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		mat.m[0][0], mat.m[0][1],
		mat.m[1][0], mat.m[1][1],
		pos.x, pos.y);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCAffineTransform2RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKAffineTransform2* t1 = DCAffineTransform2ToObject(obj1);
	DKAffineTransform2* t2 = DCAffineTransform2ToObject(obj2);
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

static PyObject* DCAffineTransform2IsIdentity(DCAffineTransform2* self, PyObject*)
{
	if (self->affineTransform2.IsIdentity()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCAffineTransform2IsDiagonal(DCAffineTransform2* self, PyObject*)
{
	if (self->affineTransform2.IsDiagonal()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCAffineTransform2Inverse(DCAffineTransform2* self, PyObject*)
{
	self->affineTransform2.Inverse();
	Py_RETURN_NONE;
}

static PyObject* DCAffineTransform2Matrix2(DCAffineTransform2* self, PyObject*)
{
	return DCMatrix2FromObject(&self->affineTransform2.matrix2);
}

static PyObject* DCAffineTransform2Matrix3(DCAffineTransform2* self, PyObject*)
{
	DKMatrix3 mat = self->affineTransform2.Matrix3();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCAffineTransform2Identity(DCAffineTransform2* self, PyObject*)
{
	self->affineTransform2.Identity();
	Py_RETURN_NONE;
}

static PyObject* DCAffineTransform2Translate(DCAffineTransform2* self, PyObject* args)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	DKVector2 pos;
	if (numArgs > 1 && DCVector2Converter(args, &pos))
	{
		self->affineTransform2.Translate(pos);
		Py_RETURN_NONE;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCVector2Converter, &pos))
	{
		self->affineTransform2.Translate(pos);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Vector2 or two floats.");
	return NULL;
}

static PyObject* DCAffineTransform2Multiply(DCAffineTransform2* self, PyObject* args)
{
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKLinearTransform2* linear = DCLinearTransform2ToObject(obj);
	if (linear)
	{
		self->affineTransform2.Multiply(*linear);
		Py_RETURN_NONE;
	}
	DKAffineTransform2 trans;
	if (DCAffineTransform2Converter(obj, &trans))
	{
		self->affineTransform2.Multiply(trans);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be AffineTransform2 or LinearTransform2 object.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "isIdentity", (PyCFunction)&DCAffineTransform2IsIdentity, METH_NOARGS },
	{ "isDiagonal", (PyCFunction)&DCAffineTransform2IsDiagonal, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCAffineTransform2Inverse, METH_NOARGS },
	{ "matrix2", (PyCFunction)&DCAffineTransform2Matrix2, METH_NOARGS },
	{ "matrix3", (PyCFunction)&DCAffineTransform2Matrix3, METH_NOARGS },
	{ "identity", (PyCFunction)&DCAffineTransform2Identity, METH_NOARGS },
	{ "translate", (PyCFunction)&DCAffineTransform2Translate, METH_VARARGS },
	{ "multiply", (PyCFunction)&DCAffineTransform2Multiply, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCAffineTransform2Translation(DCAffineTransform2* self, void*)
{
	const DKVector2& vec = self->affineTransform2.translation;
	return Py_BuildValue("ff", vec.x, vec.y);
}

static int DCAffineTransform2SetTranslation(DCAffineTransform2* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector2& vec = self->affineTransform2.translation;
	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ff", &vec.x, &vec.y))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of two floats.");
	return -1;
}

static PyObject* DCAffineTransform2Tuple(DCAffineTransform2* self, void*)
{
	const DKMatrix2& mat = self->affineTransform2.matrix2;
	const DKVector2& vec = self->affineTransform2.translation;
	return Py_BuildValue("ffffff",
		mat.m[0][0], mat.m[0][1],
		mat.m[1][0], mat.m[1][1],
		vec.x, vec.y);
}

static int DCAffineTransform2SetTuple(DCAffineTransform2* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKMatrix2& mat = self->affineTransform2.matrix2;
	DKVector2& vec = self->affineTransform2.translation;

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffffff",
		&mat.m[0][0], &mat.m[0][1],
		&mat.m[1][0], &mat.m[1][1],
		&vec.x, &vec.y))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of six floats.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "translation", (getter)&DCAffineTransform2Translation, (setter)&DCAffineTransform2SetTranslation, 0, 0 },
	{ "tuple", (getter)&DCAffineTransform2Tuple, (setter)&DCAffineTransform2SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyObject* DCAffineTransform2Mul(PyObject* lhs, PyObject* rhs)
{
	DKAffineTransform2* p = DCAffineTransform2ToObject(lhs);
	DKAffineTransform2 t;
	if (p && DCAffineTransform2Converter(rhs, &t))
	{
		DKAffineTransform2 res = (*p) * t;
		return DCAffineTransform2FromObject(&res);
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyNumberMethods numberMethods = {
	0,												/* nb_add */
	0,												/* nb_subtract */
	(binaryfunc)&DCAffineTransform2Mul,				/* nb_multiply */
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
	PYDK_MODULE_NAME ".AffineTransform2",			/* tp_name */
	sizeof(DCAffineTransform2),						/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCAffineTransform2Dealloc,			/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCAffineTransform2Repr,				/* tp_repr */
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
	(richcmpfunc)&DCAffineTransform2RichCompare,	/* tp_richcompare */
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
	(initproc)&DCAffineTransform2Init,				/* tp_init */
	0,												/* tp_alloc */
	&DCAffineTransform2New,							/* tp_new */
};

PyTypeObject* DCAffineTransform2TypeObject(void)
{
	return &objectType;
}

PyObject* DCAffineTransform2FromObject(DKAffineTransform2* affineTransform2)
{
	if (affineTransform2)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		DCAffineTransform2* self = (DCAffineTransform2*)PyObject_Call(tp, args, kwds);
		if (self)
		{
			self->affineTransform2 = *affineTransform2;
		}
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKAffineTransform2* DCAffineTransform2ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCAffineTransform2*)obj)->affineTransform2;
	}
	return NULL;
}

int DCAffineTransform2Converter(PyObject* obj, DKAffineTransform2* p)
{
	DKAffineTransform2* trans = DCAffineTransform2ToObject(obj);
	if (trans)
	{
		*p = *trans;
		return true;
	}
	DKLinearTransform2* lt = DCLinearTransform2ToObject(obj);
	if (lt)
	{
		*p = *lt;
		return true;
	}
	DKMatrix2* mat2 = DCMatrix2ToObject(obj);
	if (mat2)
	{
		*p = DKLinearTransform2(*mat2);
		return true;
	}
	DKMatrix3* mat3 = DCMatrix3ToObject(obj);
	if (mat3)
	{
		*p = *mat3;
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
		Py_ssize_t numArgs = PyTuple_GET_SIZE(obj);
		if (numArgs == 6)
		{
			DKMatrix2& mat = p->matrix2;
			DKVector2& vec = p->translation;
			if (PyArg_ParseTuple(obj, "ffffff",
				&mat.m[0][0], &mat.m[0][1],
				&mat.m[1][0], &mat.m[1][1],
				&vec.x, &vec.y))
				return true;
		}
		else if (numArgs > 2)
		{
			DKVector2 axisX;
			DKVector2 axisY;
			DKVector2 origin(0,0);

			if (PyArg_ParseTuple(obj, "O&O&|O&",
				&DCVector2Converter, &axisX,
				&DCVector2Converter, &axisY,
				&DCVector2Converter, &origin))
			{
				*p = DKAffineTransform2(axisX, axisY, origin);
				return true;
			}
		}
		else
		{
			DKLinearTransform2 linear2;
			DKVector2 translate(0, 0);

			if (PyArg_ParseTuple(obj, "O&|O&",
				&DCLinearTransform2Converter, &linear2,
				&DCVector2Converter, &translate))
			{
				*p = DKAffineTransform2(linear2, translate);
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be one of following: AffineTransform2, Matrix3, LinearTransform2 with Vector2, three Vector2s.");
	return false;
}
