#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

struct DCUSTransform
{
	PyObject_HEAD
	DKUSTransform transform;
};

static PyObject* DCUSTransformNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCUSTransform* self = (DCUSTransform*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->transform) DKUSTransform();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCUSTransformInit(DCUSTransform *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	if (numArgs > 1 && DCUSTransformConverter(args, &self->transform))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCUSTransformConverter, &self->transform))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->transform.Identity();
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be (scale:Vector3, orientation:Quaternion, position:Vector3) tuple.");
	return -1;
}

static void DCUSTransformDealloc(DCUSTransform* self)
{
	self->transform.~DKUSTransform();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCUSTransformRepr(DCUSTransform* self)
{
	float s = self->transform.scale;
	const DKQuaternion& r = self->transform.orientation;
	const DKVector3& t = self->transform.position;
	DKString str = DKString::Format(
		"<%s object scale:%.3f, orientation:(%.3f, %.3f, %.3f, %.3f), position:(%.3f, %.3f, %.3f)>",
		Py_TYPE(self)->tp_name, s,
		r.x, r.y, r.z, r.w,
		t.x, t.y, t.z);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCUSTransformRichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKUSTransform* t1 = DCUSTransformToObject(obj1);
	DKUSTransform* t2 = DCUSTransformToObject(obj2);
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


static PyObject* DCUSTransformMatrix3(DCUSTransform* self, PyObject*)
{
	DKMatrix3 mat = self->transform.Matrix3();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCUSTransformMatrix4(DCUSTransform* self, PyObject*)
{
	DKMatrix4 mat = self->transform.Matrix4();
	return DCMatrix4FromObject(&mat);
}

static PyObject* DCUSTransformIdentity(DCUSTransform* self, PyObject*)
{
	self->transform.Identity();
	Py_RETURN_NONE;
}

static PyObject* DCUSTransformInverse(DCUSTransform* self, PyObject*)
{
	self->transform.Inverse();
	Py_RETURN_NONE;
}

static PyObject* DCUSTransformInterpolate(DCUSTransform* self, PyObject* args)
{
	DKUSTransform target;
	float t;

	if (!PyArg_ParseTuple(args, "O&f", &DCUSTransformConverter, &target, &t))
		return NULL;

	DKUSTransform ret = self->transform.Interpolate(target, t);
	return DCUSTransformFromObject(&ret);
}

static PyMethodDef methods[] = {
	{ "matrix3", (PyCFunction)&DCUSTransformMatrix3, METH_NOARGS },
	{ "matrix4", (PyCFunction)&DCUSTransformMatrix4, METH_NOARGS },
	{ "identity", (PyCFunction)&DCUSTransformIdentity, METH_NOARGS },
	{ "inverse", (PyCFunction)&DCUSTransformInverse, METH_NOARGS },
	{ "interpolate", (PyCFunction)&DCUSTransformInterpolate, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

enum DCUSTransformAttr : uintptr_t
{
	DCUSTransformAttrScale,
	DCUSTransformAttrOrientation,
	DCUSTransformAttrPosition,
};

static PyObject* DCUSTransformGetAttr(DCUSTransform* self, void* closure)
{
	float s = self->transform.scale;
	const DKQuaternion& r = self->transform.orientation;
	const DKVector3& t = self->transform.position;

	DCUSTransformAttr attr = (DCUSTransformAttr)reinterpret_cast<uintptr_t>(closure);
	if (attr == DCUSTransformAttrScale)
		return PyFloat_FromDouble(s);
	else if (attr == DCUSTransformAttrOrientation)
		return Py_BuildValue("ffff", r.x, r.y, r.z, r.w);
	return Py_BuildValue("fff", t.x, t.y, t.z);
}

static int DCUSTransformSetAttr(DCUSTransform* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	const char* err = "";
	DCUSTransformAttr attr = (DCUSTransformAttr)reinterpret_cast<uintptr_t>(closure);
	if (attr == DCUSTransformAttrScale)
	{
		double s = PyFloat_AsDouble(value);
		if (!PyErr_Occurred())
		{
			self->transform.scale = s;
			return 0;
		}
		err = "attribute must be floating point number.";
	}
	else if (attr == DCUSTransformAttrOrientation)
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

static PyObject* DCUSTransformTuple(DCUSTransform* self, void*)
{
	float s = self->transform.scale;
	const DKQuaternion& r = self->transform.orientation;
	const DKVector3& t = self->transform.position;

	return Py_BuildValue("ffffffff",
		s, 
		r.x, r.y, r.z, r.w,
		t.x, t.y, t.z);
}

static int DCUSTransformSetTuple(DCUSTransform* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	float& s = self->transform.scale;
	DKQuaternion& r = self->transform.orientation;
	DKVector3& t = self->transform.position;

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffffffff",
		&s,
		&r.x, &r.y, &r.z, &r.w,
		&t.x, &t.y, &t.z))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of eight floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "scale", (getter)&DCUSTransformGetAttr, (setter)&DCUSTransformSetAttr, 0, (void*)DCUSTransformAttrScale },
	{ "orientation", (getter)&DCUSTransformGetAttr, (setter)&DCUSTransformSetAttr, 0, (void*)DCUSTransformAttrOrientation },
	{ "position", (getter)&DCUSTransformGetAttr, (setter)&DCUSTransformSetAttr, 0, (void*)DCUSTransformAttrPosition },
	{ "tuple", (getter)&DCUSTransformTuple, (setter)&DCUSTransformSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyObject* DCUSTransformMultiply(PyObject* lhs, PyObject* rhs)
{
	DKUSTransform* p = DCUSTransformToObject(lhs);
	DKUSTransform t;
	if (p && DCUSTransformConverter(rhs, &t))
	{
		DKUSTransform res = (*p) * t;
		return DCUSTransformFromObject(&res);
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyNumberMethods numberMethods = {
	0,										/* nb_add */
	0,										/* nb_subtract */
	(binaryfunc)&DCUSTransformMultiply,		/* nb_multiply */
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
	PYDK_MODULE_NAME ".USTransform",				/* tp_name */
	sizeof(DCUSTransform),							/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCUSTransformDealloc,				/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCUSTransformRepr,					/* tp_repr */
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
	"Uniform Scale Affine Transform",				/* tp_doc */
	0,												/* tp_traverse */
	0,												/* tp_clear */
	(richcmpfunc)&DCUSTransformRichCompare,			/* tp_richcompare */
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
	(initproc)&DCUSTransformInit,					/* tp_init */
	0,												/* tp_alloc */
	&DCUSTransformNew,								/* tp_new */
};

PyTypeObject* DCUSTransformTypeObject(void)
{
	return &objectType;
}

PyObject* DCUSTransformFromObject(DKUSTransform* transform)
{
	if (transform)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		DCUSTransform* self = (DCUSTransform*)PyObject_Call(tp, args, kwds);
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

DKUSTransform* DCUSTransformToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCUSTransform*)obj)->transform;
	}
	return NULL;
}

int DCUSTransformConverter(PyObject* obj, DKUSTransform* p)
{
	DKUSTransform* trans = DCUSTransformToObject(obj);
	if (trans)
	{
		*p = *trans;
		return true;
	}
	if (obj && PyTuple_Check(obj))
	{
		Py_ssize_t numItems = PyTuple_GET_SIZE(obj);

		float scale;
		DKQuaternion rotate;
		DKVector3 translate(0,0,0);

		if (numItems == 8)
		{
			if (PyArg_ParseTuple(obj, "ffffffff",
				&scale,
				&rotate.x, &rotate.y, &rotate.z, &rotate.w,
				&translate.x, &translate.y, &translate.z))
			{
				*p = DKUSTransform(scale, rotate, translate);
				return true;
			}
		}
		else
		{
			if (PyArg_ParseTuple(obj, "fO&|O&", &scale,
				&DCQuaternionConverter, &rotate,
				&DCVector3Converter, &translate))
			{
				*p = DKUSTransform(scale, rotate, translate);
				return true;
			}
		}
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be one USTransform or (scale:float, orientation:Quaternion, position:Vector3) tuple.");
	return false;
}
