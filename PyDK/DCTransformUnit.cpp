#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKGL;
using namespace DKGL;

struct DCTransformUnit
{
	PyObject_HEAD
	DKTransformUnit transformUnit;
};

static PyObject* DCTransformUnitNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCTransformUnit* self = (DCTransformUnit*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->transformUnit) DKTransformUnit();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCTransformUnitInit(DCTransformUnit *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);

	if (numArgs > 1 && DCTransformUnitConverter(args, &self->transformUnit))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCTransformUnitConverter, &self->transformUnit))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->transformUnit.Identity();
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be (scale:Vector3, rotate:Quaternion, translate:Vector3) tuple.");
	return -1;
}

static void DCTransformUnitDealloc(DCTransformUnit* self)
{
	self->transformUnit.~DKTransformUnit();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCTransformUnitRepr(DCTransformUnit* self)
{
	const DKVector3& s = self->transformUnit.scale;
	const DKQuaternion& r = self->transformUnit.rotation;
	const DKVector3& t = self->transformUnit.translation;
	DKString str = DKString::Format(
		"<%s object scale:(%.3f, %.3f, %.3f), rotate:(%.3f, %.3f, %.3f, %.3f), translate:(%.3f, %.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		s.x, s.y, s.z,
		r.x, r.y, r.z, r.w,
		t.x, t.y, t.z);

	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCTransformUnitRichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKTransformUnit* t1 = DCTransformUnitToObject(obj1);
	DKTransformUnit* t2 = DCTransformUnitToObject(obj2);
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


static PyObject* DCTransformUnitMatrix3(DCTransformUnit* self, PyObject*)
{
	DKMatrix3 mat = self->transformUnit.Matrix3();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCTransformUnitMatrix4(DCTransformUnit* self, PyObject*)
{
	DKMatrix4 mat = self->transformUnit.Matrix4();
	return DCMatrix4FromObject(&mat);
}

static PyObject* DCTransformUnitIdentity(DCTransformUnit* self, PyObject*)
{
	self->transformUnit.Identity();
	Py_RETURN_NONE;
}

static PyObject* DCTransformUnitInterpolate(DCTransformUnit* self, PyObject* args)
{
	DKTransformUnit target;
	float t;

	if (!PyArg_ParseTuple(args, "O&f", &DCTransformUnitConverter, &target, &t))
		return NULL;

	DKTransformUnit ret = self->transformUnit.Interpolate(target, t);
	return DCTransformUnitFromObject(&ret);
}

static PyMethodDef methods[] = {
	{ "matrix3", (PyCFunction)&DCTransformUnitMatrix3, METH_NOARGS },
	{ "matrix4", (PyCFunction)&DCTransformUnitMatrix4, METH_NOARGS },
	{ "identity", (PyCFunction)&DCTransformUnitIdentity, METH_NOARGS },
	{ "interpolate", (PyCFunction)&DCTransformUnitInterpolate, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

enum DCTransformUnitAttr : uintptr_t
{
	DCTransformUnitAttrScale,
	DCTransformUnitAttrRotation,
	DCTransformUnitAttrTranslation,
};

static PyObject* DCTransformUnitGetAttr(DCTransformUnit* self, void* closure)
{
	const DKVector3& s = self->transformUnit.scale;
	const DKQuaternion& r = self->transformUnit.rotation;
	const DKVector3& t = self->transformUnit.translation;

	DCTransformUnitAttr attr = (DCTransformUnitAttr)reinterpret_cast<uintptr_t>(closure);
	if (attr == DCTransformUnitAttrScale)
		return Py_BuildValue("fff", s.x, s.y, s.z);
	else if (attr == DCTransformUnitAttrRotation)
		return Py_BuildValue("ffff", r.x, r.y, r.z, r.w);
	return Py_BuildValue("fff", t.x, t.y, t.z);
}

static int DCTransformUnitSetAttr(DCTransformUnit* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	const char* err = "";
	DCTransformUnitAttr attr = (DCTransformUnitAttr)reinterpret_cast<uintptr_t>(closure);
	if (attr == DCTransformUnitAttrScale)
	{
		if (DCVector3Converter(value, &self->transformUnit.scale))
			return 0;
		err = "attribute must be Vector3 or tuple of three floating point numbers.";
	}
	else if (attr == DCTransformUnitAttrRotation)
	{
		if (DCQuaternionConverter(value, &self->transformUnit.rotation))
			return 0;
		err = "attribute must be Quaternion or tuple of four floating point numbers.";
	}
	else
	{
		if (DCVector3Converter(value, &self->transformUnit.translation))
			return 0;
		err = "attribute must be Vector3 or tuple of three floating point numbers.";
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, err);
	return -1;
}

static PyObject* DCTransformUnitTuple(DCTransformUnit* self, void*)
{
	const DKVector3& s = self->transformUnit.scale;
	const DKQuaternion& r = self->transformUnit.rotation;
	const DKVector3& t = self->transformUnit.translation;

	return Py_BuildValue("ffffffffff",
		s.x, s.y, s.z,
		r.x, r.y, r.z, r.w,
		t.x, t.y, t.z);
}

static int DCTransformUnitSetTuple(DCTransformUnit* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKVector3& s = self->transformUnit.scale;
	DKQuaternion& r = self->transformUnit.rotation;
	DKVector3& t = self->transformUnit.translation;

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffffffffff",
		&s.x, &s.y, &s.z,
		&r.x, &r.y, &r.z, &r.w,
		&t.x, &t.y, &t.z))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of ten floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "scale", (getter)&DCTransformUnitGetAttr, (setter)&DCTransformUnitSetAttr, 0, (void*)DCTransformUnitAttrScale },
	{ "rotation", (getter)&DCTransformUnitGetAttr, (setter)&DCTransformUnitSetAttr, 0, (void*)DCTransformUnitAttrRotation },
	{ "translation", (getter)&DCTransformUnitGetAttr, (setter)&DCTransformUnitSetAttr, 0, (void*)DCTransformUnitAttrTranslation },
	{ "tuple", (getter)&DCTransformUnitTuple, (setter)&DCTransformUnitSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".TransformUnit",				/* tp_name */
	sizeof(DCTransformUnit),						/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCTransformUnitDealloc,			/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCTransformUnitRepr,					/* tp_repr */
	0,												/* tp_as_number */
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
	(richcmpfunc)&DCTransformUnitRichCompare,		/* tp_richcompare */
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
	(initproc)&DCTransformUnitInit,					/* tp_init */
	0,												/* tp_alloc */
	&DCTransformUnitNew,							/* tp_new */
};

PyTypeObject* DCTransformUnitTypeObject(void)
{
	return &objectType;
}

PyObject* DCTransformUnitFromObject(DKTransformUnit* transformUnit)
{
	if (transformUnit)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();
		PyObject* tp = (PyObject*)DCObjectDefaultClass(&objectType);
		DCTransformUnit* self = (DCTransformUnit*)PyObject_Call(tp, args, kwds);
		if (self)
		{
			self->transformUnit = *transformUnit;
		}
		Py_XDECREF(tp);
		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKTransformUnit* DCTransformUnitToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCTransformUnit*)obj)->transformUnit;
	}
	return NULL;
}

int DCTransformUnitConverter(PyObject* obj, DKTransformUnit* p)
{
	DKTransformUnit* trans = DCTransformUnitToObject(obj);
	if (trans)
	{
		*p = *trans;
		return true;
	}
	if (obj && PyTuple_Check(obj))
	{
		Py_ssize_t numItems = PyTuple_GET_SIZE(obj);

		DKVector3 scale;
		DKQuaternion rotate;
		DKVector3 translate;

		if (numItems == 10)
		{
			if (PyArg_ParseTuple(obj, "ffffffffff",
				&scale.x, &scale.y, &scale.z,
				&rotate.x, &rotate.y, &rotate.z, &rotate.w,
				&translate.x, &translate.y, &translate.z))
			{
				*p = DKTransformUnit(scale, rotate, translate);
				return true;
			}
		}
		else
		{
			if (PyArg_ParseTuple(obj, "O&O&O&",
				&DCVector3Converter, &scale,
				&DCQuaternionConverter, &rotate,
				&DCVector3Converter, &translate))
			{
				*p = DKTransformUnit(scale, rotate, translate);
				return true;
			}
		}
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be one TransformUnit or (scale:Vector3, rotate:Quaternion, translate:Vector3) tuple.");
	return false;
}
