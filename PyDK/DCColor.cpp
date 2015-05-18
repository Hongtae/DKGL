#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCColor
{
	PyObject_HEAD
	DKColor color;
};

static PyObject* DCColorNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCColor* self = (DCColor*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->color) DKColor();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCColorInit(DCColor *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCColorConverter(args, &self->color))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCColorConverter, &self->color))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->color = DKColor(0.0f, 0.0f, 0.0f, 1.0f);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be four floating points or empty");
	return -1;
}

static void DCColorDealloc(DCColor* self)
{
	self->color.~DKColor();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCColorRepr(DCColor* self)
{
	DKString str = DKString::Format("<%s object (r:%.3f, g:%.3f, b:%.3f, a:%.3f)>",
		Py_TYPE(self)->tp_name, self->color.r, self->color.g, self->color.b, self->color.a);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCColorRichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKColor* vec1 = DCColorToObject(obj1);
	DKColor* vec2 = DCColorToObject(obj2);
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

static PyObject* DCColorARGB32Value(DCColor* self, PyObject*)
{
	DKColor::ARGB32 n = self->color.ARGB32Value();
	unsigned int a = n.a;
	unsigned int r = n.r;
	unsigned int g = n.g;
	unsigned int b = n.b;

	unsigned int value = (a << 24) | (r << 16) | (g << 8) | b;
	return PyLong_FromUnsignedLong(value);
}

static PyObject* DCColorRGBA32Value(DCColor* self, PyObject*)
{
	DKColor::RGBA32 n = self->color.RGBA32Value();
	unsigned int r = n.r;
	unsigned int g = n.g;
	unsigned int b = n.b;
	unsigned int a = n.a;

	unsigned int value = (r << 24) | (g << 16) | (b << 8) | a;
	return PyLong_FromUnsignedLong(value);
}

static PyMethodDef methods[] = {
	{ "argb32Value", (PyCFunction)&DCColorARGB32Value, METH_NOARGS },
	{ "rgba32Value", (PyCFunction)&DCColorRGBA32Value, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCColorGetAttr(DCColor* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->color);
	return PyFloat_FromDouble(*attr);
}

static int DCColorSetAttr(DCColor* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->color);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCColorRGB(DCColor* self, void*)
{
	return Py_BuildValue("fff", self->color.r, self->color.g, self->color.b);
}

static int DCColorSetRGB(DCColor* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "fff",
		&self->color.r, &self->color.g, &self->color.b))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of three floating point numbers (r, g, b).");
	return -1;
}

static PyObject* DCColorTuple(DCColor* self, void*)
{
	return Py_BuildValue("ffff", self->color.r, self->color.g, self->color.b, self->color.a);
}

static int DCColorSetTuple(DCColor* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffff",
		&self->color.r, &self->color.g, &self->color.b, &self->color.a))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of four floating point numbers (r, g, b, a).");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "r", (getter)&DCColorGetAttr, (setter)&DCColorSetAttr, 0, reinterpret_cast<void*>(offsetof(DKColor, r)) },
	{ "g", (getter)&DCColorGetAttr, (setter)&DCColorSetAttr, 0, reinterpret_cast<void*>(offsetof(DKColor, g)) },
	{ "b", (getter)&DCColorGetAttr, (setter)&DCColorSetAttr, 0, reinterpret_cast<void*>(offsetof(DKColor, b)) },
	{ "a", (getter)&DCColorGetAttr, (setter)&DCColorSetAttr, 0, reinterpret_cast<void*>(offsetof(DKColor, a)) },
	{ "rgb", (getter)&DCColorRGB, (setter)&DCColorSetRGB, 0, 0 },
	{ "tuple", (getter)&DCColorTuple, (setter)&DCColorSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCColorUnaryOperand(PyObject* obj, bool(*fn)(const DKColor&, DKColor&))
{
	DKColor* v = DCColorToObject(obj);
	if (v)
	{
		DKColor res;
		if (fn(*v, res))
			return DCColorFromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCColorBinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKColor&, const DKColor&, DKColor&))
{
	DKColor* p = DCColorToObject(lhs);
	DKColor v;
	if (p && DCColorConverter(rhs, &v))
	{
		DKColor res;
		if (fn(*p, v, res))
			return DCColorFromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCColorBinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKColor&, double, DKColor&))
{
	DKColor* vec = DCColorToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKColor res;
		if (fn(*vec, d, res))
			return DCColorFromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCColorAdd(PyObject* lhs, PyObject* rhs)
{
	return DCColorBinaryOperand(lhs, rhs, [](const DKColor& v1, const DKColor& v2, DKColor& r)
	{
		r = v1 + v2;
		return true;
	});
}

static PyObject* DCColorSubtract(PyObject* lhs, PyObject* rhs)
{
	return DCColorBinaryOperand(lhs, rhs, [](const DKColor& v1, const DKColor& v2, DKColor& r)
	{
		r = v1 - v2;
		return true;
	});
}

static PyObject* DCColorMultiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCColorTypeObject()))
		return DCColorBinaryOperand(lhs, rhs, [](const DKColor& v1, const DKColor& v2, DKColor& r)
	{
		r = v1 * v2;
		return true;
	});
	return DCColorBinaryOperand(lhs, rhs, [](const DKColor& v, double d, DKColor& r)
	{
		r = v * d;
		return true;
	});
}

static PyObject* DCColorNegative(PyObject* obj)
{
	return DCColorUnaryOperand(obj, [](const DKColor& v, DKColor& r)
	{
		r = DKColor(-v.r, -v.g, -v.b, -v.a);
		return true;
	});
}

static PyObject* DCColorPositive(PyObject* obj)
{
	return DCColorUnaryOperand(obj, [](const DKColor& v, DKColor& r)
	{
		r = v;
		return true;
	});
}

static PyObject* DCColorAbsolute(PyObject* obj)
{
	return DCColorUnaryOperand(obj, [](const DKColor& v, DKColor& ret)
	{
		float r = v.r < 0 ? -v.r : v.r;
		float g = v.g < 0 ? -v.g : v.g;
		float b = v.b < 0 ? -v.b : v.b;
		float a = v.a < 0 ? -v.a : v.a;
		ret = DKColor(r, g, b, a);
		return true;
	});
}

static PyObject* DCColorFloorDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector4TypeObject()))
		return DCColorBinaryOperand(lhs, rhs, [](const DKColor& v1, const DKColor& v2, DKColor& r)
	{
		if (v2.r != 0.0f && v2.g != 0.0f && v2.b != 0.0f && v2.a != 0.0f)
		{
			r = v1 / v2;
			r.r = floor(r.r);
			r.g = floor(r.g);
			r.b = floor(r.b);
			r.a = floor(r.a);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	return DCColorBinaryOperand(lhs, rhs, [](const DKColor& v, double d, DKColor& r)
	{
		if (d != 0.0)
		{
			r = v / d;
			r.r = floor(r.r);
			r.g = floor(r.g);
			r.b = floor(r.b);
			r.a = floor(r.a);
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyObject* DCColorTrueDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCVector4TypeObject()))
		return DCColorBinaryOperand(lhs, rhs, [](const DKColor& v1, const DKColor& v2, DKColor& r)
	{
		if (v2.r != 0.0f && v2.g != 0.0f && v2.b != 0.0f && v2.a != 0.0f)
		{
			r = v1 / v2;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
	return DCColorBinaryOperand(lhs, rhs, [](const DKColor& v, double d, DKColor& r)
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
	(binaryfunc)&DCColorAdd,			/* nb_add */
	(binaryfunc)&DCColorSubtract,		/* nb_subtract */
	(binaryfunc)&DCColorMultiply,		/* nb_multiply */
	0,									/* nb_remainder */
	0,									/* nb_divmod */
	0,									/* nb_power */
	(unaryfunc)&DCColorNegative,		/* nb_negative */
	(unaryfunc)&DCColorPositive,		/* nb_positive */
	(unaryfunc)&DCColorAbsolute,		/* nb_absolute */
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
	(binaryfunc)&DCColorFloorDivide,	/* nb_floor_divide */
	(binaryfunc)&DCColorTrueDivide,	/* nb_true_divide */
	0,									/* nb_inplace_floor_divide */
	0,									/* nb_inplace_true_divide */
	0									/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Color",						/* tp_name */
	sizeof(DCColor),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCColorDealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCColorRepr,							/* tp_repr */
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
	(richcmpfunc)&DCColorRichCompare,				/* tp_richcompare */
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
	(initproc)&DCColorInit,							/* tp_init */
	0,												/* tp_alloc */
	&DCColorNew,									/* tp_new */
};

PyTypeObject* DCColorTypeObject(void)
{
	return &objectType;
}

PyObject* DCColorFromObject(DKColor* color)
{
	if (color)
	{
		PyObject* args = Py_BuildValue("ffff", color->r, color->g, color->b, color->a);
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

DKColor* DCColorToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCColor*)obj)->color;
	}
	return NULL;
}

int DCColorConverter(PyObject* obj, DKColor* p)
{
	DKColor* tmp = DCColorToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	DKVector4* vec = DCVector4ToObject(obj);
	if (vec)
	{
		*p = DKColor(vec->x, vec->y, vec->z, vec->w);
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		float a = 1.0f;

		if (PyArg_ParseTuple(obj, "fff|f", &r, &g, &b, &a))
		{
			*p = DKColor(r, g, b, a);
			return true;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Color object or four floats tuple.");
	return false;
}
