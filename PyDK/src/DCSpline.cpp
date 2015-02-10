#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCSpline
{
	PyObject_HEAD
	DKSpline spline;
};

static PyObject* DCSplineNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCSpline* self = (DCSpline*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->spline) DKSpline();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCSplineInit(DCSpline *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCSplineConverter(args, &self->spline))
		return 0;
	if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCSplineConverter, &self->spline))
		return 0;
	if (numArgs == 0)
	{
		self->spline = DKSpline(0, 0, 0, 0);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be four floating points or empty");
	return -1;
}

static void DCSplineDealloc(DCSpline* self)
{
	self->spline.~DKSpline();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCSplineRepr(DCSpline* self)
{
	DKString str = DKString::Format("<%s object (%.3f, %.3f, %.3f, %.3f)>",
		Py_TYPE(self)->tp_name,
		self->spline.point0, self->spline.point1, self->spline.point2, self->spline.point3);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCSplineInterpolate(DCSpline* self, PyObject* args)
{
	float t;
	int type;
	if (!PyArg_ParseTuple(args, "fi", &t, &type))
		return NULL;

	if (type < 0 || type > DKSpline::Bezier)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range.");
		return NULL;
	}

	return PyFloat_FromDouble(self->spline.Interpolate(t, (DKSpline::Type)type));
}

static PyObject* DCSplineSplineInterpolate(PyObject*, PyObject* args)
{
	float t;
	float val[4];
	int type;
	if (!PyArg_ParseTuple(args, "fffffi", &val[0], &val[1], &val[2], &val[3], &t, &type))
		return NULL;

	if (type < 0 || type > DKSpline::Bezier)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range.");
		return NULL;
	}

	return PyFloat_FromDouble(DKSpline(val[0], val[1], val[2], val[3]).Interpolate(t, (DKSpline::Type)type));
}

static PyMethodDef methods[] = {
	{ "interpolate", (PyCFunction)&DCSplineInterpolate, METH_VARARGS },
	{ "splineInterpolate", (PyCFunction)&DCSplineSplineInterpolate, METH_VARARGS | METH_STATIC },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCSplineGetAttr(DCSpline* self, void* closure)
{
	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->spline);
	return PyFloat_FromDouble(*attr);
}

static int DCSplineSetAttr(DCSpline* self, PyObject* value, void* closure)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	uintptr_t offset = reinterpret_cast<uintptr_t>(closure);
	float* attr = reinterpret_cast<float*>(offset + (uintptr_t)&self->spline);

	double val = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
		return -1;

	*attr = val;
	return 0;
}

static PyObject* DCSplineTuple(DCSpline* self, void*)
{
	return Py_BuildValue("ffff", self->spline.point0, self->spline.point1, self->spline.point2, self->spline.point3);
}

static int DCSplineSetTuple(DCSpline* self, PyObject* value, void*)
{
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PyTuple_Check(value) && PyArg_ParseTuple(value, "ffff",
		&self->spline.point0, &self->spline.point1, &self->spline.point2, &self->spline.point3))
		return 0;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be tuple of four floating point numbers.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "p0", (getter)&DCSplineGetAttr, (setter)&DCSplineSetAttr, 0, reinterpret_cast<void*>(offsetof(DKSpline, point0)) },
	{ "p1", (getter)&DCSplineGetAttr, (setter)&DCSplineSetAttr, 0, reinterpret_cast<void*>(offsetof(DKSpline, point1)) },
	{ "p2", (getter)&DCSplineGetAttr, (setter)&DCSplineSetAttr, 0, reinterpret_cast<void*>(offsetof(DKSpline, point2)) },
	{ "p3", (getter)&DCSplineGetAttr, (setter)&DCSplineSetAttr, 0, reinterpret_cast<void*>(offsetof(DKSpline, point3)) },
	{ "tuple", (getter)&DCSplineTuple, (setter)&DCSplineSetTuple, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Spline",						/* tp_name */
	sizeof(DCSpline),									/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCSplineDealloc,						/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCSplineRepr,							/* tp_repr */
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
	methods,										/* tp_methods */
	0,												/* tp_members */
	getsets,										/* tp_getset */
	0,												/* tp_base */
	0,												/* tp_dict */
	0,												/* tp_descr_get */
	0,												/* tp_descr_set */
	0,												/* tp_dictoffset */
	(initproc)&DCSplineInit,						/* tp_init */
	0,												/* tp_alloc */
	&DCSplineNew,									/* tp_new */
};

PyTypeObject* DCSplineTypeObject(void)
{
	return &objectType;
}

PyObject* DCSplineFromObject(DKSpline* spline)
{
	if (spline)
	{
		PyObject* args = Py_BuildValue("ffff", spline->point0, spline->point1, spline->point2, spline->point3);
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

DKSpline* DCSplineToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return &((DCSpline*)obj)->spline;
	}
	return NULL;
}

int DCSplineConverter(PyObject* obj, DKSpline* p)
{
	DKSpline* tmp = DCSplineToObject(obj);
	if (tmp)
	{
		*p = *tmp;
		return true;
	}
	else if (obj && PyTuple_Check(obj))
	{
		if (PyArg_ParseTuple(obj, "ffff", &p->point0, &p->point1, &p->point2, &p->point3))
			return true;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Object must be Spline object or four floats tuple.");
	return false;
}
