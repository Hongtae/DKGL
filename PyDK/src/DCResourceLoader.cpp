#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

class DCLocalResourceLoader : public DKResourceLoader
{
public:
	DCLocalResourceLoader(PyObject* obj) : object(obj)
	{
		DKASSERT_DEBUG(object);
	}
	~DCLocalResourceLoader(void)
	{
	}
	void AddResource(const DKString& name, DKResource* res) override
	{
		if (name.Length() > 0 && res)
		{
			if (this->object && Py_IsInitialized())
			{
				DCObjectCallPyCallableGIL([&]()
				{
					PyObject* obj = DCResourceFromObject(res);
					if (obj)
					{
						PyObject* ret = PyObject_CallMethod(object,
							"addResource", "NO",
							PyUnicode_FromWideChar(name, -1),
							obj);
						Py_XDECREF(ret);
					}
					Py_XDECREF(obj);
				});
			}
		}
	}
	DKObject<DKResource> FindResource(const DKString& name) const override
	{
		DKObject<DKResource> res = NULL;
		if (name.Length() > 0)
		{
			if (this->object && Py_IsInitialized())
			{
				DCObjectCallPyCallableGIL([&]()
				{
					PyObject* ret = PyObject_CallMethod(object,
						"findResource", "N",
						PyUnicode_FromWideChar(name, -1));

					if (ret)
						res = DCResourceToObject(ret);

					Py_XDECREF(ret);
				});
			}
		}
		return res;
	}
	DKObject<DKResource> LoadResource(const DKString& name)override
	{
		DKObject<DKResource> res = NULL;
		if (name.Length() > 0)
		{
			if (this->object && Py_IsInitialized())
			{
				DCObjectCallPyCallableGIL([&]()
				{
					PyObject* ret = PyObject_CallMethod(object,
						"loadResource", "N",
						PyUnicode_FromWideChar(name, -1));

					if (ret)
						res = DCResourceToObject(ret);

					Py_XDECREF(ret);
				});
			}
		}
		return res;
	}
	DKObject<DKStream> OpenResourceStream(const DKString& name) const override
	{
		DKObject<DKStream> result = NULL;
		if (name.Length() > 0)
		{
			if (this->object && Py_IsInitialized())
			{
				DCObjectCallPyCallableGIL([&]()
				{
					PyObject* ret = PyObject_CallMethod(object,
						"openResourceStream", "N",
						PyUnicode_FromWideChar(name, -1));

					if (ret)
					{
						if (PyObject_TypeCheck(ret, DCStreamTypeObject()))
						{
							result = DCStreamToObject(ret);
						}
						else if (PyObject_TypeCheck(ret, DCDataTypeObject()))
						{
							result = DKOBJECT_NEW DKDataStream(DCDataToObject(ret));
						}
						else if (PyObject_CheckBuffer(ret))
						{
							auto cleanup = [](Py_buffer* view)
							{
								if (view->obj)
									PyBuffer_Release(view);
								delete view;
							};
							Py_buffer* view = new Py_buffer();
							if (PyObject_GetBuffer(ret, view, PyBUF_SIMPLE) == 0)
							{
								DKObject<DKData> data = DKData::StaticData(view->buf, view->len, true,
									DKFunction(cleanup)->Invocation(view));

								result = DKOBJECT_NEW DKDataStream(data);
							}
							else
								cleanup(view);
						}
					}
					Py_XDECREF(ret);
				});
			}
		}
		return result;
	}

	PyObject* object = NULL;
};

struct DCResourceLoader
{
	PyObject_HEAD
	DKObject<DKResourceLoader> loader;
};

static PyObject* DCResourceLoaderNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCResourceLoader* self = (DCResourceLoader*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->loader) DKObject<DKResourceLoader>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCResourceLoaderInit(DCResourceLoader *self, PyObject *args, PyObject *kwds)
{
	if (self->loader == NULL)
	{
		self->loader = DKOBJECT_NEW DCLocalResourceLoader((PyObject*)self);
		DCObjectSetAddress(self->loader, (PyObject*)self);
	}
	return 0;
}

static void DCResourceLoaderDealloc(DCResourceLoader* self)
{
	if (self->loader)
	{
		DCObjectSetAddress(self->loader, NULL);
		self->loader = NULL;
	}

	self->loader.~DKObject<DKResourceLoader>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCResourceLoaderResourceFromObject(DCResourceLoader* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->loader, NULL);

	PyObject* obj = NULL;
	const char* name = NULL;

	if (!PyArg_ParseTuple(args, "Os", &obj, &name))
		return NULL;

	DKObject<DKResource> res = NULL;

	if (PyObject_TypeCheck(obj, DCDataTypeObject()))
	{
		Py_BEGIN_ALLOW_THREADS
		res = self->loader->ResourceFromData(DCDataToObject(obj), name);
		Py_END_ALLOW_THREADS
	}
	else if (PyObject_TypeCheck(obj, DCStreamTypeObject()))
	{
		Py_BEGIN_ALLOW_THREADS
		res = self->loader->ResourceFromStream(DCStreamToObject(obj), name);
		Py_END_ALLOW_THREADS
	}
	else if (PyUnicode_Check(obj))		// path
	{
		const char* path = PyUnicode_AsUTF8(obj);
		Py_BEGIN_ALLOW_THREADS
		res = self->loader->ResourceFromFile(path, name);
		Py_END_ALLOW_THREADS
	}
	else if (PyObject_CheckBuffer(obj))
	{
		Py_buffer view;
		if (PyObject_GetBuffer(obj, &view, PyBUF_SIMPLE) == 0)
		{
			Py_BEGIN_ALLOW_THREADS
			DKObject<DKData> data = DKData::StaticData(view.buf, view.len);
			res = self->loader->ResourceFromData(data, name);
			Py_END_ALLOW_THREADS
			PyBuffer_Release(&view);
		}
	}

	if (PyErr_Occurred())
		return NULL;

	if (res)
	{
		res->Validate();
		res->SetName(name);

		if (PyErr_Occurred())
			return NULL;

		return DCResourceFromObject(res);
	}
	PyErr_SetString(PyExc_RuntimeError, "Cannot restore resource object.");
	return NULL;
}

static PyMethodDef methods[] = {
	{ "resourceFromObject", (PyCFunction)&DCResourceLoaderResourceFromObject, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".ResourceLoader",			/* tp_name */
	sizeof(DCResourceLoader),					/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCResourceLoaderDealloc,		/* tp_dealloc */
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
	(initproc)&DCResourceLoaderInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCResourceLoaderNew,								/* tp_new */
};

PyTypeObject* DCResourceLoaderTypeObject(void)
{
	return &objectType;
}

PyObject* DCResourceLoaderFromObject(DKResourceLoader* loader)
{
	if (loader)
	{
		DCResourceLoader* self = (DCResourceLoader*)DCObjectFromAddress(loader);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DKASSERT(dynamic_cast<DCLocalResourceLoader*>(loader) == NULL);
			
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCResourceLoader*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->loader = loader;
				DCObjectSetAddress(self->loader, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKResourceLoader* DCResourceLoaderToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCResourceLoader*)obj)->loader;
	}
	return NULL;
}
