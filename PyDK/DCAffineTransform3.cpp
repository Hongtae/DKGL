#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCAffineTransform3
{
	PyObject_HEAD
	DKAffineTransform3 affineTransform3;
};

static PyObject* DCAffineTransform3New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCAffineTransform3* self = (DCAffineTransform3*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->affineTransform3) DKAffineTransform3();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCAffineTransform3Init(DCAffineTransform3 *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	if (numArgs > 1 && DCAffineTransform3Converter(args, &self->affineTransform3))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCAffineTransform3Converter, &self->affineTransform3))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->affineTransform3.Identity();
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Matrix4 or LinearTransform3 with Vector3 or four Vector3s or empty.");
	return -1;
}

static void DCAffineTransform3Dealloc(DCAffineTransform3* self)
{
	self->affineTransform3.~DKAffineTransform3();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCAffineTransform3Repr(DCAffineTransform3* self)
{
	const DKMatrix3& mat = self->affineTransform3.matrix3;
	const DKVector3& pos = self->affineTransform3.translation;

	DKString str = DKString::Format(
		"<%s object\n" 
		"Matrix3: (%.3f, %.3f, %.3f)\n"
		"         (%.3f, %.3f, %.3f)\n"
		"         (%.3f, %.3f, %.3f)\n"
		" origin: (%.3f, %.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		mat.m[0][0], mat.m[0][1], mat.m[0][2],
		mat.m[1][0], mat.m[1][1], mat.m[1][2],
		mat.m[2][0], mat.m[2][1], mat.m[2][2],
		pos.x, pos.y, pos.z);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCAffineTransform3RichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKAffineTransform3* t1 = DCAffineTransform3ToObject(obj1);
	DKAffineTransform3* t2 = DCAffineTransform3ToObject(obj2);
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

static PyObject* DCAffineTransform3IsIdentity(DCAffineTransform3* self, PyObject*)
{
	if (self->affineTransform3.IsIdentity()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCAffineTransform3IsDiagonal(DCAffineTransform3* self, PyObject*)
{
	if (self->affineTransform3.IsDiagonal()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCAffineTransform3Inverse(DCAffineTransform3* self, PyObject*)
{
	self->affineTransform3.Inverse();
	Py_RETURN_NONE;
}

static PyObject* DCAffineTransform3Matrix3(DCAffineTransform3* self, PyObject*)
{
	return DCMatrix3FromObject(&self->affineTransform3.matrix3);
}

static PyObject* DCAffineTransform3Matrix4(DCAffineTransform3* self, PyObject*)
{
	DKMatrix4 mat = self->affineTransform3.Matrix4();
	return DCMatrix4FromObject(&mat);
}

static PyObject* DCAffineTransform3Identity(DCAffineTransform3* self, PyObject*)
{
	self->affineTransform3.Identity();
	Py_RETURN_NONE;
}

static PyObject* DCAffineTransform3Translate(DCAffineTransform3* self, PyObject* args)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	DKVector3 pos;
	if (numArgs > 1 && DCVector3Converter(args, &pos))
	{
		self->affineTransform3.Translate(pos);
		Py_RETURN_NONE;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCVector3Converter, &pos))
	{
		self->affineTransform3.Translate(pos);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Vector3 or three floats.");
	return NULL;
}

static PyObject* DCAffineTransform3Multiply(DCAffineTransform3* self, PyObject* args)
{
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKLinearTransform3* linear = DCLinearTransform3ToObject(obj);
	if (linear)
	{
		self->affineTransform3.Multiply(*linear);
		Py_RETURN_NONE;
	}
	DKAffineTransform3 trans;
	if (DCAffineTransform3Converter(obj, &trans))
	{
		self->affineTransform3.Multiply(trans);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be AffineTransform3 or LinearTransform3 object.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "isIdentity", (PyCFunction)&DCAffineTransform3IsIdentity, METH_NOARGS },
	{ "isDiagonal", (PyCFunction)&DCAffineTransform3IsDiagonal, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCAffineTransform3Inverse, METH_NOARGS },
	{ "matrix3", (PyCFunction)&DCAffineTransform3Matrix3, METH_NOARGS },
	{ "matrix4", (PyCFunction)&DCAffineTransform3Matrix4, METH_NOARGS },
	{ "identity", (PyCFunction)&DCAffineTransform3Identity, METH_NOARGS },
	{ "translate", (PyCFunction)&DCAffineTransform3Translate, METH_VARARGS },
	{ "multiply", (PyCFunction)&DCAffineTransform3Multiply, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCAffineTransform3Translation(DCAffineTransform3* self, void*)
{
	const DKVector3& vec = self->affineTransform3.translation;
	return Py_BuildValue("fff", vec.x, vec.y, vec.z);
}

static int DCAffineTransform3SetTranslation(DCAffineTransform3* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3& vec = self->affineTransform3.translation;
	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "fff", &vec.x, &vec.y, &vec.z))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of three floats.");
	return -1;
}

static PyObject* DCAffineTransform3Tuple(DCAffineTransform3* self, void*)
{
	const DKMatrix3& mat = self->affineTransform3.matrix3;
	const DKVector3& vec = self->affineTransform3.translation;
	return Py_BuildValue("ffffffffffff",
		mat.m[0][0], mat.m[0][1], mat.m[0][2],
		mat.m[1][0], mat.m[1][1], mat.m[1][2],
		mat.m[2][0], mat.m[2][1], mat.m[2][2],
		vec.x, vec.y, vec.z);
}

static int DCAffineTransform3SetTuple(DCAffineTransform3* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKMatrix3& mat = self->affineTransform3.matrix3;
	DKVector3& vec = self->affineTransform3.translation;

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffffffffffff",
		&mat.m[0][0], &mat.m[0][1], &mat.m[0][2],
		&mat.m[1][0], &mat.m[1][1], &mat.m[1][2],
		&mat.m[2][0], &mat.m[2][1], &mat.m[2][2],
		&vec.x, &vec.y, &vec.z))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of twelve floats.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "translation", (getter)&DCAffineTransform3Translation, (setter)&DCAffineTransform3SetTranslation, 0, 0 },
	{ "tuple", (getter)&DCAffineTransform3Tuple, (setter)&DCAffineTransform3SetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyObject* DCAffineTransform3Mul(PyObject* lhs, PyObject* rhs)
{
	DKAffineTransform3* p = DCAffineTransform3ToObject(lhs);
	DKAffineTransform3 t;
	if (p && DCAffineTransform3Converter(rhs, &t))
	{
		DKAffineTransform3 res = (*p) * t;
		return DCAffineTransform3FromObject(&res);
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyNumberMethods numberMethods = {
	0,												/* nb_add */
	0,												/* nb_subtract */
	(binaryfunc)&DCAffineTransform3Mul,				/* nb_multiply */
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
	PYDK_MODULE_NAME ".AffineTransform3",			/* tp_name */
	sizeof(DCAffineTransform3),						/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCAffineTransform3Dealloc,			/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCAffineTransform3Repr,				/* tp_repr */
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
	(richcmpfunc)&DCAffineTransform3RichCompare,	/* tp_richcompare */
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
	(initproc)&DCAffineTransform3Init,				/* tp_init */
	0,												/* tp_alloc */
	&DCAffineTransform3New,							/* tp_new */
};

PyTypeObject* DCAffineTransform3TypeObject(void)
{
	return &objectType;
}

PyObject* DCAffineTransform3FromObject(DKAffineTransform3* affineTransform3)
{
	if (affineTransform3)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		DCAffineTransform3* self = (DCAffineTransform3*)PyObject_Call(tp, args, kwds);
		if (self)
		{
			self->affineTransform3 = *affineTransform3;
		}
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKAffineTransform3* DCAffineTransform3ToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCAffineTransform3*)obj)->affineTransform3;
	}
	return NULL;
}

int DCAffineTransform3Converter(PyObject* obj, DKAffineTransform3* p)
{
	DKAffineTransform3* trans = DCAffineTransform3ToObject(obj);
	if (trans)
	{
		*p = *trans;
		return true;
	}
	DKLinearTransform3* lt = DCLinearTransform3ToObject(obj);
	if (lt)
	{
		*p = *lt;
		return true;
	}
	DKMatrix3* mat3 = DCMatrix3ToObject(obj);
	if (mat3)
	{
		*p = DKLinearTransform3(*mat3);
		return true;
	}
	DKMatrix4* mat4 = DCMatrix4ToObject(obj);
	if (mat4)
	{
		*p = *mat4;
		return true;
	}
	DKVector3* vec = DCVector3ToObject(obj);
	if (vec)
	{
		*p = *vec;
		return true;
	}
	if (obj && PyTuple_Check(obj))
	{ 
		Py_ssize_t numArgs = PyTuple_GET_SIZE(obj);
		if (numArgs == 12)
		{
			DKMatrix3& mat = p->matrix3;
			DKVector3& vec = p->translation;
			if (PyArg_ParseTuple(obj, "ffffffffffff",
				&mat.m[0][0], &mat.m[0][1], &mat.m[0][2],
				&mat.m[1][0], &mat.m[1][1], &mat.m[1][2],
				&mat.m[2][0], &mat.m[2][1], &mat.m[2][2],
				&vec.x, &vec.y, &vec.z))
				return true;
		}
		else if (numArgs > 2)
		{
			DKVector3 left;
			DKVector3 up;
			DKVector3 forward;
			DKVector3 origin(0, 0, 0);

			if (PyArg_ParseTuple(obj, "O&O&O&|O&",
				&DCVector3Converter, &left,
				&DCVector3Converter, &up,
				&DCVector3Converter, &forward,
				&DCVector3Converter, &origin))
			{
				*p = DKAffineTransform3(left, up, forward, origin);
				return true;
			}
		}
		else
		{
			DKLinearTransform3 linear3;
			DKVector3 translate(0, 0, 0);

			if (PyArg_ParseTuple(obj, "O&|O&",
				&DCLinearTransform3Converter, &linear3,
				&DCVector3Converter, &translate))
			{
				*p = DKAffineTransform3(linear3, translate);
				return true;
			}
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be one of following: AffineTransform3, Matrix4, LinearTransform3 with Vector3, four Vector3s.");
	return false;
}
