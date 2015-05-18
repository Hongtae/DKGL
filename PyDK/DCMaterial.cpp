#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCResource.h"
#include "DCObject.h"

struct DCMaterial
{
	DCResource base;
	DKMaterial* material;
	PyObject* textures;
	PyObject* samplers;
	PyObject* shaders;
	PyObject* programs;

	void UpdateTextures(void)
	{
		size_t numTextures = 0;
		size_t numSamplers = 0;
		this->material->samplerProperties.EnumerateForward([&](DKMaterial::SamplerPropertyMap::Pair& p)
		{
			for (DKTexture* t : p.value.textures)
			{
				if (t)
					numTextures++;
			}
			if (p.value.sampler)
				numSamplers++;
		});

		DKArray<DKTextureSampler*> samplerArray;
		samplerArray.Reserve(numSamplers);
		DKArray<DKTexture*> textureArray;
		textureArray.Reserve(numTextures);

		this->material->samplerProperties.EnumerateForward([&](DKMaterial::SamplerPropertyMap::Pair& p)
		{
			for (DKTexture* t : p.value.textures)
			{
				if (t)
					textureArray.Add(t);
			}
			if (p.value.sampler)
				samplerArray.Add(p.value.sampler);
		});

		PyObject* samTuple = PyTuple_New(samplerArray.Count());
		for (Py_ssize_t i = 0; i < samplerArray.Count(); ++i)
		{
			DKTextureSampler* ts = samplerArray.Value(i);
			DKASSERT_DEBUG(ts);
			PyObject* obj = DCTextureSamplerFromObject(ts);
			DKASSERT_DEBUG(DCTextureSamplerToObject(obj) == ts);
			PyTuple_SET_ITEM(samTuple, i, obj);
		}
		PyObject* texTuple = PyTuple_New(textureArray.Count());
		for (Py_ssize_t i = 0; i < textureArray.Count(); ++i)
		{
			DKTexture* tex = textureArray.Value(i);
			DKASSERT_DEBUG(tex);
			PyObject* obj = DCTextureFromObject(tex);
			DKASSERT_DEBUG(DCTextureToObject(obj) == tex);
			PyTuple_SET_ITEM(texTuple, i, obj);
		}
		PyObject* tmp1 = this->textures;
		PyObject* tmp2 = this->samplers;
		
		this->textures = texTuple;
		this->samplers = samTuple;
		Py_XDECREF(tmp1);
		Py_XDECREF(tmp2);
	}
	void UpdateShaders(void)
	{
		DKSet<DKShader*> shaderSet;
		DKSet<DKShaderProgram*> programSet;
		for (DKMaterial::ShaderSource& src : this->material->shaderProperties)
		{
			if (src.shader)
				shaderSet.Insert(src.shader);
		}
		for (DKMaterial::RenderingProperty& rp : this->material->renderingProperties)
		{
			for (DKMaterial::ShaderSource& src : rp.shaders)
			{
				if (src.shader)
					shaderSet.Insert(src.shader);
			}
			if (rp.program)
				programSet.Insert(rp.program);
		}

		PyObject* tuple = PyTuple_New(shaderSet.Count());
		Py_ssize_t index = 0;
		shaderSet.EnumerateForward([&](const DKShader* s)
		{
			PyObject* obj = DCShaderFromObject(const_cast<DKShader*>(s));
			DKASSERT_DEBUG(DCShaderToObject(obj) == s);
			PyTuple_SET_ITEM(tuple, index, obj);
			index++;
		});
		Py_XDECREF(this->shaders);
		this->shaders = tuple;

		index = 0;
		tuple = PyTuple_New(programSet.Count());
		programSet.EnumerateForward([&](const DKShaderProgram* p)
		{
			PyObject* obj = DCShaderProgramFromObject(const_cast<DKShaderProgram*>(p));
			DKASSERT_DEBUG(DCShaderProgramToObject(obj) == p);
			PyTuple_SET_ITEM(tuple, index, obj);
			index++;
		});
		Py_XDECREF(this->programs);
		this->programs = tuple;
	}
};

static PyObject* DCMaterialNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCMaterial* self = (DCMaterial*)DCResourceTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->material = NULL;
		self->textures = NULL;
		self->samplers = NULL;
		self->shaders = NULL;
		self->programs = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCMaterialInit(DCMaterial *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKMaterial> material = NULL;
	if (self->material == NULL)
	{
		material = DKObject<DKMaterial>::New();
		self->material = material;
		DCObjectSetAddress(self->material, (PyObject*)self);
	}
	else
	{
		self->UpdateTextures();
		self->UpdateShaders();
	}

	self->base.resource = self->material;
	return DCResourceTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static int DCMaterialClear(DCMaterial* self)
{
	self->material = NULL;
	Py_CLEAR(self->textures);
	Py_CLEAR(self->samplers);
	Py_CLEAR(self->shaders);
	Py_CLEAR(self->programs);
	return 0;
}

static int DCMaterialTraverse(DCMaterial* self, visitproc visit, void* arg)
{
	Py_VISIT(self->textures);
	Py_VISIT(self->samplers);
	Py_VISIT(self->shaders);
	Py_VISIT(self->programs);
	return 0;
}

static void DCMaterialDealloc(DCMaterial* self)
{
	DCMaterialClear(self);
	DCResourceTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCMaterialShadingProperty(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	const char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	auto p = self->material->shadingProperties.Find(name);
	if (p)
	{
		DKMaterial::ShadingProperty& sp = p->value;

		PyObject* integers = PyTuple_New(sp.value.integers.Count());
		for (Py_ssize_t i = 0; i < sp.value.integers.Count(); ++i)
		{
			PyTuple_SET_ITEM(integers, i, PyLong_FromLong(sp.value.integers.Value(i)));
		}
		PyObject* floatings = PyTuple_New(sp.value.floatings.Count());
		for (Py_ssize_t i = 0; i < sp.value.floatings.Count(); ++i)
		{
			PyTuple_SET_ITEM(floatings, i, PyFloat_FromDouble(sp.value.floatings.Value(i)));
		}
		PyObject* uniform = PyLong_FromLong((long)sp.id);
		PyObject* type = PyLong_FromLong((long)sp.type);

		PyObject* dict = PyDict_New();
		PyDict_SetItemString(dict, "id", uniform);
		PyDict_SetItemString(dict, "type", type);
		PyDict_SetItemString(dict, "integers", integers);
		PyDict_SetItemString(dict, "floatings", floatings);

		Py_DECREF(integers);
		Py_DECREF(floatings);
		Py_DECREF(uniform);
		Py_DECREF(type);

		return dict;
	}
	PyErr_Format(PyExc_KeyError, "key(%s) not found", name);
	return NULL;
}

static PyObject* DCMaterialSetShadingProperty(DCMaterial* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->material, NULL);

	const char* name;
	int uniform, type;
	PyObject* integers = NULL;
	PyObject* floatings = NULL;
	char* kwlist[] = { "name", "id", "type", "integers", "floatings", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "sii|OO", kwlist,
		&name, &uniform, &type, &integers, &floatings))
		return NULL;

	if (name == NULL || name[0] == 0)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid");
		return NULL;
	}
	if (uniform <= 0 || uniform >= DKShaderConstant::UniformMax)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range. (invalid uniform-id)");
		return NULL;
	}
	if (type <= 0 || type >= DKShaderConstant::TypeMax)
	{
		PyErr_SetString(PyExc_ValueError, "third argument is out of range. (invalid uniform-type)");
		return NULL;
	}

	DKMaterial::PropertyArray props;
	if (integers && integers != Py_None)
	{
		const char* err = "fifth argument must be integer sequence.";
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
		const char* err = "sixth argument must be float sequence.";
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
		self->material->shadingProperties.Remove(name);
	}
	else
	{
		DKMaterial::ShadingProperty& sp = self->material->shadingProperties.Value(name);
		sp.id = (DKShaderConstant::Uniform)uniform;
		sp.type = (DKShaderConstant::Type)type;
		sp.value = props;
	}
	Py_RETURN_NONE;
}

static PyObject* DCMaterialShadingPropertyNames(DCMaterial* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	PyObject* tuple = PyTuple_New(self->material->shadingProperties.Count());
	Py_ssize_t index = 0;
	self->material->shadingProperties.EnumerateBackward([&](DKMaterial::ShadingPropertyMap::Pair& p)
	{
		PyTuple_SET_ITEM(tuple, index, PyUnicode_FromWideChar(p.key, -1));
		index++;
	});
	return tuple;
}

static PyObject* DCMaterialSamplerProperty(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	const char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	auto p = self->material->samplerProperties.Find(name);
	if (p)
	{
		DKMaterial::SamplerProperty& sp = p->value;

		DKArray<DKTexture*> texArray;
		texArray.Reserve(sp.textures.Count());
		for (DKTexture* t : sp.textures)
		{
			if (t)
				texArray.Add(t);
		}
		PyObject* textures = PyTuple_New(texArray.Count());
		for (size_t i = 0; i < texArray.Count(); ++i)
		{
			PyObject* obj = DCTextureFromObject(texArray.Value(i));
			DKASSERT_DEBUG(DCTextureToObject(obj) == texArray.Value(i));
			PyTuple_SET_ITEM(textures, i, obj);
		}

		PyObject* uniform = PyLong_FromLong((long)sp.id);
		PyObject* type = PyLong_FromLong((long)sp.type);
		PyObject* sampler = DCTextureSamplerFromObject(sp.sampler);

		PyObject* dict = PyDict_New();
		PyDict_SetItemString(dict, "id", uniform);
		PyDict_SetItemString(dict, "type", type);
		PyDict_SetItemString(dict, "textures", textures);
		PyDict_SetItemString(dict, "sampler", sampler);

		Py_DECREF(textures);
		Py_DECREF(uniform);
		Py_DECREF(type);
		Py_DECREF(sampler);

		return dict;
	}
	PyErr_Format(PyExc_KeyError, "key(%s) not found", name);
	return NULL;
}

static PyObject* DCMaterialSetSamplerProperty(DCMaterial* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->material, NULL);

	const char* name;
	int uniform, type;
	PyObject* texturesObj = NULL;
	PyObject* samplerObj = NULL;
	char* kwlist[] = { "name", "id", "type", "textures", "sampler",  NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "sii|OO", kwlist,
		&name, &uniform, &type, &texturesObj, &samplerObj))
		return NULL;

	if (name == NULL || name[0] == 0)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid.");
		return NULL;
	}
	if (uniform <= 0 || uniform >= DKShaderConstant::UniformMax)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range. (invalid uniform-id)");
		return NULL;
	}
	if (type <= 0 || type >= DKShaderConstant::TypeMax)
	{
		PyErr_SetString(PyExc_ValueError, "third argument is out of range. (invalid uniform-type)");
		return NULL;
	}

	DKMaterial::TextureArray textureArray;
	if (texturesObj && texturesObj != Py_None)
	{
		const char* err = "fourth argument must be Texture object sequence.";
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
			PyErr_SetString(PyExc_TypeError, "fifth argument must be TextureSampler object.");
			return NULL;
		}
	}

	if (textureArray.IsEmpty())
	{
		self->material->samplerProperties.Remove(name);
	}
	else
	{
		DKMaterial::SamplerProperty& sp = self->material->samplerProperties.Value(name);
		sp.id = (DKShaderConstant::Uniform)uniform;
		sp.type = (DKShaderConstant::Type)type;
		sp.textures = textureArray;
		sp.sampler = sampler;
	}
	self->UpdateTextures();
	Py_RETURN_NONE;
}

static PyObject* DCMaterialSamplerPropertyNames(DCMaterial* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	PyObject* tuple = PyTuple_New(self->material->samplerProperties.Count());
	Py_ssize_t index = 0;
	self->material->samplerProperties.EnumerateBackward([&](DKMaterial::SamplerPropertyMap::Pair& p)
	{
		PyTuple_SET_ITEM(tuple, index, PyUnicode_FromWideChar(p.key, -1));
		index++;
	});
	return tuple;
}

static PyObject* DCMaterialStreamProperty(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	const char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	auto p = self->material->streamProperties.Find(name);
	if (p)
	{
		DKMaterial::StreamProperty& sp = p->value;

		PyObject* stream = PyLong_FromLong((long)sp.id);
		PyObject* type = PyLong_FromLong((long)sp.type);
		PyObject* components = PyLong_FromSize_t(sp.components);

		PyObject* dict = PyDict_New();
		PyDict_SetItemString(dict, "id", stream);
		PyDict_SetItemString(dict, "type", type);
		PyDict_SetItemString(dict, "components", components);

		Py_DECREF(stream);
		Py_DECREF(type);
		Py_DECREF(components);

		return dict;
	}
	PyErr_Format(PyExc_KeyError, "key(%s) not found", name);
	return NULL;
}


static PyObject* DCMaterialSetStreamProperty(DCMaterial* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->material, NULL);

	const char* name;
	int stream, type;
	Py_ssize_t components = 0;
	char* kwlist[] = { "name", "id", "type", "components", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "sii|n", kwlist,
		&name, &stream, &type, &components))
		return NULL;

	if (name == NULL || name[0] == 0)
	{
		PyErr_SetString(PyExc_ValueError, "first argument is invalid.");
		return NULL;
	}
	if (stream <= 0 || stream >= DKVertexStream::StreamMax)
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range. (invalid stream-id)");
		return NULL;
	}
	if (type <= 0 || type >= DKVertexStream::TypeMax)
	{
		PyErr_SetString(PyExc_ValueError, "third argument is out of range. (invalid stream-type)");
		return NULL;
	}
	if (components < 0)
	{
		PyErr_SetString(PyExc_ValueError, "fourth argument cannot be negative integer.");
		return NULL;
	}

	if (components == 0)
	{
		self->material->streamProperties.Remove(name);
	}
	else
	{
		DKMaterial::StreamProperty& sp = self->material->streamProperties.Value(name);
		sp.id = (DKVertexStream::Stream)stream;
		sp.type = (DKVertexStream::Type)type;
		sp.components = components;
	}
	Py_RETURN_NONE;
}

static PyObject* DCMaterialStreamPropertyNames(DCMaterial* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	PyObject* tuple = PyTuple_New(self->material->streamProperties.Count());
	Py_ssize_t index = 0;
	self->material->streamProperties.EnumerateBackward([&](DKMaterial::StreamPropertyMap::Pair& p)
	{
		PyTuple_SET_ITEM(tuple, index, PyUnicode_FromWideChar(p.key, -1));
		index++;
	});
	return tuple;
}

static PyObject* DCMaterialNumberOfShaderProperties(DCMaterial* self, PyObject*)
{
	size_t c = 0;
	if (self->material)
		c = self->material->shaderProperties.Count();
	return PyLong_FromSize_t(c);
}

static PyObject* DCMaterialShaderProperty(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	Py_ssize_t index;
	if (!PyArg_ParseTuple(args, "n", &index))
		return NULL;
	if (index < 0 || index >= self->material->shaderProperties.Count())
	{
		PyErr_SetString(PyExc_ValueError, "argument is out of range.");
		return NULL;
	}
	DKMaterial::ShaderSource& src = self->material->shaderProperties.Value(index);
	PyObject* name = PyUnicode_FromWideChar(src.name, -1);
	PyObject* source = PyUnicode_FromWideChar(src.source, -1);
	PyObject* type = PyLong_FromLong((long)src.type);
	PyObject* shader = DCShaderFromObject(src.shader);
	DKASSERT_DEBUG(DCShaderToObject(shader) == src.shader);

	PyObject* dict = PyDict_New();
	PyDict_SetItemString(dict, "name", name);
	PyDict_SetItemString(dict, "source", source);
	PyDict_SetItemString(dict, "type", type);
	PyDict_SetItemString(dict, "shader", shader);

	Py_DECREF(name);
	Py_DECREF(source);
	Py_DECREF(type);
	Py_DECREF(shader);
	return dict;
}

static PyObject* DCMaterialSetShaderProperty(DCMaterial* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	Py_ssize_t index;
	const char* name = NULL;
	const char* source = NULL;
	PyObject* typeObj = NULL;
	PyObject* shaderObj = NULL;
	char* kwlist[] = { "index", "name", "source", "type", "shader", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "n|$ssOO", kwlist,
		&index, &name, &source, &typeObj, &shaderObj))
		return NULL;

	if (index < 0 || index >= self->material->shaderProperties.Count())
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range.");
		return NULL;
	}

	long type = 0;
	if (typeObj)
	{
		type = PyLong_AsLong(typeObj);
		if (PyErr_Occurred())
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "argument 'type' must be integer.");
			return NULL;
		}
		if (type <= 0 || type > DKShader::TypeGeometryShader)
		{
			PyErr_SetString(PyExc_ValueError, "argument 'type' is out of range. (invalid shader type)");
			return NULL;
		}
	}
	DKShader* shader = NULL;
	if (shaderObj && shaderObj != Py_None)
	{
		shader = DCShaderToObject(shaderObj);
		if (shader == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument 'shader' is invalid.");
			return NULL;
		}
	}

	DKMaterial::ShaderSource& src = self->material->shaderProperties.Value(index);
	if (name)
		src.name = name;
	if (source)
		src.source = source;
	if (typeObj)
		src.type = (DKShader::Type)type;
	if (shaderObj)
	{
		src.shader = shader;
		self->UpdateShaders();
	}
	Py_RETURN_NONE;
}

static PyObject* DCMaterialAddShaderProperty(DCMaterial* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	const char *name, *source;
	int type;
	PyObject* obj = NULL;
	char* kwlist[] = { "name", "source", "type", "shader", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ssi|O", kwlist,
		&name, &source, &type, &obj))
		return NULL;

	if (type <= 0 || type > DKShader::TypeGeometryShader)
	{
		PyErr_SetString(PyExc_ValueError, "third argument is out of range. (invalid shader type)");
		return NULL;
	}

	DKShader* shader = NULL;
	if (obj && obj != Py_None)
	{
		shader = DCShaderToObject(obj);
		if (shader == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "fourth argument is invalid.");
			return NULL;
		}
	}

	DKMaterial::ShaderSource src;
	src.name = name;
	src.source = source;
	src.type = (DKShader::Type)type;
	src.shader = shader;
	auto index = self->material->shaderProperties.Add(src);
	self->UpdateShaders();
	return PyLong_FromSize_t(index);
}

static PyObject* DCMaterialRemoveShaderProperty(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	Py_ssize_t index;
	if (!PyArg_ParseTuple(args, "n", &index))
		return NULL;
	if (index < 0 || index >= self->material->shaderProperties.Count())
	{
		PyErr_SetString(PyExc_ValueError, "argument is out of range.");
		return NULL;
	}
	self->material->shaderProperties.Remove(index);
	self->UpdateShaders();
	Py_RETURN_NONE;
}

static PyObject* DCMaterialRemoveAllShaderProperties(DCMaterial* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	self->material->shaderProperties.Clear();
	self->UpdateShaders();
	Py_RETURN_NONE;
}

static PyObject* DCMaterialNumberOfRenderingProperties(DCMaterial* self, PyObject*)
{
	size_t c = 0;
	if (self->material)
		c = self->material->renderingProperties.Count();
	return PyLong_FromSize_t(c);
}

static PyObject* DCMaterialRenderingProperty(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	Py_ssize_t index;
	if (!PyArg_ParseTuple(args, "n", &index))
		return NULL;
	if (index < 0 || index >= self->material->renderingProperties.Count())
	{
		PyErr_SetString(PyExc_ValueError, "argument is out of range.");
		return NULL;
	}
	DKMaterial::RenderingProperty& rp = self->material->renderingProperties.Value(index);
	PyObject* name = PyUnicode_FromWideChar(rp.name, -1);
	PyObject* depthFunc = PyLong_FromLong((long)rp.depthFunc);
	PyObject* depthWrite = PyBool_FromLong(rp.depthWrite);
	PyObject* blendState = DCBlendStateFromObject(&rp.blendState);
	PyObject* shaders = PyTuple_New(rp.shaders.Count());
	for (size_t i = 0; i < rp.shaders.Count(); ++i)
	{
		DKMaterial::ShaderSource& src = rp.shaders.Value(i);
		PyObject* srcName = PyUnicode_FromWideChar(src.name, -1);
		PyObject* srcSrc = PyUnicode_FromWideChar(src.source, -1);
		PyObject* srcType = PyLong_FromLong((long)src.type);
		PyObject* srcShader = DCShaderFromObject(src.shader);
		DKASSERT_DEBUG(DCShaderToObject(srcShader) == src.shader);

		PyObject* dict = PyDict_New();
		PyDict_SetItemString(dict, "name", srcName);
		PyDict_SetItemString(dict, "source", srcSrc);
		PyDict_SetItemString(dict, "type", srcType);
		PyDict_SetItemString(dict, "shader", srcShader);

		Py_DECREF(srcName);
		Py_DECREF(srcSrc);
		Py_DECREF(srcType);
		Py_DECREF(srcShader);

		PyTuple_SET_ITEM(shaders, i, dict);
	}
	PyObject* program = DCShaderProgramFromObject(rp.program);
	DKASSERT_DEBUG(DCShaderProgramToObject(program) == rp.program);

	PyObject* dict = PyDict_New();
	PyDict_SetItemString(dict, "name", name);
	PyDict_SetItemString(dict, "depthFunc", depthFunc);
	PyDict_SetItemString(dict, "depthWrite", depthWrite);
	PyDict_SetItemString(dict, "blendState", blendState);
	PyDict_SetItemString(dict, "shaders", shaders);
	PyDict_SetItemString(dict, "program", program);

	Py_DECREF(name);
	Py_DECREF(depthFunc);
	Py_DECREF(depthWrite);
	Py_DECREF(blendState);
	Py_DECREF(shaders);
	Py_DECREF(program);
	return dict;
}

static bool UnpackShaderSourcesFromSequence(PyObject* obj, DKArray<DKMaterial::ShaderSource>& srcArray)
{
	const char* err = "argument 'shaders' must be ShaderSource dictionary sequence";
	PyObject* seq = PySequence_Fast(obj, err);
	if (seq == NULL)
	{
		return false;
	}
	DCObjectRelease r_(seq);

	Py_ssize_t count = PySequence_Fast_GET_SIZE(seq);
	srcArray.Reserve(count);
	for (Py_ssize_t i = 0; i < count; ++i)
	{
		PyObject* dict = PySequence_Fast_GET_ITEM(seq, i);
		if (!PyMapping_Check(dict))
		{
			PyErr_SetString(PyExc_TypeError, err);
			return NULL;
		}
		PyObject* srcNameObj = PyMapping_GetItemString(dict, "name");
		if (srcNameObj == NULL) return NULL;
		DCObjectRelease n_(srcNameObj);
		PyObject* srcSrcObj = PyMapping_GetItemString(dict, "source");
		if (srcSrcObj == NULL) return NULL;
		DCObjectRelease s_(srcSrcObj);
		PyObject* srcTypeObj = PyMapping_GetItemString(dict, "type");
		if (srcTypeObj == NULL) return NULL;
		DCObjectRelease t_(srcTypeObj);
		PyObject* srcShaderObj = PyMapping_GetItemString(dict, "shader");
		if (srcShaderObj == NULL) PyErr_Clear();

		const char* srcName = PyUnicode_AsUTF8(srcNameObj);
		if (srcName == NULL)
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "argument 'shaders' dictionary value for key:'name' must be string.");
			return NULL;
		}
		const char* srcSrc = PyUnicode_AsUTF8(srcSrcObj);
		if (srcSrc == NULL)
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "argument 'shaders' dictionary value for key:'source' must be string.");
			return NULL;
		}
		long srcType = PyLong_AsLong(srcTypeObj);
		if (PyErr_Occurred())
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "argument 'shaders' dictionary value for key:'type' must be integer.");
			return NULL;
		}
		if (srcType <= 0 || srcType > 3)		// DKShader::Type max value
		{
			PyErr_SetString(PyExc_ValueError, "argument 'shaders' dictionary value for key:'type' is out of range.");
			return NULL;
		}
		DKShader* srcShader = NULL;
		if (srcShaderObj && srcShaderObj != Py_None)
		{
			srcShader = DCShaderToObject(srcShaderObj);
			if (srcShader == NULL)
			{
				PyErr_SetString(PyExc_ValueError, "argument 'shaders' dictionary value for key:'shader' must be Shader object.");
				return NULL;
			}
		}
		DKMaterial::ShaderSource src;
		src.name = srcName;
		src.source = srcSrc;
		src.type = (DKShader::Type)srcType;
		src.shader = srcShader;
		srcArray.Add(src);
	}
	return true;
}

static PyObject* DCMaterialSetRenderingProperty(DCMaterial* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	Py_ssize_t index;
	const char* name = NULL;
	PyObject* depthFuncObj = NULL;
	PyObject* depthWriteObj = NULL;
	PyObject* blendStateObj = NULL;
	PyObject* shadersObj = NULL;
	PyObject* programObj = NULL;

	char* kwlist[] = { "index", "name", "depthFunc", "depthWrite", "blendState", "shaders", "program", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "n|$sOOOOO", kwlist,
		&index, &name, &depthFuncObj, &depthWriteObj, &blendStateObj, &shadersObj, &programObj))
		return NULL;

	if (index < 0 || index >= self->material->renderingProperties.Count())
	{
		PyErr_SetString(PyExc_ValueError, "first argument is out of range.");
		return NULL;
	}

	long depthFunc = 0;
	if (depthFuncObj)
	{
		depthFunc = PyLong_AsLong(depthFuncObj);
		if (PyErr_Occurred())
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "argument 'depthFunc' must be integer.");
			return NULL;
		}
		if (depthFunc < 0 || depthFunc > 7) // DKMaterial::RenderingProperty::DepthFunc max-value
		{
			PyErr_SetString(PyExc_ValueError, "argument 'depthFunc' is out of range.");
			return NULL;
		}
	}
	int depthWrite = 0;
	if (depthWriteObj)
	{
		depthWrite = PyObject_IsTrue(depthWriteObj);
		if (depthWrite < 0)
		{
			PyErr_SetString(PyExc_TypeError, "argument 'depthWrite' must be boolean.");
			return NULL;
		}
	}
	DKBlendState* bs = NULL;
	if (blendStateObj)
	{
		bs = DCBlendStateToObject(blendStateObj);
		if (bs == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument 'blendState' must be BlendState object.");
			return NULL;
		}
	}
	DKArray<DKMaterial::ShaderSource> shaders;
	if (shadersObj && !UnpackShaderSourcesFromSequence(shadersObj, shaders))
		return NULL;

	DKShaderProgram* program = NULL;
	if (programObj)
	{
		program = DCShaderProgramToObject(programObj);
		if (program == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument 'program' must be ShaderProgram object.");
			return NULL;
		}
	}

	DKMaterial::RenderingProperty& rp = self->material->renderingProperties.Value(index);
	if (name)
		rp.name = name;
	if (depthFuncObj)
		rp.depthFunc = (DKMaterial::RenderingProperty::DepthFunc)depthFunc;
	if (depthWriteObj)
		rp.depthWrite = depthWrite != 0;
	if (shadersObj)
		rp.shaders = shaders;
	if (programObj)
		rp.program = program;

	if (shadersObj || programObj)
		self->UpdateShaders();
	Py_RETURN_NONE;
}

static PyObject* DCMaterialAddRenderingProperty(DCMaterial* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	const char *name;
	int depthFunc;
	int depthWrite;
	PyObject* blendStateObj;
	PyObject* shadersObj;
	PyObject* programObj = NULL;
	char* kwlist[] = { "name", "depthFunc", "depthWrite", "blendState", "shaders", "program", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "sipOO|O", kwlist,
		&name, &depthFunc, &depthWrite, &blendStateObj, &shadersObj, &programObj))
		return NULL;

	if (depthFunc < 0 || depthFunc > 7) // DKMaterial::RenderingProperty::DepthFunc max-value
	{
		PyErr_SetString(PyExc_ValueError, "second argument is out of range.");
		return NULL;
	}
	DKBlendState* bs = DCBlendStateToObject(blendStateObj);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "fourth argument must be BlendState object.");
		return NULL;
	}
	DKArray<DKMaterial::ShaderSource> shaders;
	if (!UnpackShaderSourcesFromSequence(shadersObj, shaders))
		return NULL;

	DKShaderProgram* program = NULL;
	if (programObj && programObj != Py_None)
	{
		program = DCShaderProgramToObject(programObj);
		if (program == NULL)
		{
			PyErr_SetString(PyExc_ValueError, "sixth argument must be ShaderProgram object.");
			return NULL;
		}
	}

	DKMaterial::RenderingProperty rp;
	rp.name = name;
	rp.depthFunc = (DKMaterial::RenderingProperty::DepthFunc)depthFunc;
	rp.depthWrite = depthWrite != 0;
	rp.blendState = *bs;
	rp.shaders = shaders;
	rp.program = program;
	auto index = self->material->renderingProperties.Add(rp);
	return PyLong_FromSize_t(index);
}

static PyObject* DCMaterialRemoveRenderingProperty(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	Py_ssize_t index;
	if (!PyArg_ParseTuple(args, "n", &index))
		return NULL;
	if (index < 0 || index >= self->material->renderingProperties.Count())
	{
		PyErr_SetString(PyExc_ValueError, "argument is out of range.");
		return NULL;
	}
	self->material->renderingProperties.Remove(index);
	self->UpdateShaders();
	Py_RETURN_NONE;
}

static PyObject* DCMaterialRemoveAllRenderingProperties(DCMaterial* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	self->material->renderingProperties.Clear();
	self->UpdateShaders();
	Py_RETURN_NONE;
}

static PyObject* DCMaterialBuildProgram(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	if (index < 0 || index >= self->material->renderingProperties.Count())
	{
		PyErr_SetString(PyExc_ValueError, "argument is out of range.");
		return NULL;
	}

	DKMaterial::BuildLog log;

	PyObject* dict = PyDict_New();
	bool result = false;
	Py_BEGIN_ALLOW_THREADS
	result = self->material->BuildProgram(index, &log);
	Py_END_ALLOW_THREADS

	PyObject* errorLog = PyUnicode_FromWideChar(log.errorLog, -1);
	PyObject* failedShader = PyUnicode_FromWideChar(log.failedShader, -1);
	PyObject* failedProgram = PyUnicode_FromWideChar(log.failedProgram, -1);

	PyDict_SetItemString(dict, "result", result ? Py_True : Py_False);
	PyDict_SetItemString(dict, "errorLog", errorLog);
	PyDict_SetItemString(dict, "failedShader", failedShader);
	PyDict_SetItemString(dict, "failedProgram", failedProgram);

	Py_DECREF(errorLog);
	Py_DECREF(failedShader);
	Py_DECREF(failedProgram);

	return dict;
}

static PyObject* DCMaterialBuild(DCMaterial* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->material, NULL);

	DKMaterial::BuildLog log;

	PyObject* dict = PyDict_New();
	bool result = false;
	Py_BEGIN_ALLOW_THREADS
	result = self->material->Build(&log);
	Py_END_ALLOW_THREADS

	PyObject* errorLog = PyUnicode_FromWideChar(log.errorLog, -1);
	PyObject* failedShader = PyUnicode_FromWideChar(log.failedShader, -1);
	PyObject* failedProgram = PyUnicode_FromWideChar(log.failedProgram, -1);

	PyDict_SetItemString(dict, "result", result ? Py_True : Py_False);
	PyDict_SetItemString(dict, "errorLog", errorLog);
	PyDict_SetItemString(dict, "failedShader", failedShader);
	PyDict_SetItemString(dict, "failedProgram", failedProgram);

	Py_DECREF(errorLog);
	Py_DECREF(failedShader);
	Py_DECREF(failedProgram);

	return dict;
}

static PyObject* DCMaterialValidate(DCMaterial* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->material, NULL);
	bool result = false;
	Py_BEGIN_ALLOW_THREADS
	result = self->material->Validate();
	Py_END_ALLOW_THREADS
	return PyBool_FromLong(result);
}

static PyObject* DCMaterialIsValid(DCMaterial* self, PyObject*)
{
	bool b = false;
	if (self->material)
		b = self->material->IsValid();
	return PyBool_FromLong(b);
}

static PyMethodDef methods[] = {
	/* material->shadingProperties */
	{ "shadingProperty", (PyCFunction)&DCMaterialShadingProperty, METH_VARARGS },
	{ "setShadingProperty", (PyCFunction)&DCMaterialSetShadingProperty, METH_VARARGS | METH_KEYWORDS },
	{ "shadingPropertyNames", (PyCFunction)&DCMaterialShadingPropertyNames, METH_NOARGS },

	/* material->samplerProperties */
	{ "samplerProperty", (PyCFunction)&DCMaterialSamplerProperty, METH_VARARGS },
	{ "setSamplerProperty", (PyCFunction)&DCMaterialSetSamplerProperty, METH_VARARGS | METH_KEYWORDS },
	{ "samplerPropertyNames", (PyCFunction)&DCMaterialSamplerPropertyNames, METH_NOARGS },

	/* material->streamProperties */
	{ "streamProperty", (PyCFunction)&DCMaterialStreamProperty, METH_VARARGS },
	{ "setStreamProperty", (PyCFunction)&DCMaterialSetStreamProperty, METH_VARARGS | METH_KEYWORDS },
	{ "streamPropertyNames", (PyCFunction)&DCMaterialStreamPropertyNames, METH_NOARGS },

	/* material->shaderProperties */
	{ "numberOfShaderProperties", (PyCFunction)&DCMaterialNumberOfShaderProperties, METH_NOARGS },
	{ "shaderProperty", (PyCFunction)&DCMaterialShaderProperty, METH_VARARGS },
	{ "setShaderProperty", (PyCFunction)&DCMaterialSetShaderProperty, METH_VARARGS | METH_KEYWORDS },
	{ "addShaderProperty", (PyCFunction)&DCMaterialAddShaderProperty, METH_VARARGS | METH_KEYWORDS },
	{ "removeShaderProperty", (PyCFunction)&DCMaterialRemoveShaderProperty, METH_VARARGS },
	{ "removeAllShaderProperties", (PyCFunction)&DCMaterialRemoveAllShaderProperties, METH_NOARGS },

	/* material->renderingProperties */
	{ "numberOfRenderingProperties", (PyCFunction)&DCMaterialNumberOfRenderingProperties, METH_NOARGS },
	{ "renderingProperty", (PyCFunction)&DCMaterialRenderingProperty, METH_VARARGS },
	{ "setRenderingProperty", (PyCFunction)&DCMaterialSetRenderingProperty, METH_VARARGS | METH_KEYWORDS },
	{ "addRenderingProperty", (PyCFunction)&DCMaterialAddRenderingProperty, METH_VARARGS | METH_KEYWORDS },
	{ "removeRenderingProperty", (PyCFunction)&DCMaterialRemoveRenderingProperty, METH_VARARGS },
	{ "removeAllRenderingProperties", (PyCFunction)&DCMaterialRemoveAllRenderingProperties, METH_NOARGS },

	{ "buildProgram", (PyCFunction)&DCMaterialBuildProgram, METH_VARARGS },
	{ "build", (PyCFunction)&DCMaterialBuild, METH_NOARGS },
	{ "validate", (PyCFunction)&DCMaterialValidate, METH_NOARGS },
	{ "isValid", (PyCFunction)&DCMaterialIsValid, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Material",				/* tp_name */
	sizeof(DCMaterial),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCMaterialDealloc,				/* tp_dealloc */
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
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCMaterialTraverse,			/* tp_traverse */
	(inquiry)&DCMaterialClear,					/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	0,											/* tp_getset */
	DCResourceTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCMaterialInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCMaterialNew,								/* tp_new */
};

PyTypeObject* DCMaterialTypeObject(void)
{
	return &objectType;
}

PyObject* DCMaterialFromObject(DKMaterial* material)
{
	if (material)
	{
		DCMaterial* self = (DCMaterial*)DCObjectFromAddress(material);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCMaterial*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->material = material;
				DCObjectSetAddress(self->material, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKMaterial* DCMaterialToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCMaterial*)obj)->material;
	}
	return NULL;
}
