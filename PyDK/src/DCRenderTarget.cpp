#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCRenderTarget
{
	PyObject_HEAD
	DKObject<DKRenderTarget> target;
	PyObject* colorTextures;
	PyObject* depthTexture;
	void UpdateColorTextures(void)
	{
		size_t numTex = this->target->NumberOfColorTextures();
		PyObject* tuple = PyTuple_New(numTex);
		for (int i = 0; i < numTex; ++i)
		{
			DKTexture2D* tex = this->target->ColorTexture(i);
			DKASSERT_DEBUG(tex);
			PyObject* obj = DCTextureFromObject(tex);
			DKASSERT_DEBUG(DCTextureToObject(obj) == tex);
			PyTuple_SET_ITEM(tuple, i, obj);
		}
		Py_CLEAR(this->colorTextures);
		this->colorTextures = tuple;
	}
	void UpdateDepthTexture(void)
	{
		PyObject* old = this->depthTexture;
		DKTexture2D* tex = this->target->DepthTexture();
		if (tex)
		{
			PyObject* obj = DCTexture2DFromObject(tex);
			DKASSERT_DEBUG(DCTexture2DToObject(obj) == tex);
			this->depthTexture = obj;
		}
		else
			this->depthTexture = NULL;
		Py_XDECREF(old);
	}
};

static PyObject* DCRenderTargetNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCRenderTarget* self = (DCRenderTarget*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->target) DKObject<DKRenderTarget>();
		self->colorTextures = NULL;
		self->depthTexture = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCRenderTargetInit(DCRenderTarget *self, PyObject *args, PyObject *kwds)
{
	if (self->target == NULL)
	{
		int width = 0;
		int height = 0;
		int depth = (int)DKRenderTarget::DepthFormatNone;
		char* kwlist[] = { "width", "height", "depthFormat", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "iii", kwlist,
			&width, &height, &depth))
			return -1;

		if (width <= 0 && height <= 0 && depth == (int)DKRenderTarget::DepthFormatNone)
		{
			self->target = DKObject<DKRenderTarget>::New();
		}
		else
		{
			size_t maxSize = DKTexture::MaxTextureSize();
			if (width < 1 || width > maxSize)
			{
				PyErr_SetString(PyExc_ValueError, "first argument is invalid");
				return -1;
			}
			if (height < 1 || height > maxSize)
			{
				PyErr_SetString(PyExc_ValueError, "second argument is invalid");
				return -1;
			}
			if (depth < (int)DKRenderTarget::DepthFormatNone || depth >(int)DKRenderTarget::DepthFormat32)
			{
				PyErr_SetString(PyExc_ValueError, "thrid argument is invalid.");
				return -1;
			}

			self->target = DKRenderTarget::Create(width, height, (DKRenderTarget::DepthFormat)depth);
			if (self->target == NULL)
			{
				PyErr_SetString(PyExc_RuntimeError, "failed to create RenderTarget.");
				return -1;
			}
		}
		DKASSERT_DEBUG(self->target);
		DCObjectSetAddress(self->target, (PyObject*)self);
	}
	self->UpdateColorTextures();
	self->UpdateDepthTexture();
	return 0;
}

static int DCRenderTargetClear(DCRenderTarget* self)
{
	Py_CLEAR(self->colorTextures);
	Py_CLEAR(self->depthTexture);
	return 0;
}

static int DCRenderTargetTraverse(DCRenderTarget* self, visitproc visit, void* arg)
{
	Py_VISIT(self->colorTextures);
	Py_VISIT(self->depthTexture);
	return 0;
}

static void DCRenderTargetDealloc(DCRenderTarget* self)
{
	DCRenderTargetClear(self);
	if (self->target)
	{
		DCObjectSetAddress(self->target, NULL);
		self->target = NULL;
	}
	self->target.~DKObject<DKRenderTarget>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCRenderTargetNumberOfColorTextures(DCRenderTarget* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	return PyLong_FromSize_t(self->target->NumberOfColorTextures());
}

static PyObject* DCRenderTargetColorTextureAtIndex(DCRenderTarget* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;
	size_t numTex = self->target->NumberOfColorTextures();
	if (index < 0 || index >= numTex)
	{
		PyErr_Format(PyExc_IndexError, "argument index is out of range.");
		return NULL;
	}
	DKTexture2D* tex = self->target->ColorTexture(index);
	DKASSERT_DEBUG(tex);
	return DCTexture2DFromObject(tex);
}

static PyObject* DCRenderTargetColorTextures(DCRenderTarget* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	if (self->colorTextures)
	{
		Py_INCREF(self->colorTextures);
		return self->colorTextures;
	}
	return PyTuple_New(0);
}

static PyObject* DCRenderTargetSetColorTextures(DCRenderTarget* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	DKASSERT_DEBUG(PyTuple_Check(args));

	DKArray<DKTexture2D*> texArray;
	Py_ssize_t numTex = PyTuple_GET_SIZE(args);
	if (numTex == 1 && PyTuple_GET_ITEM(args, 0) == Py_None)
	{
	}
	else
	{
		texArray.Reserve(numTex);
		for (Py_ssize_t i = 0; i < numTex; ++i)
		{
			PyObject* obj = PyTuple_GET_ITEM(args, i);
			DKTexture2D* tex = DCTexture2DToObject(obj);
			if (tex == NULL)
			{
				PyErr_SetString(PyExc_TypeError, "argument must be Texture2D object.");
				return NULL;
			}
			texArray.Add(tex);
		}
	}
	bool result = false;
	if (texArray.IsEmpty())
	{
		result = self->target->SetColorTextures(NULL, 0);
	}
	else
	{
		result = self->target->SetColorTextures((DKTexture2D**)texArray, texArray.Count());
	}
	if (!result)
	{
		PyErr_SetString(PyExc_ValueError, "Invalid texture.");
		return NULL;
	}
	self->UpdateColorTextures();
	Py_RETURN_NONE;
}

static PyObject* DCRenderTargetDepthTexture(DCRenderTarget* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	if (self->depthTexture)
	{
		Py_INCREF(self->depthTexture);
		return self->depthTexture;
	}
	Py_RETURN_NONE;
}

static PyObject* DCRenderTargetSetDepthTexture(DCRenderTarget* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKTexture2D* tex = NULL;
	if (obj != Py_None)
	{
		tex = DCTexture2DToObject(obj);
		if (tex == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument must be Texture2D object or None.");
			return NULL;
		}
	}
	if (!self->target->SetDepthTexture(tex))
	{
		PyErr_SetString(PyExc_ValueError, "Invalid texture.");
		return NULL;
	}
	self->UpdateDepthTexture();
	Py_RETURN_NONE;
}

static PyObject* DCRenderTargetSetDepthBuffer(DCRenderTarget* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	int width, height, depth;
	if (!PyArg_ParseTuple(args, "iii", &width, &height, &depth))
		return NULL;
	if (width < 0)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid.");
		return NULL;
	}
	if (height < 0)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is invalid.");
		return NULL;
	}
	if (depth < (int)DKRenderTarget::DepthFormatNone || depth >(int)DKRenderTarget::DepthFormat32)
	{
		PyErr_SetString(PyExc_ValueError, "third argument is invalid.");
		return NULL;
	}
	if (!self->target->SetDepthBuffer(width, height, (DKRenderTarget::DepthFormat)depth))
	{
		PyErr_SetString(PyExc_ValueError, "Invalid arguments.");
		return NULL;
	}
	self->UpdateDepthTexture();
	Py_RETURN_NONE;
}

static PyObject* DCRenderTargetBind(DCRenderTarget* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	return PyBool_FromLong(self->target->Bind());
}

static PyObject* DCRenderTargetValidate(DCRenderTarget* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	return PyBool_FromLong(self->target->Validate());
}

static PyObject* DCRenderTargetIsValid(DCRenderTarget* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	return PyBool_FromLong(self->target->IsValid());
}

static PyObject* DCRenderTargetIsProxy(DCRenderTarget* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	return PyBool_FromLong(self->target->IsProxy());
}

static PyObject* DCRenderTargetMaxColorTextures(PyObject*, PyObject*)
{
	return PyLong_FromLong(DKRenderTarget::MaxColorTextures());
}

static PyMethodDef methods[] = {
	{ "numberOfColorTextures", (PyCFunction)&DCRenderTargetNumberOfColorTextures, METH_NOARGS },
	{ "colorTextureAtIndex", (PyCFunction)&DCRenderTargetColorTextureAtIndex, METH_VARARGS },
	{ "colorTextures", (PyCFunction)&DCRenderTargetColorTextures, METH_NOARGS },
	{ "setColorTextures", (PyCFunction)&DCRenderTargetSetColorTextures, METH_VARARGS },

	{ "depthTexture", (PyCFunction)&DCRenderTargetDepthTexture, METH_NOARGS },
	{ "setDepthTexture", (PyCFunction)&DCRenderTargetSetDepthTexture, METH_VARARGS },
	{ "setDepthBuffer", (PyCFunction)&DCRenderTargetSetDepthBuffer, METH_VARARGS },

	{ "bind", (PyCFunction)&DCRenderTargetBind, METH_NOARGS },
	{ "validate", (PyCFunction)&DCRenderTargetValidate, METH_NOARGS },
	{ "isValid", (PyCFunction)&DCRenderTargetIsValid, METH_NOARGS },
	{ "isProxy", (PyCFunction)&DCRenderTargetIsProxy, METH_NOARGS },

	{ "maxColorTextures", &DCRenderTargetMaxColorTextures, METH_NOARGS | METH_STATIC },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCRenderTargetDepthFormat(DCRenderTarget* self, void*)
{
	DCOBJECT_VALIDATE(self->target, NULL);
	return PyLong_FromLong(self->target->DepthBufferFormat());
}

static PyGetSetDef getsets[] = {
	{ "depthFormat", (getter)&DCRenderTargetDepthFormat, NULL, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".RenderTarget",		/* tp_name */
	sizeof(DCRenderTarget),					/* tp_basicsize */
	0,										/* tp_itemsize */
	(destructor)&DCRenderTargetDealloc,		/* tp_dealloc */
	0,										/* tp_print */
	0,										/* tp_getattr */
	0,										/* tp_setattr */
	0,										/* tp_reserved */
	0,										/* tp_repr */
	0,										/* tp_as_number */
	0,										/* tp_as_sequence */
	0,										/* tp_as_mapping */
	0,										/* tp_hash  */
	0,										/* tp_call */
	0,										/* tp_str */
	0,										/* tp_getattro */
	0,										/* tp_setattro */
	0,										/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,						/* tp_flags */
	0,										/* tp_doc */
	(traverseproc)&DCRenderTargetTraverse,	/* tp_traverse */
	(inquiry)&DCRenderTargetClear,			/* tp_clear */
	0,										/* tp_richcompare */
	0,										/* tp_weaklistoffset */
	0,										/* tp_iter */
	0,										/* tp_iternext */
	methods,								/* tp_methods */
	0,										/* tp_members */
	getsets,								/* tp_getset */
	0,										/* tp_base */
	0,										/* tp_dict */
	0,										/* tp_descr_get */
	0,										/* tp_descr_set */
	0,										/* tp_dictoffset */
	(initproc)&DCRenderTargetInit,			/* tp_init */
	0,										/* tp_alloc */
	&DCRenderTargetNew,						/* tp_new */
};

PyTypeObject* DCRenderTargetTypeObject(void)
{
	return &objectType;
}

PyObject* DCRenderTargetFromObject(DKRenderTarget* target)
{
	if (target)
	{
		DCRenderTarget* self = (DCRenderTarget*)DCObjectFromAddress(target);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCRenderTarget*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->target = target;
				DCObjectSetAddress(self->target, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKRenderTarget* DCRenderTargetToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCRenderTarget*)obj)->target;
	}
	return NULL;
}
