#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCCamera
{
	PyObject_HEAD
	DKObject<DKCamera> camera;
};

static PyObject* DCCameraNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCCamera* self = (DCCamera*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->camera) DKObject<DKCamera>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCCameraInit(DCCamera *self, PyObject *args, PyObject *kwds)
{
	if (self->camera == NULL)
	{
		self->camera = DKObject<DKCamera>::New();
		DCObjectSetAddress(self->camera, (PyObject*)self);
	}

	return 0;
}

static void DCCameraDealloc(DCCamera* self)
{
	if (self->camera)
	{
		DCObjectSetAddress(self->camera, NULL);
		self->camera = NULL;
	}
	self->camera.~DKObject<DKCamera>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCCameraSetView(DCCamera* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->camera, NULL);

	DKVector3 pos, dir, up;
	if (!PyArg_ParseTuple(args, "O&O&O&",
		&DCVector3Converter, &pos,
		&DCVector3Converter, &dir,
		&DCVector3Converter, &up))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be three Vector3 objects.");
		return NULL;
	}
	self->camera->SetView(pos, dir, up);
	Py_RETURN_NONE;
}

static PyObject* DCCameraSetPerspective(DCCamera* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->camera, NULL);

	float fov, aspect, nz, fz;
	if (!PyArg_ParseTuple(args, "ffff", &fov, &aspect, &nz, &fz))
		return NULL;

	if (fov <= 0 || aspect <= 0 || nz <= 0 || fz <= 0)
	{
		PyErr_SetString(PyExc_ValueError, "all arguments must be greater than zero.");
		return NULL;
	}
	if (nz >= fz)
	{
		PyErr_SetString(PyExc_ValueError, "fourth argument must be greater than third argument.");
		return NULL;
	}

	self->camera->SetPerspective(fov, aspect, nz, fz);
	Py_RETURN_NONE;
}

static PyObject* DCCameraSetOrthographic(DCCamera* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->camera, NULL);

	float width, height, nz, fz;
	if (!PyArg_ParseTuple(args, "ffff", &width, &height, &nz, &fz))
		return NULL;

	if (width <= 0)
	{
		PyErr_SetString(PyExc_ValueError, "first arguments must be greater than zero.");
		return NULL;
	}
	if (height <= 0)
	{
		PyErr_SetString(PyExc_ValueError, "second arguments must be greater than zero.");
		return NULL;
	}
	if (nz >= fz)
	{
		PyErr_SetString(PyExc_ValueError, "fourth argument must be greater than third argument.");
		return NULL;
	}

	self->camera->SetOrthographic(width, height, nz, fz);
	Py_RETURN_NONE;
}

static PyObject* DCCameraViewMatrix(DCCamera* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	DKMatrix4 mat = self->camera->ViewMatrix();
	return DCMatrix4FromObject(&mat);
}

static PyObject* DCCameraSetViewMatrix(DCCamera* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->camera, NULL);

	DKMatrix4 mat;
	if (!PyArg_ParseTuple(args, "O&", &DCMatrix4Converter, &mat))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Matrix4 object.");
		return NULL;
	}
	self->camera->SetView(mat);
	Py_RETURN_NONE;
}

static PyObject* DCCameraProjectionMatrix(DCCamera* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	DKMatrix4 mat = self->camera->ProjectionMatrix();
	return DCMatrix4FromObject(&mat);
}

static PyObject* DCCameraSetProjectionMatrix(DCCamera* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->camera, NULL);

	DKMatrix4 mat;
	if (!PyArg_ParseTuple(args, "O&", &DCMatrix4Converter, &mat))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Matrix4 object.");
		return NULL;
	}
	self->camera->SetProjection(mat);
	Py_RETURN_NONE;
}

static PyObject* DCCameraViewProjectionMatrix(DCCamera* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	DKMatrix4 mat = self->camera->ViewProjectionMatrix();
	return DCMatrix4FromObject(&mat);
}

static PyObject* DCCameraSetViewProjectionMatrix(DCCamera* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->camera, NULL);

	DKMatrix4 v, p;
	if (!PyArg_ParseTuple(args, "O&O&", &DCMatrix4Converter, &v, &DCMatrix4Converter, &p))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "arguments must be two Matrix4 objects.");
		return NULL;
	}
	self->camera->SetViewProjection(v, p);
	Py_RETURN_NONE;
}

static PyObject* DCCameraPosition(DCCamera* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	DKVector3 pos = self->camera->ViewPosition();
	return DCVector3FromObject(&pos);
}

static PyObject* DCCameraDirection(DCCamera* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	DKVector3 pos = self->camera->ViewDirection();
	return DCVector3FromObject(&pos);
}

static PyObject* DCCameraUp(DCCamera* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	DKVector3 pos = self->camera->ViewUp();
	return DCVector3FromObject(&pos);
}

static PyObject* DCCameraIsPerspective(DCCamera* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	if (self->camera->IsPerspective()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCCameraIsOrthographic(DCCamera* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	if (self->camera->IsOrthographic()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCCameraIsPointInside(DCCamera* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	DKVector3 pt;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &pt))
		return NULL;

	if (self->camera->IsPointInside(pt)) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCCameraIsSphereInside(DCCamera* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->camera, NULL);
	DKVector3 center;
	float radius;
	if (!PyArg_ParseTuple(args, "O&f", &DCVector3Converter, &center, &radius))
		return NULL;

	if (self->camera->IsSphereInside(center, radius)){ Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "setView", (PyCFunction)&DCCameraSetView, METH_VARARGS },
	{ "setPerspective", (PyCFunction)&DCCameraSetPerspective, METH_VARARGS },
	{ "setOrthographic", (PyCFunction)&DCCameraSetOrthographic, METH_VARARGS },

	{ "viewMatrix", (PyCFunction)&DCCameraViewMatrix, METH_NOARGS },
	{ "setViewMatrix", (PyCFunction)&DCCameraSetViewMatrix, METH_VARARGS },
	{ "projectionMatrix", (PyCFunction)&DCCameraProjectionMatrix, METH_NOARGS },
	{ "setProjectionMatrix", (PyCFunction)&DCCameraSetProjectionMatrix, METH_VARARGS },
	{ "viewProjectionMatrix", (PyCFunction)&DCCameraViewProjectionMatrix, METH_NOARGS },
	{ "setViewProjectionMatrix", (PyCFunction)&DCCameraSetViewProjectionMatrix, METH_VARARGS },

	{ "position", (PyCFunction)&DCCameraPosition, METH_NOARGS },
	{ "direction", (PyCFunction)&DCCameraDirection, METH_NOARGS },
	{ "up", (PyCFunction)&DCCameraUp, METH_NOARGS },
	{ "isPerspective", (PyCFunction)&DCCameraIsPerspective, METH_NOARGS },
	{ "isOrthographic", (PyCFunction)&DCCameraIsOrthographic, METH_NOARGS },
	{ "isPointInside", (PyCFunction)&DCCameraIsPointInside, METH_NOARGS },
	{ "isSphereInside", (PyCFunction)&DCCameraIsSphereInside, METH_NOARGS },
	
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};


static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Camera",					/* tp_name */
	sizeof(DCCamera),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCCameraDealloc,				/* tp_dealloc */
	0,											/* tp_print */
	0,											/* tp_getattr */
	0,											/* tp_setattr */
	0,											/* tp_reserved */
	0,											/* tp_repr */
	0,											/* tp_as_number */
	0,											/* tp_as_sequence */
	0,											/* tp_as_mapping */
	0,											/* tp_hash  */
	0,											/* tp_call */
	0,											/* tp_str */
	0,											/* tp_getattro */
	0,											/* tp_setattro */
	0,											/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
	0,											/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCCameraInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCCameraNew,								/* tp_new */
};

PyTypeObject* DCCameraTypeObject(void)
{
	return &objectType;
}

PyObject* DCCameraFromObject(DKCamera* camera)
{
	if (camera)
	{
		DCCamera* self = (DCCamera*)DCObjectFromAddress(camera);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCCamera*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->camera = camera;
				DCObjectSetAddress(self->camera, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKCamera* DCCameraToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCCamera*)obj)->camera;
	}
	return NULL;
}
