#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCMesh.h"
#include "DCObject.h"

void DCMesh::UpdateMaterial(void)
{
	PyObject* old = this->material;
	this->material = NULL;
	DKMaterial* mat = this->mesh->Material();
	if (mat)
	{
		this->material = DCMaterialFromObject(mat);
		DKASSERT_DEBUG(DCMaterialToObject(this->material) == mat);
	}
	Py_XDECREF(old);
}

void DCMesh::UpdateTextures(void)
{
	DKMesh::TextureSamplerMap& map = this->mesh->SamplerMap();
	size_t numTex = 0;
	size_t numTexSamplers = 0;
	map.EnumerateForward([&](DKMesh::TextureSamplerMap::Pair& pair)
	{
		numTex += pair.value.textures.Count();
		if (pair.value.sampler)
			numTexSamplers++;
	});

	DKArray<DKTextureSampler*> texSamplerArray;
	texSamplerArray.Reserve(numTexSamplers);
	DKArray<DKTexture*> texArray;
	texArray.Reserve(numTex);
	map.EnumerateForward([&](DKMesh::TextureSamplerMap::Pair& pair)
	{
		for (DKTexture* tex : pair.value.textures)
		{
			if (tex)
				texArray.Add(tex);
		}
		if (pair.value.sampler)
			texSamplerArray.Add(pair.value.sampler);
	});

	PyObject* samplerTuple = PyTuple_New(texSamplerArray.Count());
	for (Py_ssize_t i = 0; i < texSamplerArray.Count(); ++i)
	{
		DKTextureSampler* ts = texSamplerArray.Value(i);
		DKASSERT_DEBUG(ts);
		PyObject* obj = DCTextureSamplerFromObject(ts);
		DKASSERT_DEBUG(DCTextureSamplerToObject(obj) == ts);
		PyTuple_SET_ITEM(samplerTuple, i, obj);
	}
	Py_CLEAR(this->samplers);
	this->samplers = samplerTuple;

	PyObject* textureTuple = PyTuple_New(texArray.Count());
	for (Py_ssize_t i = 0; i < texArray.Count(); ++i)
	{
		DKTexture* tex = texArray.Value(i);
		DKASSERT_DEBUG(tex);
		PyObject* obj = DCTextureFromObject(tex);
		DKASSERT_DEBUG(DCTextureToObject(obj) == tex);
		PyTuple_SET_ITEM(textureTuple, i, obj);
	}
	Py_CLEAR(this->textures);
	this->textures = textureTuple;
}

static PyObject* DCMeshNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCMesh* self = (DCMesh*)DCModelTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->mesh = NULL;
		self->material = NULL;
		self->textures = NULL;
		self->samplers = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCMeshInit(DCMesh *self, PyObject *args, PyObject *kwds)
{
	if (self->mesh == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid Object");
		return -1;
	}
	self->UpdateMaterial();
	self->UpdateTextures();

	self->base.model = self->mesh;
	return DCModelTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static int DCMeshClear(DCMesh* self)
{
	Py_CLEAR(self->material);
	Py_CLEAR(self->textures);
	Py_CLEAR(self->samplers);
	return DCModelTypeObject()->tp_clear((PyObject*)self);
}

static int DCMeshTraverse(DCMesh* self, visitproc visit, void* arg)
{
	Py_VISIT(self->material);
	Py_VISIT(self->textures);
	Py_VISIT(self->samplers);
	return DCModelTypeObject()->tp_traverse((PyObject*)self, visit, arg);
}

static void DCMeshDealloc(DCMesh* self)
{
	Py_CLEAR(self->material);
	self->mesh = NULL;
	DCModelTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCMeshMaterial(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	self->UpdateMaterial();
	if (self->material)
	{
		Py_INCREF(self->material);
		return self->material;
	}
	Py_RETURN_NONE;
}

static PyObject* DCMeshSetMaterial(DCMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	PyObject* material;
	if (!PyArg_ParseTuple(args, "O", &material))
		return NULL;

	if (material == Py_None)
	{
		self->mesh->SetMaterial(NULL);
	}
	else
	{
		DKMaterial* mat = DCMaterialToObject(material);
		if (mat == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument must be Material object.");
			return NULL;
		}
		self->mesh->SetMaterial(mat);
	}
	self->UpdateMaterial();
	Py_RETURN_NONE;
}

static PyObject* DCMeshCanAdoptMaterial(DCMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	PyObject* material;
	if (!PyArg_ParseTuple(args, "O", &material))
		return NULL;

	if (material != Py_None)
	{
		DKMaterial* mat = DCMaterialToObject(material);
		if (mat == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument must be Material object.");
			return NULL;
		}
		return PyBool_FromLong( self->mesh->CanAdoptMaterial(mat));
	}
	Py_RETURN_FALSE;
}

static PyObject* DCMeshScale(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKVector3 v = self->mesh->Scale();
	return DCVector3FromObject(&v);
}

static PyObject* DCMeshSetScale(DCMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKVector3 v;
	if (!PyArg_ParseTuple(args, "O&", &DCVector3Converter, &v))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Vector3.");
		return NULL;
	}
	self->mesh->SetScale(v);
	Py_RETURN_NONE;
}

static PyObject* DCMeshAABB(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKAabb aabb = self->mesh->Aabb();
	return Py_BuildValue("NN", DCVector3FromObject(&aabb.positionMin), DCVector3FromObject(&aabb.positionMax));
}

static PyObject* DCMeshSetAABB(DCMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKVector3 vmin, vmax;
	if (!PyArg_ParseTuple(args, "O&O&", &DCVector3Converter, &vmin, &DCVector3Converter, &vmax))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be two Vector3 objects.");
		return NULL;
	}
	self->mesh->SetAabb(DKAabb(vmin, vmax));
	Py_RETURN_NONE;
}

static PyObject* DCMeshBoundingSphere(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKSphere sp = self->mesh->BoundingSphere();
	return Py_BuildValue("Nf", DCVector3FromObject(&sp.center), sp.radius);
}

static PyObject* DCMeshSetBoundingSphere(DCMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKVector3 c;
	float f;
	if (!PyArg_ParseTuple(args, "O&f", &DCVector3Converter, &c, &f))
		return NULL;
	self->mesh->SetBoundingSphere(DKSphere(c,f));
	Py_RETURN_NONE;
}

static PyObject* DCMeshScaledAABB(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKAabb aabb = self->mesh->ScaledAabb();
	return Py_BuildValue("NN", DCVector3FromObject(&aabb.positionMin), DCVector3FromObject(&aabb.positionMax));
}

static PyObject* DCMeshScaledBoundingSphere(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKSphere sp = self->mesh->ScaledBoundingSphere();
	return Py_BuildValue("Nf", DCVector3FromObject(&sp.center), sp.radius);
}

static PyObject* DCMeshScaledWorldTransformMatrix(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKMatrix4 tm = self->mesh->ScaledWorldTransformMatrix();
	return DCMatrix4FromObject(&tm);
}

static PyObject* DCMeshSampler(DCMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	const char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	DKMesh::TextureSampler* sampler = self->mesh->Sampler(name);
	if (sampler)
	{
		DKArray<DKTexture*> texArray;
		texArray.Reserve(sampler->textures.Count());
		for (DKTexture* t : sampler->textures)
		{
			if (t)
				texArray.Add(t);
		}

		PyObject* texTuple = PyTuple_New(texArray.Count());
		for (Py_ssize_t i = 0; i < texArray.Count(); ++i)
		{
			DKTexture* t = texArray.Value(i);
			DKASSERT_DEBUG(t);
			PyObject* obj = DCTextureFromObject(t);
			DKASSERT_DEBUG(DCTextureToObject(obj) == t);
			PyTuple_SET_ITEM(texTuple, i, obj);
		}

		PyObject* texSampler = DCTextureSamplerFromObject(sampler->sampler);
		if (sampler->sampler)
		{
			DKASSERT_DEBUG(DCTextureSamplerToObject(texSampler) == sampler->sampler);
		}
		PyObject* dict = PyDict_New();
		PyDict_SetItemString(dict, "textures", texTuple);
		PyDict_SetItemString(dict, "sampler", texSampler);

		Py_DECREF(texTuple);
		Py_DECREF(texSampler);
		return dict;
	}
	PyErr_Format(PyExc_KeyError, "key(%s) not found", name);
	return NULL;
}

static PyObject* DCMeshSetSampler(DCMesh* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	const char* name;
	PyObject* texturesObj = NULL;
	PyObject* samplerObj = NULL;
	char* kwlist[] = { "name", "textures", "sampler", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|OO", kwlist,
		&name, &texturesObj, &samplerObj))
		return NULL;

	if (name == NULL || name[0] == 0)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid.");
		return NULL;
	}

	DKMaterial::TextureArray textureArray;
	if (texturesObj && texturesObj != Py_None)
	{
		const char* err = "second argument must be Texture object sequence.";
		PyObject* seq = PySequence_Fast(texturesObj, err);
		if (seq == NULL)
			return NULL;
		DCObjectRelease tmp(seq);

		Py_ssize_t count = PySequence_Fast_GET_SIZE(seq);
		textureArray.Reserve(count);
		for (Py_ssize_t i = 0; i < count; ++i)
		{
			PyObject* obj = PySequence_Fast_GET_ITEM(seq, i);
			DKASSERT_DEBUG(obj);
			DKTexture* tex = DCTextureToObject(obj);
			if (tex == NULL)
			{
				PyErr_SetString(PyExc_TypeError, err);
				return NULL;
			}
			textureArray.Add(tex);
		}
	}
	DKTextureSampler* sampler = NULL;
	if (samplerObj && samplerObj != Py_None)
	{
		sampler = DCTextureSamplerToObject(samplerObj);
		if (sampler == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "third argument must be TextureSampler object.");
			return NULL;
		}
	}
	if (textureArray.IsEmpty())
	{
		self->mesh->RemoveSampler(name);
	}
	else
	{
		self->mesh->SetSampler(name, textureArray, sampler);
	}
	self->UpdateTextures();
	Py_RETURN_NONE;
}

static PyObject* DCMeshSamplerNames(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKMesh::TextureSamplerMap& map = self->mesh->SamplerMap();
	PyObject* tuple = PyTuple_New(map.Count());
	Py_ssize_t index = 0;
	map.EnumerateForward([&](DKMesh::TextureSamplerMap::Pair& pair)
	{
		PyTuple_SET_ITEM(tuple, index, PyUnicode_FromWideChar(pair.key, -1));
		index++;
	});
	return tuple;
}

static PyObject* DCMeshMaterialProperty(DCMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	const char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	DKMesh::PropertyArray* props = self->mesh->MaterialProperty(name);
	if (props)
	{
		PyObject* integers = PyTuple_New(props->integers.Count());
		for (Py_ssize_t i = 0; i < props->integers.Count(); ++i)
		{
			PyTuple_SET_ITEM(integers, i, PyLong_FromLong(props->integers.Value(i)));
		}
		PyObject* floatings = PyTuple_New(props->floatings.Count());
		for (Py_ssize_t i = 0; i < props->floatings.Count(); ++i)
		{
			PyTuple_SET_ITEM(floatings, i, PyFloat_FromDouble(props->floatings.Value(i)));
		}
		PyObject* dict = PyDict_New();
		PyDict_SetItemString(dict, "integers", integers);
		PyDict_SetItemString(dict, "floatings", floatings);

		Py_DECREF(integers);
		Py_DECREF(floatings);
		return dict;
	}
	PyErr_Format(PyExc_KeyError, "key(%s) not found", name);
	return NULL;
}

static PyObject* DCMeshSetMaterialProperty(DCMesh* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	const char* name;
	PyObject* integers = NULL;
	PyObject* floatings = NULL;
	char* kwlist[] = { "name", "integers", "floatings", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|OO", kwlist,
		&name, &integers, &floatings))
		return NULL;

	if (name == NULL || name[0] == 0)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid");
		return NULL;
	}

	DKMesh::PropertyArray props;
	if (integers && integers != Py_None)
	{
		const char* err = "second argument must be integer sequence.";
		PyObject* seq = PySequence_Fast(integers, err);
		if (seq == NULL)
			return NULL;
		DCObjectRelease tmp(seq);

		Py_ssize_t count = PySequence_Fast_GET_SIZE(seq);
		props.integers.Reserve(count);
		for (Py_ssize_t i = 0; i < count; ++i)
		{
			PyObject* obj = PySequence_Fast_GET_ITEM(seq, i);
			DKASSERT_DEBUG(obj);
			int v = (int)PyLong_AsLong(obj);
			if (PyErr_Occurred())
			{
				PyErr_Clear();
				PyErr_SetString(PyExc_TypeError, err);
				return NULL;
			}
			props.integers.Add(v);
		}
	}
	if (floatings && floatings != Py_None)
	{
		const char* err = "third argument must be float sequence.";
		PyObject* seq = PySequence_Fast(floatings, err);
		if (seq == NULL)
			return NULL;
		DCObjectRelease tmp(seq);

		Py_ssize_t count = PySequence_Fast_GET_SIZE(seq);
		props.floatings.Reserve(count);
		for (Py_ssize_t i = 0; i < count; ++i)
		{
			PyObject* obj = PySequence_Fast_GET_ITEM(seq, i);
			DKASSERT_DEBUG(obj);
			float v = PyFloat_AsDouble(obj);
			if (PyErr_Occurred())
			{
				PyErr_Clear();
				PyErr_SetString(PyExc_TypeError, err);
				return NULL;
			}
			props.floatings.Add(v);
		}
	}

	if (props.integers.IsEmpty() && props.floatings.IsEmpty())
	{
		self->mesh->RemoveMaterialProperty(name);
	}
	else
	{
		self->mesh->SetMaterialProperty(name, props);
	}
	Py_RETURN_NONE;
}

static PyObject* DCMeshMaterialPropertyNames(DCMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	DKMesh::PropertyMap& map = self->mesh->MaterialPropertyMap();
	PyObject* tuple = PyTuple_New(map.Count());
	Py_ssize_t index = 0;
	map.EnumerateForward([&](DKMesh::PropertyMap::Pair& pair)
	{
		PyTuple_SET_ITEM(tuple, index, PyUnicode_FromWideChar(pair.key, -1));
		index++;
	});
	return tuple;
}

static PyMethodDef methods[] = {
	{ "material", (PyCFunction)&DCMeshMaterial, METH_NOARGS },
	{ "setMaterial", (PyCFunction)&DCMeshSetMaterial, METH_VARARGS },
	{ "canAdoptMaterial", (PyCFunction)&DCMeshCanAdoptMaterial, METH_VARARGS },
	{ "scale", (PyCFunction)&DCMeshScale, METH_NOARGS },
	{ "setScale", (PyCFunction)&DCMeshSetScale, METH_VARARGS },
	{ "aabb", (PyCFunction)&DCMeshAABB, METH_NOARGS },
	{ "setAabb", (PyCFunction)&DCMeshSetAABB, METH_VARARGS },
	{ "boundingSphere", (PyCFunction)&DCMeshBoundingSphere, METH_NOARGS },
	{ "setBoundingSphere", (PyCFunction)&DCMeshSetBoundingSphere, METH_VARARGS },
	{ "scaledAabb", (PyCFunction)&DCMeshScaledAABB, METH_NOARGS },
	{ "scaledBoundingSphere", (PyCFunction)&DCMeshScaledBoundingSphere, METH_NOARGS },
	{ "scaledWorldTransformMatrix", (PyCFunction)&DCMeshScaledWorldTransformMatrix, METH_NOARGS },
	{ "sampler", (PyCFunction)&DCMeshSampler, METH_VARARGS },
	{ "setSampler", (PyCFunction)&DCMeshSetSampler, METH_VARARGS | METH_KEYWORDS },
	{ "samplerNames", (PyCFunction)&DCMeshSamplerNames, METH_NOARGS },
	{ "materialProperty", (PyCFunction)&DCMeshMaterialProperty, METH_VARARGS },
	{ "setMaterialProperty", (PyCFunction)&DCMeshSetMaterialProperty, METH_VARARGS | METH_KEYWORDS },
	{ "materialPropertyNames", (PyCFunction)&DCMeshMaterialPropertyNames, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCMeshHidden(DCMesh* self, void*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	return PyBool_FromLong(self->mesh->IsHidden());
}

static int DCMeshSetHidden(DCMesh* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->mesh, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int h = PyObject_IsTrue(value);
	if (h < 0)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}
	self->mesh->SetHidden(h != 0);
	return 0;
}

static PyObject* DCMeshDefaultPrimitiveType(DCMesh* self, void*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	int type = (int)self->mesh->DefaultPrimitiveType();
	return PyLong_FromLong(type);
}

static int DCMeshSetDefaultPrimitiveType(DCMesh* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->mesh, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	long type = PyLong_AsLong(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Integer.");
		return -1;
	}
	if (type <= 0 || type > (long)DKPrimitive::TypeTriangleStripAdjacency)
	{
		PyErr_SetString(PyExc_ValueError, "attribute is out of range.");
		return -1;
	}
	self->mesh->SetDefaultPrimitiveType((DKPrimitive::Type)type);
	return 0;
}

static PyObject* DCMeshDrawFace(DCMesh* self, void*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	int face = (int)self->mesh->DrawFace();
	return PyLong_FromLong(face);
}

static int DCMeshSetDrawFace(DCMesh* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->mesh, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	long face = PyLong_AsLong(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be Integer.");
		return -1;
	}
	if (face < 0 || face > (long)DKMesh::DrawFaceBoth)
	{
		PyErr_SetString(PyExc_ValueError, "attribute is out of range.");
		return -1;
	}
	self->mesh->SetDrawFace((DKMesh::DrawFaceMode)face);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "hidden", (getter)&DCMeshHidden, (setter)&DCMeshSetHidden, 0, 0 },
	{ "defaultPrimitiveType", (getter)&DCMeshDefaultPrimitiveType, (setter)&DCMeshSetDefaultPrimitiveType, 0, 0 },
	{ "drawFace", (getter)&DCMeshDrawFace, (setter)&DCMeshSetDrawFace, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Mesh",					/* tp_name */
	sizeof(DCMesh),								/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCMeshDealloc,					/* tp_dealloc */
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
	0,											/* tp_as_mesh */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCMeshTraverse,				/* tp_traverse */
	(inquiry)&DCMeshClear,						/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	getsets,									/* tp_getset */
	DCModelTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCMeshInit,						/* tp_init */
	0,											/* tp_alloc */
	&DCMeshNew,									/* tp_new */
};

PyTypeObject* DCMeshTypeObject(void)
{
	return &objectType;
}

PyObject* DCMeshFromObject(DKMesh* mesh)
{
	if (mesh)
	{
		DCMesh* self = (DCMesh*)DCObjectFromAddress(mesh);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DCOBJECT_DYANMIC_CAST_CONVERT(StaticMesh, mesh);

			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCMesh*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->mesh = mesh;
				DCObjectSetAddress(self->mesh, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKMesh* DCMeshToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCMesh*)obj)->mesh;
	}
	return NULL;
}
