#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCRenderer
{
	PyObject_HEAD
	DKObject<DKRenderer> renderer;
	PyObject* renderTarget;
	void UpdateRenderTarget(void)
	{
		PyObject* old = this->renderTarget;
		this->renderTarget = NULL;
		DKRenderTarget* target = renderer->RenderTarget();
		if (target)
		{
			PyObject* obj = DCRenderTargetFromObject(target);
			DKASSERT_DEBUG(DCRenderTargetToObject(obj) == target);
			this->renderTarget = obj;
		}
		Py_XDECREF(old);
	}
};

static PyObject* DCRendererNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCRenderer* self = (DCRenderer*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->renderer) DKObject<DKRenderer>();
		self->renderTarget = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCRendererClear(DCRenderer* self)
{
	Py_CLEAR(self->renderTarget);
	return 0;
}

static int DCRendererTraverse(DCRenderer* self, visitproc visit, void* arg)
{
	Py_VISIT(self->renderTarget);
	return 0;
}

static int DCRendererInit(DCRenderer *self, PyObject *args, PyObject *kwds)
{
	if (self->renderer == NULL)
	{
		PyObject* obj;
		if (!PyArg_ParseTuple(args, "O", &obj))
			return -1;

		DKRenderTarget* target = DCRenderTargetToObject(obj);
		if (target == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "argument must be RenderTarget object.");
			return -1;
		}

		self->renderer = DKObject<DKRenderer>::New(target);
		DCObjectSetAddress(self->renderer, (PyObject*)self);
	}
	self->UpdateRenderTarget();
	return 0;
}

static void DCRendererDealloc(DCRenderer* self)
{
	DCRendererClear(self);

	if (self->renderer)
	{
		DCObjectSetAddress(self->renderer, NULL);
		self->renderer = NULL;
	}
	self->renderer.~DKObject<DKRenderer>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCRendererRepr(DCRenderer* self)
{
	DKString str = DKString::Format("<%s object>",
		Py_TYPE(self)->tp_name);
	return PyUnicode_FromWideChar((const wchar_t*)str, -1);
}

static PyObject* DCRendererClearTarget(DCRenderer* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	DKColor color;
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCColorConverter(args, &color))
	{
		self->renderer->Clear(color);
		Py_RETURN_NONE;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCColorConverter, &color))
	{
		self->renderer->Clear(color);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Color object.");
	return NULL;
}

static PyObject* DCRendererClearColorBuffer(DCRenderer* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	DKColor color;
	Py_ssize_t numArgs = PyTuple_GET_SIZE(args);
	if (numArgs > 1 && DCColorConverter(args, &color))
	{
		self->renderer->ClearColorBuffer(color);
		Py_RETURN_NONE;
	}
	else if (numArgs == 1 && PyArg_ParseTuple(args, "O&", &DCColorConverter, &color))
	{
		self->renderer->ClearColorBuffer(color);
		Py_RETURN_NONE;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Color object.");
	return NULL;
}

static PyObject* DCRendererClearDepthBuffer(DCRenderer* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	self->renderer->ClearDepthBuffer();
	Py_RETURN_NONE;
}

static PyObject* DCRendererRenderSolidRects(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer rects, trans;
	Py_ssize_t count;
	DKColor color;
	PyObject* blend;

	char* kwlist[] = { "rects", "transforms", "count", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*y*nO&O", kwlist,
		&rects, &trans, &count, &DCColorConverter, &color, &blend))
		return NULL;

	DCBufferRelease _tmp[] = { &rects, &trans };

	size_t numRects = rects.len / sizeof(DKRect);
	size_t numTrans = trans.len / sizeof(DKMatrix3);
	if (count < 0 || count > Min(numRects, numTrans))
	{
		PyErr_SetString(PyExc_ValueError, "third argument (count) is out of range.");
		return NULL;
	}
	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "fifth argument (blend) must be BlendState object.");
		return NULL;
	}
	if (count > 0)
		self->renderer->RenderSolidRects((DKRect*)rects.buf, (DKMatrix3*)trans.buf, count, color, *bs);
	Py_RETURN_NONE;
}

template <void (DKRenderer::*proc)(const DKPoint*, size_t, const DKColor&, const DKBlendState&) const>
static PyObject* DCRendererRenderSolid(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer points;
	Py_ssize_t count;
	DKColor color;
	PyObject* blend;

	char* kwlist[] = { "vertices", "count", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*nO&O", kwlist,
		&points, &count, &DCColorConverter, &color, &blend))
		return NULL;

	DCBufferRelease _tmp[] = { &points };

	size_t numVerts = points.len / sizeof(DKPoint);
	if (count < 0 || count > numVerts)
	{
		PyErr_SetString(PyExc_ValueError, "second argument (count) is out of range.");
		return NULL;
	}

	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "fourth argument (blend) must be BlendState object.");
		return NULL;
	}

	if (count > 0)
		(self->renderer->*proc)((const DKPoint*)points.buf, count, color, *bs);
	Py_RETURN_NONE;
}

static PyObject* DCRendererRenderSolidEllipses(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer bounds;
	Py_ssize_t count;
	DKColor color;
	PyObject* blend;

	char* kwlist[] = { "bounds", "count", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*nO&O", kwlist,
		&bounds, &count, &DCColorConverter, &color, &blend))
		return NULL;

	DCBufferRelease _tmp[] = { &bounds };

	size_t numRects = bounds.len / sizeof(DKRect);
	if (count < 0 || count > numRects)
	{
		PyErr_SetString(PyExc_ValueError, "second argument (count) is out of range.");
		return NULL;
	}
	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "fourth argument (blend) must be BlendState object.");
		return NULL;
	}
	DKRect* rc = (DKRect*)bounds.buf;
	for (size_t i = 0; i < count; ++i)
	{
		self->renderer->RenderSolidEllipse(rc[i], color, *bs);
	}
	Py_RETURN_NONE;
}

static PyObject* DCRendererRenderTexturedRects(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer posRects, posTrans, texRects, texTrans;
	Py_ssize_t count;
	PyObject* texture;
	PyObject* sampler;
	DKColor color;
	PyObject* blend;

	char* kwlist[] = { "rects", "trans", "texRects", "texTrans", "count", "texture", "sampler", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*y*y*y*nOOO&O", kwlist,
		&posRects, &posTrans, &texRects, &texTrans,
		&count,
		&texture, &sampler, &DCColorConverter, &color, &blend))
		return NULL;

	DCBufferRelease _tmp[] = {&posRects, &posTrans, &texRects, &texTrans};

	size_t numRects = posRects.len / sizeof(DKRect);
	size_t numPosTM = posTrans.len / sizeof(DKMatrix3);
	size_t numTexes = texRects.len / sizeof(DKRect);
	size_t numTexTM = texTrans.len / sizeof(DKMatrix3);
	if (count < 0 || count > Min(Min(numRects, numPosTM), Min(numTexes, numTexTM)))
	{
		PyErr_SetString(PyExc_ValueError, "fifth argument (count) is out of range.");
		return NULL;
	}
	DKTexture* tex = DCTextureToObject(texture);
	if (tex == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "sixth argument (texture) must be Texture object.");
		return NULL;
	}
	DKTextureSampler* ts = NULL;
	if (sampler != Py_None)
	{
		ts = DCTextureSamplerToObject(sampler);
		if (ts == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "seventh argument (sampler) must be TextureSampler object or None.");
			return NULL;
		}
	}
	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "ninth argument (blend) must be BlendState object.");
		return NULL;
	}
	if (count > 0)
		self->renderer->RenderTexturedRects(
		(DKRect*)posRects.buf,
		(DKMatrix3*)posTrans.buf,
		(DKRect*)texRects.buf,
		(DKMatrix3*)texTrans.buf,
		count, tex, ts, color, *bs);

	Py_RETURN_NONE;
}

template <void (DKRenderer::*proc)(const DKRenderer::Vertex2D*, size_t, const DKTexture*, const DKTextureSampler*, const DKColor&, const DKBlendState&) const>
static PyObject* DCRendererRenderTextured(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer points;
	Py_ssize_t count;
	PyObject* texture;
	PyObject* sampler;
	DKColor color;
	PyObject* blend;

	char* kwlist[] = { "vertices", "count", "texture", "sampler", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*nOOO&O", kwlist,
		&points, &count, &texture, &sampler, &DCColorConverter, &color, &blend))
		return NULL;

	DCBufferRelease _tmp[] = { &points };

	size_t numPoints = points.len / sizeof(DKRenderer::Vertex2D);
	if (count < 0 || count > numPoints)
	{
		PyErr_SetString(PyExc_ValueError, "second argument (count) is out of range.");
		return NULL;
	}
	DKTexture* tex = DCTextureToObject(texture);
	if (tex == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "third argument (texture) must be Texture object.");
		return NULL;
	}
	DKTextureSampler* ts = NULL;
	if (sampler != Py_None)
	{
		ts = DCTextureSamplerToObject(sampler);
		if (ts == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "fourth argument (sampler) must be TextureSampler object or None.");
			return NULL;
		}
	}
	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "sixth argument (blend) must be BlendState object.");
		return NULL;
	}
	if (count > 0)
		(self->renderer->*proc)(reinterpret_cast<DKRenderer::Vertex2D*>(points.buf), count, tex, ts, color, *bs);
	Py_RETURN_NONE;
}

static PyObject* DCRendererRenderTexturedEllipses(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer bounds;
	Py_buffer texBounds;
	Py_ssize_t count;
	PyObject* texture;
	PyObject* sampler;
	DKColor color;
	PyObject* blend;

	char* kwlist[] = { "bounds", "texBounds", "count", "texture", "sampler", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*y*nOOO&O", kwlist,
		&bounds, &texBounds, &count, &texture, &sampler, &DCColorConverter, &color, &blend))
		return NULL;

	DCBufferRelease _tmp[] = { &bounds, &texBounds };

	size_t numRects = bounds.len / sizeof(DKRect);
	size_t numTexRects = texBounds.len / sizeof(DKRect);
	DKRect* rects = (DKRect*)bounds.buf;
	DKRect* texRects = (DKRect*)texBounds.buf;
	if (count < 0 || count > Min(numRects, numTexRects))
	{
		PyErr_SetString(PyExc_ValueError, "third argument (count) is out of range.");
		return NULL;
	}
	DKTexture* tex = DCTextureToObject(texture);
	if (tex == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "fourth argument (texture) must be Texture object.");
		return NULL;
	}
	DKTextureSampler* ts = NULL;
	if (sampler != Py_None)
	{
		ts = DCTextureSamplerToObject(sampler);
		if (ts == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "fifth argument (sampler) must be TextureSampler object or None.");
			return NULL;
		}
	}
	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "seventh argument (blend) must be BlendState object.");
		return NULL;
	}
	for (size_t i = 0; i < count; ++i)
	{
		self->renderer->RenderTexturedEllipse(rects[i], texRects[i], tex, ts, color, *bs);
	}
	Py_RETURN_NONE;
}

static PyObject* DCRendererRenderTextBaseline(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);
	DKPoint lineBegin;
	DKPoint lineEnd;
	const char* text = "";
	PyObject* fontObj = NULL;
	DKColor color;
	PyObject* blendObj = NULL;

	char* kwlist[] = { "begin", "end", "text", "font", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&O&sOO&O", kwlist,
		&DCPointConverter, &lineBegin,
		&DCPointConverter, &lineEnd,
		&text,
		&fontObj,
		&DCColorConverter, &color,
		&blendObj))
		return NULL;

	if (text == NULL || text[0] == 0)
	{
		Py_RETURN_NONE;
	}

	DKFont* font = DCFontToObject(fontObj);
	if (font == NULL)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "fourth argument must be Font object.");
		return NULL;
	}
	DKBlendState* blend = DCBlendStateToObject(blendObj);
	if (blend == NULL)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "fifth argument must be BlendState object.");
		return NULL;
	}

	self->renderer->RenderText(lineBegin, lineEnd, text, font, color, *blend);
	Py_RETURN_NONE;
}

static PyObject* DCRendererRenderTextRect(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);
	DKRect bounds;
	DKMatrix3 transform;
	const char* text = "";
	PyObject* fontObj = NULL;
	DKColor color;
	PyObject* blendObj = NULL;

	char* kwlist[] = { "bounds", "transform", "text", "font", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&O&sOO&O", kwlist,
		&DCRectConverter, &bounds,
		&DCMatrix3Converter, &transform,
		&text,
		&fontObj,
		&DCColorConverter, &color,
		&blendObj))
		return NULL;

	if (text == NULL || text[0] == 0)
	{
		Py_RETURN_NONE;
	}

	DKFont* font = DCFontToObject(fontObj);
	if (font == NULL)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "fourth argument must be Font object.");
		return NULL;
	}
	DKBlendState* blend = DCBlendStateToObject(blendObj);
	if (blend == NULL)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "fifth argument must be BlendState object.");
		return NULL;
	}

	self->renderer->RenderText(bounds, transform, text, font, color, *blend);
	Py_RETURN_NONE;
}

template <void (DKRenderer::*proc)(const DKVector3*, size_t, const DKMatrix4&, const DKColor&, const DKBlendState&) const>
static PyObject* DCRendererRenderSolid3D(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer vertices;
	Py_ssize_t count;
	DKMatrix4 transform;
	DKColor color;
	PyObject* blend;

	char* kwlist[] = { "vertices", "count", "transform", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*nO&O&O", kwlist,
		&vertices, &count, &DCMatrix4Converter, &transform, &DCColorConverter, &color, &blend))
		return NULL;

	DCBufferRelease _tmp[] = { &vertices };

	size_t numVectors = vertices.len / sizeof(DKVector3);
	if (count < 0 || count > numVectors)
	{
		PyErr_SetString(PyExc_ValueError, "second argument (count) is out of range.");
		return NULL;
	}
	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "fifth argument (blend) must be BlendState object.");
		return NULL;
	}
	if (count > 0)
		(self->renderer->*proc)((DKVector3*)vertices.buf, count, transform, color, *bs);
	Py_RETURN_NONE;
}

template <void (DKRenderer::*proc)(const DKRenderer::Vertex3DColored*, size_t, const DKMatrix4&, const DKBlendState&) const>
static PyObject* DCRendererRenderColored3D(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer vertices;
	Py_ssize_t count;
	DKMatrix4 transform;
	PyObject* blend;

	char* kwlist[] = { "vertices", "count", "transform", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*nO&O", kwlist,
		&vertices, &count, &DCMatrix4Converter, &transform, &blend))
		return NULL;

	DCBufferRelease _tmp[] = { &vertices };

	size_t numVectors = vertices.len / sizeof(DKRenderer::Vertex3DColored);
	if (count < 0 || count > numVectors)
	{
		PyErr_SetString(PyExc_ValueError, "second argument (count) is out of range.");
		return NULL;
	}
	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "fourth argument (blend) must be BlendState object.");
		return NULL;
	}
	if (count > 0)
		(self->renderer->*proc)((DKRenderer::Vertex3DColored*)vertices.buf, count, transform, *bs);
	Py_RETURN_NONE;
}

template <void (DKRenderer::*proc)(const DKRenderer::Vertex3DTextured*, size_t, const DKMatrix4&, const DKTexture*, const DKTextureSampler*, const DKColor&, const DKBlendState&) const>
static PyObject* DCRendererRenderTextured3D(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	Py_buffer vertices;
	Py_ssize_t count;
	DKMatrix4 transform;
	PyObject* texture;
	PyObject* sampler;
	DKColor color;
	PyObject* blend;

	char* kwlist[] = { "vertices", "count", "transform", "texture", "sampler", "color", "blend", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*nO&OOO&O", kwlist,
		&vertices, &count, &DCMatrix4Converter, &transform, &texture, &sampler, &DCColorConverter, &color, &blend))
		return NULL;

	DCBufferRelease _tmp[] = { &vertices };

	size_t numVectors = vertices.len / sizeof(DKRenderer::Vertex3DTextured);
	if (count < 0 || count > numVectors)
	{
		PyErr_SetString(PyExc_ValueError, "second argument (count) is out of range.");
		return NULL;
	}
	DKTexture* tex = DCTextureToObject(texture);
	if (tex == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "fourth argument (texture) must be Texture object.");
		return NULL;
	}
	DKTextureSampler* ts = NULL;
	if (sampler != Py_None)
	{
		ts = DCTextureSamplerToObject(sampler);
		if (ts == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "fifth argument (sampler) must be TextureSampler object or None.");
			return NULL;
		}
	}
	DKBlendState* bs = DCBlendStateToObject(blend);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "seventh argument (blend) must be BlendState object.");
		return NULL;
	}
	if (count > 0)
		(self->renderer->*proc)((DKRenderer::Vertex3DTextured*)vertices.buf, count, transform, tex, ts, color, *bs);
	Py_RETURN_NONE;
}

static PyObject* DCRendererRenderScene(DCRenderer* self, PyObject* args, PyObject* kwargs)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);
	PyObject* sceneObj;
	PyObject* cameraObj;
	int sceneIndex;
	int enableCulling = 1;
	PyObject* materialDict = NULL;
	PyObject* filterCb = NULL;
	PyObject* colorCb = NULL;

	char* kwlist[] = { "scene", "camera", "sceneIndex", "enableCulling", "materialInfo", "meshFilter", "objectColorCallback", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOi|p$OOO", kwlist,
		&sceneObj, &cameraObj, &sceneIndex, &enableCulling, &materialDict, &filterCb, &colorCb))
		return NULL;

	DKScene* scene = DCSceneToObject(sceneObj);
	if (scene == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "first argument must be Scene object.");
		return NULL;
	}
	DKCamera* camera = DCCameraToObject(cameraObj);
	if (camera == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "second argument must be Camera object.");
		return NULL;
	}
	DKObject<DKMaterial::PropertyCallback> matCallback = NULL;
	if (materialDict)
	{
		if (!PyMapping_Check(materialDict))
		{
			PyErr_SetString(PyExc_TypeError, "optional keyword argument 'materialInfo' must be mapping object or None.");
			return NULL;
		}
		struct MatCallback : public DKMaterial::PropertyCallback
		{
			PyObject* dict;
			DKMap<DKString, DKArray<int>> cachedIntValues;
			DKMap<DKString, DKArray<float>> cachedFloatValues;
			DKMap<DKString, Sampler> cachedSamplerValues;
			bool noException;

			PyObject* GetFastSequenceObject(const DKString& name)
			{
				PyObject* value = PyMapping_GetItemString(this->dict, (char*)(const char*)DKStringU8(name));
				if (value)
				{
					PyObject* seq = PySequence_Fast(value, "materialInfo value must be sequence object.");
					Py_DECREF(value);
					return seq;
				}
				PyErr_Clear();
				return NULL;
			}
			IntArray GetIntProperty(const DKShaderConstant& sc, int) override
			{
				auto p = this->cachedIntValues.Find(sc.name);
				if (p)
					return IntArray((int*)p->value, p->value.Count());

				DKArray<int>& values = this->cachedIntValues.Value(sc.name);
				PyGILState_STATE st = PyGILState_Ensure();
				if (!PyErr_Occurred())
				{
					PyObject* seq = GetFastSequenceObject(sc.name);
					if (seq)
					{
						Py_ssize_t count = PySequence_Fast_GET_SIZE(seq);
						PyObject** items = PySequence_Fast_ITEMS(seq);
						for (Py_ssize_t i = 0; i < count; ++i)
						{
							long val = PyLong_AsLong(items[i]);
							if (PyErr_Occurred())
							{
								PyErr_Format(PyExc_TypeError, "materialInfo[%s] items should be integer sequence.", (const char*)DKStringU8(sc.name));
								values.Clear();
								break;
							}
							else
							{
								values.Add((int)val);
							}
						}
						Py_DECREF(seq);
					}
					if (this->noException)
						PyErr_Clear();
				}
				PyGILState_Release(st);
				return IntArray((int*)values, values.Count());
			}
			FloatArray GetFloatProperty(const DKShaderConstant& sc, int) override
			{
				auto p = this->cachedFloatValues.Find(sc.name);
				if (p)
					return FloatArray((float*)p->value, p->value.Count());

				DKArray<float>& values = this->cachedFloatValues.Value(sc.name);
				PyGILState_STATE st = PyGILState_Ensure();
				if (!PyErr_Occurred())
				{
					PyObject* seq = GetFastSequenceObject(sc.name);
					if (seq)
					{
						Py_ssize_t count = PySequence_Fast_GET_SIZE(seq);
						PyObject** items = PySequence_Fast_ITEMS(seq);
						for (Py_ssize_t i = 0; i < count; ++i)
						{
							double val = PyFloat_AsDouble(items[i]);
							if (PyErr_Occurred())
							{
								PyErr_Format(PyExc_TypeError, "materialInfo[%s] items should be float sequence.", (const char*)DKStringU8(sc.name));
								values.Clear();
								break;
							}
							else
							{
								values.Add(val);
							}
						}
						Py_DECREF(seq);
					}
					if (this->noException)
						PyErr_Clear();
				}
				PyGILState_Release(st);
				return FloatArray((float*)values, values.Count());
			}
			const Sampler* GetSamplerProperty(const DKShaderConstant& sc, int) override
			{
				auto p = this->cachedSamplerValues.Find(sc.name);
				if (p)
					return &p->value;

				Sampler& sampler = this->cachedSamplerValues.Value(sc.name);
				PyGILState_STATE st = PyGILState_Ensure();
				if (!PyErr_Occurred())
				{
					PyObject* seq = GetFastSequenceObject(sc.name);
					if (seq)
					{
						Py_ssize_t count = PySequence_Fast_GET_SIZE(seq);
						PyObject** items = PySequence_Fast_ITEMS(seq);
						for (Py_ssize_t i = 0; i < count; ++i)
						{
							DKTexture* tex = DCTextureToObject(items[i]);
							if (tex)
							{
								sampler.textures.Add(tex);
							}
							else
							{
								DKTextureSampler* ts = DCTextureSamplerToObject(items[i]);
								if (ts)
								{
									sampler.sampler = ts;
								}
								else
								{
									PyErr_Format(PyExc_TypeError, "materialInfo[%s] items should be Texture or TextureSampler object sequence.", (const char*)DKStringU8(sc.name));
									sampler.textures.Clear();
									sampler.sampler = NULL;
									break;
								}
							}
						}
						Py_DECREF(seq);
					}
					if (this->noException)
						PyErr_Clear();
				}
				PyGILState_Release(st);
				return &sampler;
			}
		};
		DKObject<MatCallback> cb = DKOBJECT_NEW MatCallback();
		cb->dict = materialDict;
		cb->noException = false;
		matCallback = cb.SafeCast<DKMaterial::PropertyCallback>();
	}

	DKObject<DKRenderer::RenderSceneCallback::MeshFilter> filter = NULL;
	if (filterCb && filterCb != Py_None)
	{
		if (!PyCallable_Check(filterCb))
		{
			PyErr_SetString(PyExc_TypeError, "optional keyword argument 'meshFilter' must be callable or None.");
			return NULL;
		}

		auto cb = [filterCb](DKArray<const DKMesh*>& meshes)
		{
			PyGILState_STATE st = PyGILState_Ensure();

			DKArray<PyObject*> meshObjArray;
			meshObjArray.Reserve(meshes.Count());
			for (const DKMesh* m : meshes)
			{
				PyObject* obj = DCObjectFromAddress(m);
				if (obj)
				{
					Py_INCREF(obj);		// 레퍼런스는 나중에 PyList_SET_ITEM 함수가 가져간다.
					DKASSERT_DEBUG(DCMeshToObject(obj) == m);
					meshObjArray.Add(obj);
				}
			}

			PyObject* meshList = PyList_New(meshObjArray.Count());
			for (Py_ssize_t i = 0; i < meshObjArray.Count(); ++i)
			{
				PyObject* obj = meshObjArray.Value(i);
				PyList_SET_ITEM(meshList, i, obj);
			}
			PyObject* ret = PyObject_CallFunction(filterCb, "N", meshList);

			meshes.Clear();

			if (ret)
			{
				if (PySequence_Check(ret))
				{
					Py_ssize_t len = PySequence_Size(ret);
					meshes.Reserve(len);
					for (Py_ssize_t i = 0; i < len; ++i)
					{
						PyObject* obj = PySequence_ITEM(ret, i);
						if (obj)
						{
							DKMesh* mesh = DCMeshToObject(obj);
							if (mesh)
								meshes.Add(mesh);
							Py_DECREF(obj);
						}
					}
				}
				else if (ret != Py_None)
				{
					PyErr_Clear();
					PyErr_SetString(PyExc_TypeError, "filter callable must return Mesh Sequence object or None.");
				}
				Py_DECREF(ret);
			}
			PyGILState_Release(st);
		};
		filter = DKFunction(cb);
	}
	DKObject<DKRenderer::RenderSceneCallback::ObjectColorCallback> objectColorsCb = NULL;

	if (colorCb && colorCb != Py_None)
	{
		if (!PyCallable_Check(colorCb))
		{
			PyErr_SetString(PyExc_TypeError, "optional keyword argument 'objectColorCallback' must be callable or None.");
			return NULL;
		}
		auto cb = [colorCb](const DKCollisionObject* co, DKColor& fc, DKColor& ec)->bool
		{
			bool result = false;

			PyGILState_STATE st = PyGILState_Ensure();
			if (!PyErr_Occurred())
			{
				PyObject* col = DCObjectFromAddress(co);
				if (col)
				{
					Py_INCREF(col);
					PyObject* ret = PyObject_CallFunction(colorCb, "ONN", col, DCColorFromObject(&fc), DCColorFromObject(&ec));
					if (ret && ret != Py_None)
					{
						if (PySequence_Check(ret))
						{
							Py_ssize_t len = PySequence_Size(ret);
							DKColor* colors[] = { &fc, &ec };
							for (Py_ssize_t i = 0; i < len && i < 2; ++i)
							{
								PyObject* obj = PySequence_ITEM(ret, i);
								DCObjectRelease _tmp[] = { obj };

								if (obj == NULL)
									break;
								if (obj != Py_None)
								{
									DKColor* c = DCColorToObject(obj);
									if (c)
									{
										*(colors[i]) = *c;
										result = true;
									}
									else
									{
										PyErr_Clear();
										PyErr_SetString(PyExc_TypeError, "objectColorCallback callable must return Color objects or None.");
										result = false;
										break;
									}
								}
							}
						}
						else
						{
							DKColor* c = DCColorToObject(ret);
							if (c)
							{
								fc = *c;
								result = true;
							}
							else
							{
								PyErr_Clear();
								PyErr_SetString(PyExc_TypeError, "objectColorCallback callable must return Color objects or None.");
								result = false;
							}
						}
					}
					Py_XDECREF(ret);
					Py_DECREF(col);
				}
			}

			PyGILState_Release(st);
			return result;
		};
		objectColorsCb = DKFunction(cb);
	}

	Py_BEGIN_ALLOW_THREADS

	DKRenderer::RenderSceneCallback cb;
	cb.materialCallback = matCallback;
	cb.meshFilter = filter;
	cb.objectColors = objectColorsCb;

	self->renderer->RenderScene(scene, *camera, sceneIndex, enableCulling != 0, &cb);

	Py_END_ALLOW_THREADS

	if (PyErr_Occurred())
		return NULL;

	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{ "clear", (PyCFunction)&DCRendererClearTarget, METH_VARARGS },
	{ "clearColorBuffer", (PyCFunction)&DCRendererClearColorBuffer, METH_VARARGS },
	{ "clearDepthBuffer", (PyCFunction)&DCRendererClearDepthBuffer, METH_NOARGS },

	// 2D 단색 렌더링
	{ "renderSolidRects", (PyCFunction)&DCRendererRenderSolidRects, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolidQuads", (PyCFunction)&DCRendererRenderSolid<&DKRenderer::RenderSolidQuads>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolidTriangles", (PyCFunction)&DCRendererRenderSolid<&DKRenderer::RenderSolidTriangles>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolidTriangleStrip", (PyCFunction)&DCRendererRenderSolid<&DKRenderer::RenderSolidTriangleStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolidTriangleFan", (PyCFunction)&DCRendererRenderSolid<&DKRenderer::RenderSolidTriangleFan>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolidLines", (PyCFunction)&DCRendererRenderSolid<&DKRenderer::RenderSolidLines>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolidLineStrip", (PyCFunction)&DCRendererRenderSolid<&DKRenderer::RenderSolidLineStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolidPoints", (PyCFunction)&DCRendererRenderSolid<&DKRenderer::RenderSolidPoints>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolidEllipses", (PyCFunction)&DCRendererRenderSolidEllipses, METH_VARARGS | METH_KEYWORDS },

	// 2D 텍스쳐 렌더링
	{ "renderTexturedRects", (PyCFunction)&DCRendererRenderTexturedRects, METH_VARARGS | METH_KEYWORDS },
	{ "renderTexturedQuads", (PyCFunction)&DCRendererRenderTextured<&DKRenderer::RenderTexturedQuads>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTexturedTriangles", (PyCFunction)&DCRendererRenderTextured<&DKRenderer::RenderTexturedTriangles>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTexturedTriangleStrip", (PyCFunction)&DCRendererRenderTextured<&DKRenderer::RenderTexturedTriangleStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTexturedTriangleFan", (PyCFunction)&DCRendererRenderTextured<&DKRenderer::RenderTexturedTriangleFan>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTexturedLines", (PyCFunction)&DCRendererRenderTextured<&DKRenderer::RenderTexturedLines>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTexturedLineStrip", (PyCFunction)&DCRendererRenderTextured<&DKRenderer::RenderTexturedLineStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTexturedPoints", (PyCFunction)&DCRendererRenderTextured<&DKRenderer::RenderTexturedPoints>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTexturedEllipses", (PyCFunction)&DCRendererRenderTexturedEllipses, METH_VARARGS | METH_KEYWORDS },

	// 2D 텍스트 렌더링
	{ "renderTextBaseline", (PyCFunction)&DCRendererRenderTextBaseline, METH_VARARGS | METH_KEYWORDS },
	{ "renderTextRect", (PyCFunction)&DCRendererRenderTextRect, METH_VARARGS | METH_KEYWORDS },

	// 3D 단색 도형 렌더링
	{ "renderSolid3DTriangles", (PyCFunction)&DCRendererRenderSolid3D<&DKRenderer::RenderSolidTriangles>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolid3DTriangleStrip", (PyCFunction)&DCRendererRenderSolid3D<&DKRenderer::RenderSolidTriangleStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolid3DTriangleFan", (PyCFunction)&DCRendererRenderSolid3D<&DKRenderer::RenderSolidTriangleFan>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolid3DLines", (PyCFunction)&DCRendererRenderSolid3D<&DKRenderer::RenderSolidLines>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolid3DLineStrip", (PyCFunction)&DCRendererRenderSolid3D<&DKRenderer::RenderSolidLineStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderSolid3DPoints", (PyCFunction)&DCRendererRenderSolid3D<&DKRenderer::RenderSolidPoints>, METH_VARARGS | METH_KEYWORDS },

	// 3D 컬러 도형 렌더링
	{ "renderColored3DTriangles", (PyCFunction)&DCRendererRenderColored3D<&DKRenderer::RenderColoredTriangles>, METH_VARARGS | METH_KEYWORDS },
	{ "renderColored3DTriangleStrip", (PyCFunction)&DCRendererRenderColored3D<&DKRenderer::RenderColoredTriangleStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderColored3DTriangleFan", (PyCFunction)&DCRendererRenderColored3D<&DKRenderer::RenderColoredTriangleFan>, METH_VARARGS | METH_KEYWORDS },
	{ "renderColored3DLines", (PyCFunction)&DCRendererRenderColored3D<&DKRenderer::RenderColoredLines>, METH_VARARGS | METH_KEYWORDS },
	{ "renderColored3DLineStrip", (PyCFunction)&DCRendererRenderColored3D<&DKRenderer::RenderColoredLineStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderColored3DPoints", (PyCFunction)&DCRendererRenderColored3D<&DKRenderer::RenderColoredPoints>, METH_VARARGS | METH_KEYWORDS },

	// 3D 텍스쳐 도형 렌더링
	{ "renderTextured3DTriangles", (PyCFunction)&DCRendererRenderTextured3D<&DKRenderer::RenderTexturedTriangles>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTextured3DTriangleStrip", (PyCFunction)&DCRendererRenderTextured3D<&DKRenderer::RenderTexturedTriangleStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTextured3DTriangleFan", (PyCFunction)&DCRendererRenderTextured3D<&DKRenderer::RenderTexturedTriangleFan>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTextured3DLines", (PyCFunction)&DCRendererRenderTextured3D<&DKRenderer::RenderTexturedLines>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTextured3DLineStrip", (PyCFunction)&DCRendererRenderTextured3D<&DKRenderer::RenderTexturedLineStrip>, METH_VARARGS | METH_KEYWORDS },
	{ "renderTextured3DPoints", (PyCFunction)&DCRendererRenderTextured3D<&DKRenderer::RenderTexturedPoints>, METH_VARARGS | METH_KEYWORDS },

	// 3D scene 렌더링
	{ "renderScene", (PyCFunction)&DCRendererRenderScene, METH_VARARGS | METH_KEYWORDS },

	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCRendererViewport(DCRenderer* self, void*)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	const DKRect& rc = self->renderer->Viewport();
	return Py_BuildValue("ffff", rc.origin.x, rc.origin.y, rc.size.width, rc.size.height);
}

static int DCRendererSetViewport(DCRenderer* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->renderer, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKRect rc;
	if (DCRectConverter(value, &rc))
	{
		self->renderer->SetViewport(rc);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Rect or tuple of four floating point numbers (x, y, width, height).");
	return -1;
}

static PyObject* DCRendererBounds(DCRenderer* self, void*)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	const DKRect& bounds = self->renderer->ContentBounds();
	return Py_BuildValue("ffff", bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
}

static int DCRendererSetBounds(DCRenderer* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->renderer, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKRect rc;
	if (DCRectConverter(value, &rc))
	{
		float minScale = DKRenderer::minimumScaleFactor;
		if (rc.size.width < minScale || rc.size.height < minScale)
		{
			PyErr_SetString(PyExc_ValueError, "invalid attribute value.");
			return -1;
		}
		self->renderer->SetContentBounds(rc);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Size or tuple of two floating point numbers (width, height).");
	return -1;
}

static PyObject* DCRendererTransform(DCRenderer* self, void*)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	const DKMatrix3& mat = self->renderer->ContentTransform();
	return Py_BuildValue("fffffffff",
		mat.m[0][0], mat.m[0][1], mat.m[0][2],
		mat.m[1][0], mat.m[1][1], mat.m[1][2],
		mat.m[2][0], mat.m[2][1], mat.m[2][2]);
}

static int DCRendererSetTransform(DCRenderer* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->renderer, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKMatrix3 mat;
	if (DCMatrix3Converter(value, &mat))
	{
		self->renderer->SetContentTransform(mat);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Matrix3.");
	return -1;
}

static PyObject* DCRendererPolygonOffset(DCRenderer* self, void*)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);

	float offset[2] = {0.0f, 0.0f};
	self->renderer->PolygonOffset(offset);
	return Py_BuildValue("ff", offset[0], offset[1]);
}

static int DCRendererSetPolygonOffset(DCRenderer* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->renderer, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	if (PySequence_Check(value) && PySequence_Size(value) == 2)
	{
		bool err = false;
		float offset[2] = {0.0f, 0.0f};
		for (int i = 0; i < 2; ++i)
		{
			PyObject* obj = PySequence_GetItem(value, i);
			offset[i] = PyFloat_AsDouble(obj);
			Py_XDECREF(obj);
			if (PyErr_Occurred())
			{
				err = true;
				break;
			}
		}
		if (!err)
		{
			self->renderer->SetPolygonOffset(offset[0], offset[1]);
			return 0;
		}
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be two floats sequence.");
	return -1;
}

static PyObject* DCRendererRenderTarget(DCRenderer* self, void*)
{
	DCOBJECT_VALIDATE(self->renderer, NULL);
	DKRenderTarget* target = self->renderer->RenderTarget();
	if (target)
	{
		PyObject* obj = DCRenderTargetFromObject(target);
		if (obj != self->renderTarget)
			self->UpdateRenderTarget();
		return obj;
	}
	Py_RETURN_NONE;
}

static PyGetSetDef getsets[] = {
	{ "viewport", (getter)&DCRendererViewport, (setter)&DCRendererSetViewport, 0, 0 },
	{ "bounds", (getter)&DCRendererBounds, (setter)&DCRendererSetBounds, 0, 0 },
	{ "transform", (getter)&DCRendererTransform, (setter)&DCRendererSetTransform, 0, 0 },
	{ "polygonOffset", (getter)&DCRendererPolygonOffset, (setter)&DCRendererSetPolygonOffset, 0, 0 },
	{ "renderTarget", (getter)&DCRendererRenderTarget, 0, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Renderer",					/* tp_name */
	sizeof(DCRenderer),								/* tp_basicsize */
	0,												/* tp_itemsize */
	(destructor)&DCRendererDealloc,					/* tp_dealloc */
	0,												/* tp_print */
	0,												/* tp_getattr */
	0,												/* tp_setattr */
	0,												/* tp_reserved */
	(reprfunc)&DCRendererRepr,						/* tp_repr */
	0,												/* tp_as_number */
	0,												/* tp_as_sequence */
	0,												/* tp_as_mapping */
	0,												/* tp_hash  */
	0,												/* tp_call */
	0,												/* tp_str */
	0,												/* tp_getattro */
	0,												/* tp_setattro */
	0,												/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,								/* tp_flags */
	0,												/* tp_doc */
	(traverseproc)&DCRendererTraverse,				/* tp_traverse */
	(inquiry)&DCRendererClear,						/* tp_clear */
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
	(initproc)&DCRendererInit,						/* tp_init */
	0,												/* tp_alloc */
	&DCRendererNew,									/* tp_new */
};

PyTypeObject* DCRendererTypeObject(void)
{
	return &objectType;
}

PyObject* DCRendererFromObject(DKRenderer* renderer)
{
	if (renderer)
	{
		DCRenderer* self = (DCRenderer*)DCObjectFromAddress(renderer);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCRenderer*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->renderer = renderer;
				DCObjectSetAddress(self->renderer, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKRenderer* DCRendererToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCRenderer*)obj)->renderer;
	}
	return NULL;
}

