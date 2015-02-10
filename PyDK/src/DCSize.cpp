#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

struct DCSize
{
	PyObject_HEAD
	DKSize size;
};

static PyObject* DCSizeNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCSize* self = (DCSize*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->size) DKSize();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCSizeInit(DCSize *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCSizeConverter(args, &self->size))
	{
		return 0;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCSizeConverter, &self->size))
	{
		return 0;
	}
	else if (numArgs == 0)
	{
		self->size = DKSize(0, 0);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be two floating point (width,height) or empty");
	return -1;
}

static void DCSizeDealloc(DCSize* self)
{
	self->size.~DKSize();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCSizeRepr(DCSize* self)
{
	DKString str = DKString::Format("<%s object (width:%.3f, height:%.3f)>",
		Py_TYPE(self)->tp_name, self->size.width, self->size.height);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCSizeRichCompare(PyObject *obj1, PyObject *obj2, int op)
{
	DKSize* s1 = DCSizeToObject(obj1);
	DKSize* s2 = DCSizeToObject(obj2);
	if (s1 && s2)
	{
		if (op == Py_EQ)
		{
			if (*s1 == *s2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
		else if (op == Py_NE)
		{
			if (*s1 != *s2) { Py_RETURN_TRUE; }
			else { Py_RETURN_FALSE; }
		}
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCSizeVector(DCSize* self)
{
	DKVector2 vec = self->size.Vector();
	return DCVector2FromObject(&vec);
}

static PyMethodDef methods[] = {
	{ "vector", (PyCFunction)&DCSizeVector, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCSizeGetAttr(DCSize* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->size);
	return PyFloat_FromDouble(*attr);
}

static int DCSizeSetAttr(DCSize* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->size);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCSizeTuple(DCSize* self, void*)
{
	return Py_BuildValue("ff", self->size.width, self->size.height);
}

static int DCSizeSetTuple(DCSize* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ff", &self->size.width, &self->size.height))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of two floating point numbers (width, height).");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "width", (getter)&DCSizeGetAttr, (setter)&DCSizeSetAttr, 0, reinterpret_cast<void*>(offsetof(DKSize, width)) },
	{ "height", (getter)&DCSizeGetAttr, (setter)&DCSizeSetAttr, 0, reinterpret_cast<void*>(offsetof(DKSize, height)) },
	{ "tuple", (getter)&DCSizeTuple, (setter)&DCSizeSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static inline PyObject* DCSizeUnaryOperand(PyObject* obj, bool(*fn)(const DKSize&, DKSize&))
{
	DKSize* s = DCSizeToObject(obj);
	if (s)
	{
		DKSize res;
		if (fn(*s, res))
			return DCSizeFromObject(&res);
		return NULL;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCSizeBinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKSize&, const DKSize&, DKSize&))
{
	DKSize* p = DCSizeToObject(lhs);
	DKSize sz;
	if (p && DCSizeConverter(rhs, &sz))
	{
		DKSize res;
		if (fn(*p, sz, res))
			return DCSizeFromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static inline PyObject* DCSizeBinaryOperand(PyObject* lhs, PyObject* rhs, bool(*fn)(const DKSize&, double, DKSize&))
{
	DKSize* s = DCSizeToObject(lhs);
	double d = PyFloat_AsDouble(rhs);
	if (!PyErr_Occurred())
	{
		DKSize res;
		if (fn(*s, d, res))
			return DCSizeFromObject(&res);
		return NULL;
	}
	PyErr_Clear();
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* DCSizeAdd(PyObject* lhs, PyObject* rhs)
{
	return DCSizeBinaryOperand(lhs, rhs, [](const DKSize& s1, const DKSize& s2, DKSize& r)
	{
		r = s1 + s2;
		return true;
	});
}

static PyObject* DCSizeSubtract(PyObject* lhs, PyObject* rhs)
{
	return DCSizeBinaryOperand(lhs, rhs, [](const DKSize& s1, const DKSize& s2, DKSize& r)
	{
		r = s1 - s2;
		return true;
	});
}

static PyObject* DCSizeMultiply(PyObject* lhs, PyObject* rhs)
{
	if (PyObject_TypeCheck(rhs, DCSizeTypeObject()))
		return DCSizeBinaryOperand(lhs, rhs, [](const DKSize& s1, const DKSize& s2, DKSize& r)
	{
		r = s1 * s2;
		return true;
	});
	return DCSizeBinaryOperand(lhs, rhs, [](const DKSize& s, double d, DKSize& r)
	{
		r = s * d;
		return true;
	});
}

static PyObject* DCSizeNegative(PyObject* obj)
{
	return DCSizeUnaryOperand(obj, [](const DKSize& s, DKSize& r)
	{
		r = DKSize(-s.width, -s.height);
		return true;
	});
}

static PyObject* DCSizePositive(PyObject* obj)
{
	return DCSizeUnaryOperand(obj, [](const DKSize& s, DKSize& r)
	{
		r = s;
		return true;
	});
}

static PyObject* DCSizeAbsolute(PyObject* obj)
{
	return DCSizeUnaryOperand(obj, [](const DKSize& s, DKSize& r)
	{
		r = DKSize((s.width < 0 ? -s.width : s.width), (s.height < 0 ? -s.height : s.height));
		return true;
	});
}

static PyObject* DCSizeFloorDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCSizeTypeObject()))
		return DCSizeBinaryOperand(lhs, rhs, [](const DKSize& s1, const DKSize& s2, DKSize& r)
   {
	   if (s2.width != 0 && s2.height != 0)
	   {
		   r = s1 / s2;
		   r.width = floor(r.width);
		   r.height = floor(r.height);
		   return true;
	   }
	   PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
	   return false;
   });
	
	return DCSizeBinaryOperand(lhs, rhs, [](const DKSize& s, double d, DKSize& r)
   {
	   if (d != 0.0)
	   {
		   r = s / d;
		   r.width = floor(r.width);
		   r.height = floor(r.height);
		   return true;
	   }
	   PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
	   return false;
   });
}

static PyObject* DCSizeTrueDivide(PyObject *lhs, PyObject *rhs)
{
	if (PyObject_TypeCheck(rhs, DCSizeTypeObject()))
		return DCSizeBinaryOperand(lhs, rhs, [](const DKSize& s1, const DKSize& s2, DKSize& r)
	{
		if (s2.width != 0 && s2.height != 0)
		{
			r = s1 / s2;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});

	return DCSizeBinaryOperand(lhs, rhs, [](const DKSize& s, double d, DKSize& r)
	{
		if (d != 0.0)
		{
			r = s / d;
			return true;
		}
		PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
		return false;
	});
}

static PyNumberMethods numberMethods = {
	(binaryfunc)&DCSizeAdd,				/* nb_add */
	(binaryfunc)&DCSizeSubtract,		/* nb_subtract */
	(binaryfunc)&DCSizeMultiply,		/* nb_multiply */
	0,									/* nb_remainder */
	0,									/* nb_divmod */
	0,									/* nb_power */
	(unaryfunc)&DCSizeNegative,			/* nb_negative */
	(unaryfunc)&DCSizePositive,			/* nb_positive */
	(unaryfunc)&DCSizeAbsolute,			/* nb_absolute */
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
	(binaryfunc)&DCSizeFloorDivide,		/* nb_floor_divide */
	(binaryfunc)&DCSizeTrueDivide,		/* nb_true_divide */
	0,									/* nb_inplace_floor_divide */
	0,									/* nb_inplace_true_divide */
	0									/* nb_index */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Size",						/* tp_name */
	sizeof(DCSize),									/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCSizeDealloc,						/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCSizeRepr,							/* tp_repr */
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
	(richcmpfunc)&DCSizeRichCompare,				/* tp_richcompare */
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
	(initproc)&DCSizeInit,							/* tp_init */
	0,												/* tp_alloc */
	&DCSizeNew,										/* tp_new */
};

PyTypeObject* DCSizeTypeObject(void)
{
	return &objectType;
}

PyObject* DCSizeFromObject(DKSize* size)
{
	if (size)
	{
		PyObject* args = Py_BuildValue("ff", size->width, size->height);
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

DKSize* DCSizeToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCSize*)obj)->size;
	}
	return NULL;
}

int DCSizeConverter(PyObject* obj, DKSize* p)
{
	DKSize* tmp = DCSizeToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	DKVector2* vec = DCVector2ToObject(obj);
	if (vec)
	{
		*p = *vec;
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		if (PyArg_ParseTuple(obj, "ff", &p->width, &p->height))
			return true;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Size object or two floats tuple.");
	return false;
}
