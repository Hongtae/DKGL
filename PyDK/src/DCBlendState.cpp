#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCBlendState
{
	PyObject_HEAD
	DKBlendState blendState;
};

static PyObject* DCBlendStateNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCBlendState* self = (DCBlendState*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->blendState) DKBlendState();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCBlendStateInit(DCBlendState *self, PyObject *args, PyObject *kwds)
{
	return 0;
}

static void DCBlendStateDealloc(DCBlendState* self)
{
	self->blendState.~DKBlendState();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

template <int index>
static PyObject* DCBlendStateGetColorWrite(DCBlendState* self, void*)
{
	static_assert(index >= 0 && index < 4, "index out of range");

	bool val[] = {
		self->blendState.colorWriteR,
		self->blendState.colorWriteG,
		self->blendState.colorWriteB,
		self->blendState.colorWriteA };
	return PyBool_FromLong(val[index]);
}

template <int index>
static int DCBlendStateSetColorWrite(DCBlendState* self, PyObject* value, void*)
{
	static_assert(index >= 0 && index < 4, "index out of range");

	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	int b = PyObject_IsTrue(value);
	if (b < 0)
	{
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}

	switch (index)
	{
	case 0:	self->blendState.colorWriteR = b > 0 ? 1 : 0; break;
	case 1:	self->blendState.colorWriteG = b > 0 ? 1 : 0; break;
	case 2:	self->blendState.colorWriteB = b > 0 ? 1 : 0; break;
	case 3:	self->blendState.colorWriteA = b > 0 ? 1 : 0; break;
	}
	return 0;
}

template <typename T> static T& DCBlendStateGetOffset(DCBlendState* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	T* attr = reinterpret_cast<T*>(offset + (uintptr_t)&self->blendState);
	return attr[0];
}


template <typename T> struct BlendStateAttr;
template <> struct BlendStateAttr<DKBlendState::BlendMode>
{
	enum { MaxValue = DKBlendState::BlendModeSrcAlphaSaturate };
};
template <> struct BlendStateAttr<DKBlendState::BlendFunc>
{
	enum { MaxValue = DKBlendState::BlendFuncMax };
};

template <typename T>
static PyObject* DCBlendStateGetIntAttr(DCBlendState* self, void* closure)
{
	return PyLong_FromLong((long)DCBlendStateGetOffset<T>(self, closure));
}

template <typename T>
static int DCBlendStateSetIntAttr(DCBlendState* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	long v = PyLong_AsLong(value);
	if (!PyErr_Occurred())
	{
		if (v >= 0 && v <= BlendStateAttr<T>::MaxValue)
		{
			T& mode = DCBlendStateGetOffset<T>(self, closure);
			mode = (T)v;
			return 0;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Blend State constant value.");
	return -1;
}

static PyObject* DCBlendStateGetConstantColor(DCBlendState* self, void*)
{
	DKColor color = self->blendState.constantColor;
	return Py_BuildValue("ffff", color.r, color.g, color.b, color.a);
}

static int DCBlendStateSetConstantColor(DCBlendState* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);
	DKColor c;
	if (DCColorConverter(value, &c))
	{
		self->blendState.constantColor = c.RGBA32Value();
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must Color object.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "colorWriteR", (getter)&DCBlendStateGetColorWrite<0>, (setter)&DCBlendStateSetColorWrite<0>, 0, 0 },
	{ "colorWriteG", (getter)&DCBlendStateGetColorWrite<1>, (setter)&DCBlendStateSetColorWrite<1>, 0, 0 },
	{ "colorWriteB", (getter)&DCBlendStateGetColorWrite<2>, (setter)&DCBlendStateSetColorWrite<2>, 0, 0 },
	{ "colorWriteA", (getter)&DCBlendStateGetColorWrite<3>, (setter)&DCBlendStateSetColorWrite<3>, 0, 0 },
	{ "srcBlendRGB", (getter)&DCBlendStateGetIntAttr<DKBlendState::BlendMode>, (setter)&DCBlendStateSetIntAttr<DKBlendState::BlendMode>, 0, (void*)offsetof(DKBlendState, srcBlendRGB) },
	{ "srcBlendAlpha", (getter)&DCBlendStateGetIntAttr<DKBlendState::BlendMode>, (setter)&DCBlendStateSetIntAttr<DKBlendState::BlendMode>, 0, (void*)offsetof(DKBlendState, srcBlendAlpha) },
	{ "dstBlendRGB", (getter)&DCBlendStateGetIntAttr<DKBlendState::BlendMode>, (setter)&DCBlendStateSetIntAttr<DKBlendState::BlendMode>, 0, (void*)offsetof(DKBlendState, dstBlendRGB) },
	{ "dstBlendAlpha", (getter)&DCBlendStateGetIntAttr<DKBlendState::BlendMode>, (setter)&DCBlendStateSetIntAttr<DKBlendState::BlendMode>, 0, (void*)offsetof(DKBlendState, dstBlendAlpha) },
	{ "blendFuncRGB", (getter)&DCBlendStateGetIntAttr<DKBlendState::BlendFunc>, (setter)&DCBlendStateSetIntAttr<DKBlendState::BlendFunc>, 0, (void*)offsetof(DKBlendState, blendFuncRGB) },
	{ "blendFuncAlpha", (getter)&DCBlendStateGetIntAttr<DKBlendState::BlendFunc>, (setter)&DCBlendStateSetIntAttr<DKBlendState::BlendFunc>, 0, (void*)offsetof(DKBlendState, blendFuncAlpha) },
	{ "constantColor", (getter)&DCBlendStateGetConstantColor, (setter)&DCBlendStateSetConstantColor, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".BlendState",					/* tp_name */
	sizeof(DCBlendState),							/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCBlendStateDealloc,				/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	0,												/* tp_repr */
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
	0,												/* tp_richcompare */
	0,												/* tp_weaklistoffset */
	0,												/* tp_iter */
	0,												/* tp_iternext */
	0,												/* tp_methods */
	0,												/* tp_members */
	getsets,										/* tp_getset */
	0,												/* tp_base */
	0,												/* tp_dict */
	0,												/* tp_descr_get */
	0,												/* tp_descr_set */
	0,												/* tp_dictoffset */
	(initproc)&DCBlendStateInit,					/* tp_init */
	0,												/* tp_alloc */
	&DCBlendStateNew,								/* tp_new */
};

PyTypeObject* DCBlendStateTypeObject(void)
{
	return &objectType;
}

PyObject* DCBlendStateFromObject(DKBlendState* blendState)
{
	if (blendState)
	{
		PyObject* args = PyTuple_New(0);
		PyObject* kwds = PyDict_New();

		DCBlendState* self = (DCBlendState*)DCObjectCreateDefaultClass(&objectType, args, kwds);
		if (self)
		{
			self->blendState = *blendState;
			Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
		}

		Py_XDECREF(args);
		Py_XDECREF(kwds);

		return (PyObject*)self;
	}
	Py_RETURN_NONE;
}

DKBlendState* DCBlendStateToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCBlendState*)obj)->blendState;
	}
	return NULL;
}
