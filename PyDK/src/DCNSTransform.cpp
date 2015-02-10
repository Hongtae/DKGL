#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCNSTransform
{
	PyObject_HEAD
	DKNSTransform transform;
};

static PyObject* DCNSTransformNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCNSTransform* self = (DCNSTransform*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->transform) DKNSTransform();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCNSTransformInit(DCNSTransform *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	if (numArgs > 1 && DCNSTransformConverter(args, &self->transform))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCNSTransformConverter, &self->transform))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->transform.Identity();
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be (orientation:Quaternion, position:Vector3) tuple.");
	return -1;
}

static void DCNSTransformDealloc(DCNSTransform* self)
{
	self->transform.~DKNSTransform();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCNSTransformRepr(DCNSTransform* self)
{
	const DKQuaternion& r = self->transform.orientation;
	const DKVector3& t = self->transform.position;
	DKString str = DKString::Format(
		"<%s object orientation:(%.3f, %.3f, %.3f, %.3f), position:(%.3f, %.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		r.x, r.y, r.z, r.w,
		t.x, t.y, t.z);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCNSTransformRichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKNSTransform* t1 = DCNSTransformToObject(obj1);
	DKNSTransform* t2 = DCNSTransformToObject(obj2);
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


static PyObject* DCNSTransformMatrix3(DCNSTransform* self, PyObject*)
{
	DKMatrix3 mat = self->transform.Matrix3();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCNSTransformMatrix4(DCNSTransform* self, PyObject*)
{
	DKMatrix4 mat = self->transform.Matrix4();
	return DCMatrix4FromObject(&mat);
}

static PyObject* DCNSTransformIdentity(DCNSTransform* self, PyObject*)
{
	self->transform.Identity();
	Py_RETURN_NONE;
}

static PyObject* DCNSTransformInverse(DCNSTransform* self, PyObject*)
{
	self->transform.Inverse();
	Py_RETURN_NONE;
}

static PyObject* DCNSTransformInterpolate(DCNSTransform* self, PyObject* args)
{
	DKNSTransform target;
	float t;

	if (!PyArg_ParseTuple(args, "O&f", &DCNSTransformConverter, &target, &t))
		return NULL;

	DKNSTransform ret = self->transform.Interpolate(target, t);
	return DCNSTransformFromObject(&ret);
}

static PyMethodDef methods[] = {
	{ "matrix3", (PyCFunction)&DCNSTransformMatrix3, METH_NOARGS },
	{ "matrix4", (PyCFunction)&DCNSTransformMatrix4, METH_NOARGS },
	{ "identity", (PyCFunction)&DCNSTransformIdentity, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCNSTransformInverse, METH_NOARGS },
	{ "interpolate", (PyCFunction)&DCNSTransformInterpolate, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

enum DCNSTransformAttr : uintptr_t
{
	DCNSTransformAttrOrientation,
	DCNSTransformAttrPosition,
};

static PyObject* DCNSTransformGetAttr(DCNSTransform* self, void* closure)
{
	const DKQuaternion& r = self->transform.orientation;
	const DKVector3& t = self->transform.position;

	DCNSTransformAttr attr = (DCNSTransformAttr)reinterpret_cast<uintptr_t>(closure);
	if (attr == DCNSTransformAttrOrientation)
		return Py_BuildValue("ffff", r.x, r.y, r.z, r.w);
	return Py_BuildValue("fff", t.x, t.y, t.z);
}

static int DCNSTransformSetAttr(DCNSTransform* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	const char* err = "";
	DCNSTransformAttr attr = (DCNSTransformAttr)reinterpret_cast<uintptr_t>(closure);
	if (attr == DCNSTransformAttrOrientation)
	{
		if (DCQuaternionConverter(value, &self->transform.orientation))
			return 0;
		err = "attribute must be Quaternion or tuple of four floating point numbers.";
	}
	else
	{
		if (DCVector3Converter(value, &self->transform.position))
			return 0;
		err = "attribute must be Vector3 or tuple of three floating point numbers.";
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, err);
	return -1;
}

static PyObject* DCNSTransformTuple(DCNSTransform* self, void*)
{
	const DKQuaternion& r = self->transform.orientation;
	const DKVector3& t = self->transform.position;

	return Py_BuildValue("fffffff",
		r.x, r.y, r.z, r.w,
		t.x, t.y, t.z);
}

static int DCNSTransformSetTuple(DCNSTransform* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKQuaternion& r = self->transform.orientation;
	DKVector3& t = self->transform.position;

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "fffffff",
		&r.x, &r.y, &r.z, &r.w,
		&t.x, &t.y, &t.z))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of seven floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "orientation", (getter)&DCNSTransformGetAttr, (setter)&DCNSTransformSetAttr, 0, (void*)DCNSTransformAttrOrientation },
	{ "position", (getter)&DCNSTransformGetAttr, (setter)&DCNSTransformSetAttr, 0, (void*)DCNSTransformAttrPosition },
	{ "tuple", (getter)&DCNSTransformTuple, (setter)&DCNSTransformSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyObject* DCNSTransformMultiply(PyObject* lhs, PyObject* rhs)
{
	DKNSTransform* p = DCNSTransformToObject(lhs);
	DKNSTransform t;
	if (p && DCNSTransformConverter(rhs, &t))
	{
		DKNSTransform res = (*p) * t;
		return DCNSTransformFromObject(&res);
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyNumberMethods numberMethods = {
	0,										/* nb_add */
	0,										/* nb_subtract */
	(binaryfunc)&DCNSTransformMultiply,		/* nb_multiply */
	0,										/* nb_remainder */
	0,										/* nb_divmod */
	0,										/* nb_power */
	0,										/* nb_negative */
	0,										/* nb_positive */
	0,										/* nb_absolute */
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
	0,										/* nb_floor_divide */
	0,										/* nb_true_divide */
	0,										/* nb_inplace_floor_divide */
	0,										/* nb_inplace_true_divide */
	0										/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".NSTransform",				/* tp_name */
	sizeof(DCNSTransform),							/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCNSTransformDealloc,				/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCNSTransformRepr,					/* tp_repr */
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
	"Non Scale Affine Transform",					/* tp_doc */
	0,												/* tp_traverse */
	0,												/* tp_clear */
	(richcmpfunc)&DCNSTransformRichCompare,			/* tp_richcompare */
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
	(initproc)&DCNSTransformInit,					/* tp_init */
	0,												/* tp_alloc */
	&DCNSTransformNew,								/* tp_new */
};

PyTypeObject* DCNSTransformTypeObject(void)
{
	return &objectType;
}

PyObject* DCNSTransformFromObject(DKNSTransform* transform)
{
	if (transform)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		DCNSTransform* self = (DCNSTransform*)PyObject_Call(tp, args, kwds);
		if (self)
		{
			self->transform = *transform;
		}
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKNSTransform* DCNSTransformToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCNSTransform*)obj)->transform;
	}
	return NULL;
}

int DCNSTransformConverter(PyObject* obj, DKNSTransform* p)
{
	DKNSTransform* trans = DCNSTransformToObject(obj);
	if (trans)
	{
		*p = *trans;
		return true;
	}
	DKQuaternion* quat = DCQuaternionToObject(obj);
	if (quat)
	{
		*p = *quat;
		return true;
	}
	DKMatrix3* mat = DCMatrix3ToObject(obj);
	if (mat)
	{
		*p = *mat;
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
		Py_ssize_t numItems = PyTuple_GET_SIZE(obj);

		if (numItems == 7)
		{
			DKQuaternion rotate;
			DKVector3 position(0, 0, 0);

			if (PyArg_ParseTuple(obj, "fffffff",
				&rotate.x, &rotate.y, &rotate.z, &rotate.w,
				&position.x, &position.y, &position.z))
			{
				*p = DKNSTransform(rotate, position);
				return true;
			}
		}
		else
		{
			PyObject* basis;
			DKVector3 position(0, 0, 0);
			if (PyArg_ParseTuple(obj, "O|O&", &basis, &DCVector3Converter, &position))
			{
				DKQuaternion quat;
				if (DCQuaternionConverter(basis, &quat))
				{
					*p = DKNSTransform(quat, position);
					return true;
				}
				PyErr_Clear();
				DKMatrix3 mat;
				if (DCMatrix3Converter(basis, &mat))
				{
					*p = DKNSTransform(mat, position);
					return true;
				}
			}
		}
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be one NSTransform or (orientation:Quaternion, position:Vector3) tuple.");
	return false;
}
