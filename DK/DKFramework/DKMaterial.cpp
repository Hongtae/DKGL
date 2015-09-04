//
//  File: DKMaterial.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKMaterial.h"
#include "DKOpenGLContext.h"
#include "DKTexture.h"

namespace DKFramework
{
	namespace Private
	{
		namespace
		{
			inline GLenum GetGLValue(DKMaterial::RenderingProperty::DepthFunc d)
			{
				switch (d)
				{
				case DKFramework::DKMaterial::RenderingProperty::DepthFuncNever:			return GL_NEVER;
				case DKFramework::DKMaterial::RenderingProperty::DepthFuncAlways:			return GL_ALWAYS;
				case DKFramework::DKMaterial::RenderingProperty::DepthFuncLess:				return GL_LESS;
				case DKFramework::DKMaterial::RenderingProperty::DepthFuncLessEqual:		return GL_LEQUAL;
				case DKFramework::DKMaterial::RenderingProperty::DepthFuncEqual:			return GL_EQUAL;
				case DKFramework::DKMaterial::RenderingProperty::DepthFuncGreater:			return GL_GREATER;
				case DKFramework::DKMaterial::RenderingProperty::DepthFuncGreaterEqual:		return GL_GEQUAL;
				case DKFramework::DKMaterial::RenderingProperty::DepthFuncNotEqual:			return GL_NOTEQUAL;
				}
				DKFoundation::DKLog("Warning: DepthFunc (%x) invalid or not supported.\n", d);
				return GL_NEVER;
			}
		}
	}
}

using namespace DKFoundation;
using namespace DKFramework;


DKMaterial::DKMaterial(void)
{
}

DKMaterial::~DKMaterial(void)
{
}

bool DKMaterial::Bind(int programIndex, PropertyCallback* callback, const DKBlendState* blending) const
{
	if (programIndex < this->renderingProperties.Count())
	{
		int activeTextureStage = 0;

		const RenderingProperty& rp = this->renderingProperties.Value(programIndex);
		if (rp.program)
		{
			rp.program->Bind();

			DKRenderState& renderState = DKOpenGLContext::RenderState();
			renderState.DepthMask(rp.depthWrite);
			renderState.DepthFunc(Private::GetGLValue(rp.depthFunc));

			if (rp.depthFunc == RenderingProperty::DepthFuncAlways && rp.depthWrite == false)
				renderState.Disable(DKRenderState::GLStateDepthTest); // depth not writable if DEPTH_TEST disabled.
			else
				renderState.Enable(DKRenderState::GLStateDepthTest);

			if (blending)  // override blend modes if provided.
				blending->Bind();
			else
				rp.blendState.Bind();

			auto BindSampler = [&](const DKShaderConstant& sc)->bool
			{
				const TextureArray* texArray = NULL;
				const DKTextureSampler* texSampler = NULL;

				if (callback)
				{
					const Sampler* sampler = callback->GetSamplerProperty(sc, programIndex);
					if (sampler)
					{
						texArray = &sampler->textures;
						texSampler = sampler->sampler;
					}
				}
				if (texArray == NULL)
				{
					auto p = this->samplerProperties.Find(sc.name);
					if (p)
					{
						texArray = &p->value.textures;
						texSampler = p->value.sampler;
					}
				}
				if (texArray)
				{
					DKArray<GLint> boundTexStages; // texture stage id has been bound
					boundTexStages.Reserve(sc.components);
					for (size_t i = 0; i < texArray->Count(); ++i)
					{
						const DKTexture* tex = texArray->Value(i);
						if (tex)
						{
							if (activeTextureStage < renderState.maxCombinedTextureImageUnits)
							{
								renderState.ActiveTexture(activeTextureStage);
								tex->Bind();
								if (texSampler)
									texSampler->Bind(tex->ObjectTarget());

								boundTexStages.Add(activeTextureStage);
								activeTextureStage++;
							}
							else
							{
								DKLog("Warning: Max texture unit limit exceed! Cannot bind texture anymore.\n");
								break;
							}
						}
					}
					size_t numStages = Min<size_t>(boundTexStages.Count(), sc.components);
					if (numStages > 0)
					{
						glUniform1iv(sc.location, numStages, (GLint*)boundTexStages);
						return true;
					}
				}
				return false;
			};
			auto BindIntProperty = [&](const DKShaderConstant& sc)->bool
			{
				const GLint* values = NULL;
				size_t count = 0;
				if (callback)
				{
					auto v = callback->GetIntProperty(sc, programIndex);
					values = v;
					count = v.Count();
				}
				if (values == NULL)
				{
					auto p = this->shadingProperties.Find(sc.name);
					if (p)
					{
						values = p->value.value.integers;
						count = p->value.value.integers.Count();
					}
				}
				if (values && count > 0)
				{
					switch (sc.type)
					{
					case DKShaderConstant::TypeBool1:
					case DKShaderConstant::TypeInt1:
						glUniform1iv(sc.location, Min<GLsizei>(count / 1, sc.components), values);
						break;
					case DKShaderConstant::TypeBool2:
					case DKShaderConstant::TypeInt2:
						glUniform2iv(sc.location, Min<GLsizei>(count / 2, sc.components), values);
						break;
					case DKShaderConstant::TypeBool3:
					case DKShaderConstant::TypeInt3:
						glUniform3iv(sc.location, Min<GLsizei>(count / 3, sc.components), values);
						break;
					case DKShaderConstant::TypeBool4:
					case DKShaderConstant::TypeInt4:
						glUniform4iv(sc.location, Min<GLsizei>(count / 4, sc.components), values);
						break;
					}
					return true;
				}
				return false;
			};
			auto BindFloatProperty = [&](const DKShaderConstant& sc)->bool
			{
				const GLfloat* values = NULL;
				size_t count = 0;
				if (callback)
				{
					auto v = callback->GetFloatProperty(sc, programIndex);
					values = v;
					count = v.Count();
				}
				if (values == NULL)
				{
					auto p = this->shadingProperties.Find(sc.name);
					if (p)
					{
						values = p->value.value.floatings;
						count = p->value.value.floatings.Count();
					}
				}
				if (values && count > 0)
				{
					switch (sc.type)
					{
						case DKShaderConstant::TypeFloat1:
							glUniform1fv(sc.location, Min<GLsizei>(count / 1, sc.components), values);
							break;
						case DKShaderConstant::TypeFloat2:
							glUniform2fv(sc.location, Min<GLsizei>(count / 2, sc.components), values);
							break;
						case DKShaderConstant::TypeFloat3:
							glUniform3fv(sc.location, Min<GLsizei>(count / 3, sc.components), values);
							break;
						case DKShaderConstant::TypeFloat4:
							glUniform4fv(sc.location, Min<GLsizei>(count / 4, sc.components), values);
							break;
						case DKShaderConstant::TypeFloat2x2:
							glUniformMatrix2fv(sc.location, Min<GLsizei>(count / 4, sc.components), GL_FALSE, values);
							break;
						case DKShaderConstant::TypeFloat3x3:
							glUniformMatrix3fv(sc.location, Min<GLsizei>(count / 9, sc.components), GL_FALSE, values);
							break;
						case DKShaderConstant::TypeFloat4x4:
							glUniformMatrix4fv(sc.location, Min<GLsizei>(count / 16, sc.components), GL_FALSE, values);
							break;
					}
					return true;
				}
				return false;
			};

			// bind Uinform, Sampler into shader program.
			for (const DKShaderConstant& sc : rp.program->uniforms)
			{
				if (sc.components == 0 || sc.location < 0)
					continue;

				bool result = false;
				DKShaderConstant::BaseType baseType = DKShaderConstant::GetBaseType(sc.type);
				switch (baseType)
				{
				case DKShaderConstant::BaseTypeSampler:
					result = BindSampler(sc);
					break;
				case DKShaderConstant::BaseTypeBoolean:
				case DKShaderConstant::BaseTypeInteger:
					result = BindIntProperty(sc);
					break;
				case DKShaderConstant::BaseTypeFloating:
					result = BindFloatProperty(sc);
					break;
				default:
					DKLog("Warning: uniform:(%ls) is unknown type.\n", (const wchar_t*)sc.name);
					break;
				}

				if (!result)
				{
					DKLog("Warning: uniform:(%ls) bind failed.\n", (const wchar_t*)sc.name);
				}
			}
			return true;
		}
	}
	return false;
}

const DKFoundation::DKArray<DKVertexStream>& DKMaterial::StreamArray(int state) const
{
	return renderingProperties.Value(state).program->attributes;
}

bool DKMaterial::CompileShaderSource(ShaderSource* src, DKString& result)
{
	if (src == NULL)
		return false;

	if (src->shader)
	{
		DKString source = src->shader->GetSource();
		if (source == src->source)		// no need to update
			return true;
	}

	src->shader = DKShader::Create(src->source, src->type, result);
	if (src->shader)
		return true;

	return false;
}

bool DKMaterial::BuildProgram(int index, BuildLog* log)
{
	if (index >= renderingProperties.Count())
		return false;

	for (ShaderSource& src : shaderProperties)
	{
		// remove whitespaces from the source
		DKString content = DKString(src.source).RemoveWhitespaces();
		if (content.Length() == 0)
			continue;

		DKString mesg = L"";
		bool compiled = CompileShaderSource(&src, mesg);
		if (!compiled)
		{
			if (log)
			{
				log->errorLog = mesg;
				log->failedShader = src.name;
				log->failedProgram = L"";		// global
			}
			return false;
		}
	}

	RenderingProperty& rp = renderingProperties.Value(index);
	for (ShaderSource& src : rp.shaders)
	{
		// remove whitespaces from the source
		DKString content = DKString(src.source).RemoveWhitespaces();
		if (content.Length() == 0)
			continue;

		DKString mesg = L"";
		bool compiled = CompileShaderSource(&src, mesg);
		if (!compiled)
		{
			if (log)
			{
				log->errorLog = mesg;
				log->failedShader = src.name;
				log->failedProgram = rp.name;
			}
			return false;
		}
	}

	DKArray<DKObject<DKShader>>	shaderArray;
	shaderArray.Reserve(shaderProperties.Count() + rp.shaders.Count());
	// add material's global shader
	for (int i = 0; i < shaderProperties.Count(); i++)
		shaderArray.Add(shaderProperties.Value(i).shader);
	// add program's shader
	for (int i = 0; i < rp.shaders.Count(); i++)
		shaderArray.Add(rp.shaders.Value(i).shader);

	DKString mesg = L"";
	rp.program = DKShaderProgram::Create(shaderArray, mesg);

	if (log)
	{
		log->errorLog = mesg;
		log->failedProgram = rp.name;
	}

	if (rp.program)
	{
		for (int i = 0; i < rp.program->attributes.Count(); i++)
		{
			DKVertexStream&	s = rp.program->attributes.Value(i);
			StreamPropertyMap::Pair* p = streamProperties.Find(s.name);
			if (p)
			{
				if (s.type != p->value.type)
					DKLog("Warning: attribute \"%ls\" type mismatch! (shader:%ls, material:%ls)\n", (const wchar_t*)s.name, (const wchar_t*)DKVertexStream::TypeToString(s.type), (const wchar_t*)DKVertexStream::TypeToString(p->value.type));

				s.id = p->value.id;
			}
			else
			{
				DKLog("Warning: attribute \"%ls\" not found!\n", (const wchar_t*)s.name);
			}
		}
		for (int i = 0; i < rp.program->uniforms.Count(); i++)
		{
			DKShaderConstant& s = rp.program->uniforms.Value(i);
			if (DKShaderConstant::GetBaseType(s.type) == DKShaderConstant::BaseTypeSampler)
			{
				SamplerPropertyMap::Pair* p = samplerProperties.Find(s.name);
				if (p)
				{
					if (s.type != p->value.type)
						DKLog("Warning: sampler \"%ls\" type mismatch! (shader:%ls, material:%ls)\n", (const wchar_t*)s.name, (const wchar_t*)DKShaderConstant::TypeToString(s.type), (const wchar_t*)DKShaderConstant::TypeToString(p->value.type));

					s.id = p->value.id;
				}
				else
					DKLog("Warning: sampler \"%ls\" not found!\n", (const wchar_t*)s.name);
			}
			else
			{
				ShadingPropertyMap::Pair* p = shadingProperties.Find(s.name);
				if (p)
					s.id = p->value.id;
				else
					DKLog("Warning: uniform \"%ls\" not found!\n", (const wchar_t*)s.name);
			}
		}
		return true;
	}
	return false;
}

bool DKMaterial::Build(BuildLog* log)
{
	for (int i = 0; i < renderingProperties.Count(); i++)
	{
		if (!BuildProgram(i, log))
		{
			return false;
		}
	}
	return true;
}

void DKMaterial::ReleaseShaderCompiler(void)
{
	glReleaseShaderCompiler();
}

size_t DKMaterial::NumberOfRenderProperties(void) const
{
	return renderingProperties.Count();
}

int DKMaterial::IndexOfRenderPropertyName(const DKFoundation::DKString& name) const
{
	for (int i = 0; i < renderingProperties.Count(); i++)
	{
		if (renderingProperties.Value(i).name == name)
			return i;
	}
	return -1;
}

bool DKMaterial::Validate(void)
{
	BuildLog log;
	if (Build(&log))
		return IsValid();

	DKLog("[%s] Material build error: %ls, in shader: %ls, program: %ls\n",
		DKGL_FUNCTION_NAME,
		(const wchar_t*)log.errorLog,
		(const wchar_t*)log.failedShader,
		(const wchar_t*)log.failedProgram);
	return false;
}

bool DKMaterial::IsValid(void) const
{
	if (renderingProperties.IsEmpty())
		return false;

	for (int i = 0; i < renderingProperties.Count(); i++)
	{
		if (renderingProperties.Value(i).program == NULL)
			return false;
	}
	return true;
}

DKObject<DKSerializer> DKMaterial::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKMaterial* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKMaterial");
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"super", target->DKResource::Serializer(), NULL);
			this->Bind(L"samplerProperties",
				DKFunction(this, &LocalSerializer::GetSamplerProperties),
				DKFunction(this, &LocalSerializer::SetSamplerProperties),
				DKFunction(this, &LocalSerializer::CheckSamplerProperties),
				NULL);

			this->Bind(L"textures",
				DKFunction(this, &LocalSerializer::GetTextureMap),
				DKFunction(this, &LocalSerializer::SetTextureMap),
				NULL,
				DKSerializer::ExternalResourceReferenceIfPossible,
				DKValue(0));

			this->Bind(L"textureSamplers",
				DKFunction(this, &LocalSerializer::GetTexSamplerMap),
				DKFunction(this, &LocalSerializer::SetTexSamplerMap),
				NULL,
				DKSerializer::ExternalResourceReferenceIfPossible,
				DKValue(0));

			this->Bind(L"shadingProperties",
				DKFunction(this, &LocalSerializer::GetShadingProperties),
				DKFunction(this, &LocalSerializer::SetShadingProperties),
				DKFunction(this, &LocalSerializer::CheckShadingProperties),
				NULL);

			this->Bind(L"streamProperties",
				DKFunction(this, &LocalSerializer::GetStreamProperties),
				DKFunction(this, &LocalSerializer::SetStreamProperties),
				DKFunction(this, &LocalSerializer::CheckStreamProperties),
				NULL);

			this->Bind(L"shaderProperties",
				DKFunction(this, &LocalSerializer::GetShaderProperties),
				DKFunction(this, &LocalSerializer::SetShaderProperties),
				DKFunction(this, &LocalSerializer::CheckShaderProperties),
				NULL);

			this->Bind(L"renderingProperties",
				DKFunction(this, &LocalSerializer::GetRenderingProperties),
				DKFunction(this, &LocalSerializer::SetRenderingProperties),
				DKFunction(this, &LocalSerializer::CheckRenderingProperties),
				NULL);

			return this;
		}
	private:
		void GetSamplerProperties(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypePairs);
			target->samplerProperties.EnumerateForward([&v](const SamplerPropertyMap::Pair& pair)
			{
				const SamplerProperty& sp = pair.value;
				DKVariant vnode;
				vnode.Pairs().Insert(L"id", (const DKVariant::VString&)DKShaderConstant::UniformToString(sp.id));
				vnode.Pairs().Insert(L"type", (const DKVariant::VString&)DKShaderConstant::TypeToString(sp.type));
				DKVariant texArray;
				for (size_t i = 0; i < sp.textures.Count(); ++i)
				{
					DKString texKey = DKString::Format("Sampler:%ls:%u", (const wchar_t*)pair.key, i);
					texArray.Array().Add((const DKVariant::VString&)texKey);
				}
				vnode.Pairs().Insert(L"textureArray", texArray);
				v.Pairs().Insert(pair.key, vnode);
			});
		}
		void SetSamplerProperties(DKVariant& v)
		{
			this->samplers.Clear();
			v.Pairs().EnumerateForward([this](DKVariant::VPairs::Pair& pair)
			{
				const DKVariant& vnode = pair.value;
				if (vnode.ValueType() == DKVariant::TypePairs)
				{
					const DKVariant::VPairs::Pair* pId = vnode.Pairs().Find(L"id");
					const DKVariant::VPairs::Pair* pType = vnode.Pairs().Find(L"type");
					const DKVariant::VPairs::Pair* pTex = vnode.Pairs().Find(L"textureArray");
					if (pId && pId->value.ValueType() == DKVariant::TypeString &&
						pType && pType->value.ValueType() == DKVariant::TypeString &&
						pTex && pTex->value.ValueType() == DKVariant::TypeArray)
					{
						SamplerData sd;
						sd.id = DKShaderConstant::StringToUniform(pId->value.String());
						sd.type = DKShaderConstant::StringToType(pType->value.String());
						for (size_t j = 0; j < pTex->value.Array().Count(); ++j)
						{
							const DKVariant& vtex = pTex->value.Array().Value(j);
							if (vtex.ValueType() == DKVariant::TypeString)
								sd.textures.Add(vtex.String());
						}
						this->samplers.Insert(pair.key, sd);
					}
				}
			});
		}
		bool CheckSamplerProperties(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypePairs;
		}
		void GetTextureMap(ExternalMapType& v)
		{
			target->samplerProperties.EnumerateForward([&v](SamplerPropertyMap::Pair& pair)
			{
				SamplerProperty& sp = pair.value;
				for (size_t i = 0; i < sp.textures.Count(); ++i)
				{
					DKResource* res = sp.textures.Value(i).SafeCast<DKResource>();
					if (res)
					{
						DKString texKey = DKString::Format("Sampler:%ls:%u", (const wchar_t*)pair.key, i);
						v.Insert(texKey, res);
					}
				}
			});
		}
		void SetTextureMap(ExternalMapType& v)
		{
			this->textureMap.Clear();
			v.EnumerateForward([this](ExternalMapType::Pair& pair)
			{
				DKTexture* ptex = pair.value.SafeCast<DKTexture>();
				if (ptex)
					this->textureMap.Insert(pair.key, ptex);
			});
		}
		void GetTexSamplerMap(ExternalMapType& v)
		{
			target->samplerProperties.EnumerateForward([&v](SamplerPropertyMap::Pair& pair)
			{
				DKResource* res = pair.value.sampler.SafeCast<DKResource>();
				if (res)
					v.Insert(pair.key, res);
			});
		}
		void SetTexSamplerMap(ExternalMapType& v)
		{
			this->texSamplerMap.Clear();
			v.EnumerateForward([this](ExternalMapType::Pair& pair)
			{
				DKTextureSampler* sampler = pair.value.SafeCast<DKTextureSampler>();
				if (sampler)
					this->texSamplerMap.Insert(pair.key, sampler);
			});
		}
		void GetShadingProperties(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypePairs);
			target->shadingProperties.EnumerateForward([&v](const ShadingPropertyMap::Pair& pair)
			{
				const ShadingProperty& shading = pair.value;
				DKVariant vnode;
				vnode.Pairs().Insert(L"id", (const DKVariant::VString&)DKShaderConstant::UniformToString(shading.id));
				vnode.Pairs().Insert(L"type", (const DKVariant::VString&)DKShaderConstant::TypeToString(shading.type));
				if (shading.id == DKShaderConstant::UniformUserDefine)
				{
					DKVariant valueArray;
					switch (DKShaderConstant::GetBaseType(shading.type))
					{
					case DKShaderConstant::BaseTypeBoolean:
					case DKShaderConstant::BaseTypeInteger:
						for (size_t j = 0; j < shading.value.integers.Count(); ++j)
							valueArray.Array().Add((DKVariant::VInteger)shading.value.integers.Value(j));
						break;
					case DKShaderConstant::BaseTypeFloating:
						for (size_t j = 0; j < shading.value.floatings.Count(); ++j)
							valueArray.Array().Add((DKVariant::VFloat)shading.value.floatings.Value(j));
						break;
					}
					vnode.Pairs().Insert(L"value", valueArray);
				}
				v.Pairs().Insert(pair.key, vnode);
			});
		}
		void SetShadingProperties(DKVariant& v)
		{
			target->shadingProperties.Clear();
			v.Pairs().EnumerateForward([this](DKVariant::VPairs::Pair& pair)
			{
				const DKVariant& vnode = pair.value;
				if (vnode.ValueType() == DKVariant::TypePairs)
				{
					const DKVariant::VPairs::Pair* pId = vnode.Pairs().Find(L"id");
					const DKVariant::VPairs::Pair* pType = vnode.Pairs().Find(L"type");
					if (pId && pId->value.ValueType() == DKVariant::TypeString &&
						pType && pType->value.ValueType() == DKVariant::TypeString)
					{
						ShadingProperty sp;
						sp.id = DKShaderConstant::StringToUniform(pId->value.String());
						sp.type = DKShaderConstant::StringToType(pType->value.String());

						const DKVariant::VPairs::Pair* pValue = vnode.Pairs().Find(L"value");
						if (sp.id == DKShaderConstant::UniformUserDefine && pValue && pValue->value.ValueType() == DKVariant::TypeArray)
						{
							switch (DKShaderConstant::GetBaseType(sp.type))
							{
							case DKShaderConstant::BaseTypeBoolean:
							case DKShaderConstant::BaseTypeInteger:
								for (size_t j = 0; j < pValue->value.Array().Count(); ++j)
								{
									const DKVariant& value = pValue->value.Array().Value(j);
									if (value.ValueType() == DKVariant::TypeInteger)
										sp.value.integers.Add((int)value.Integer());
								}
								break;
							case DKShaderConstant::BaseTypeFloating:
								for (size_t j = 0; j < pValue->value.Array().Count(); ++j)
								{
									const DKVariant& value = pValue->value.Array().Value(j);
									if (value.ValueType() == DKVariant::TypeFloat)
										sp.value.floatings.Add((float)value.Float());
								}
								break;
							}
						}
						target->shadingProperties.Insert(pair.key, sp);
					}
				}
			});
		}
		bool CheckShadingProperties(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypePairs;
		}
		void GetStreamProperties(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypePairs);
			target->streamProperties.EnumerateForward([&v](const StreamPropertyMap::Pair& pair)
			{
				const StreamProperty& stream = pair.value;
				DKVariant vnode;
				vnode.Pairs().Insert(L"id", (const DKVariant::VString&)DKVertexStream::StreamToString(stream.id));
				vnode.Pairs().Insert(L"type", (const DKVariant::VString&)DKVertexStream::TypeToString(stream.type));
				vnode.Pairs().Insert(L"components", (DKVariant::VInteger)stream.components);
				v.Pairs().Insert(pair.key, vnode);
			});
		}
		void SetStreamProperties(DKVariant& v)
		{
			target->streamProperties.Clear();
			v.Pairs().EnumerateForward([this](DKVariant::VPairs::Pair& pair)
			{
				if (pair.value.ValueType() == DKVariant::TypePairs)
				{
					const DKVariant::VPairs::Pair* pId = pair.value.Pairs().Find(L"id");
					const DKVariant::VPairs::Pair* pType = pair.value.Pairs().Find(L"type");
					const DKVariant::VPairs::Pair* pComp = pair.value.Pairs().Find(L"components");
					if (pId && pId->value.ValueType() == DKVariant::TypeString &&
						pType && pType->value.ValueType() == DKVariant::TypeString &&
						pComp && pComp->value.ValueType() == DKVariant::TypeInteger)
					{
						DKMaterial::StreamProperty sp = {
							DKVertexStream::StringToStream(pId->value.String()),
							DKVertexStream::StringToType(pType->value.String()),
							(size_t)pComp->value.Integer()
						};
						target->streamProperties.Insert(pair.key, sp);
					}
				}
			});
		}
		bool CheckStreamProperties(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypePairs;
		}
		void GetShaderSourceVariant(DKVariant& v, const DKMaterial::ShaderSource& src) const
		{
			v.SetValueType(DKVariant::TypePairs);
			v.Pairs().Insert(L"name", (const DKVariant::VString&)src.name);
			v.Pairs().Insert(L"source", (const DKVariant::VString&)src.source);
			switch (src.type)
			{
			case DKShader::TypeVertexShader:	v.Pairs().Insert(L"type", (const DKVariant::VString&)L"VertexShader");		break;
			case DKShader::TypeFragmentShader:	v.Pairs().Insert(L"type", (const DKVariant::VString&)L"FragmentShader");	break;
			case DKShader::TypeGeometryShader:	v.Pairs().Insert(L"type", (const DKVariant::VString&)L"GeometryShader");	break;
			default:							v.Pairs().Insert(L"type", (const DKVariant::VString&)L"Unknown");			break;
			}
		}
		bool SetShaderSource(const DKVariant& v, DKMaterial::ShaderSource& src)
		{
			if (v.ValueType() == DKVariant::TypePairs)
			{
				const DKVariant::VPairs::Pair* pName = v.Pairs().Find(L"name");
				const DKVariant::VPairs::Pair* pSrc = v.Pairs().Find(L"source");
				const DKVariant::VPairs::Pair* pType = v.Pairs().Find(L"type");
				if (pName && pName->value.ValueType() == DKVariant::TypeString &&
					pSrc && pSrc->value.ValueType() == DKVariant::TypeString &&
					pType && pType->value.ValueType() == DKVariant::TypeString)
				{
					src.name = pName->value.String();
					src.source = pSrc->value.String();
					src.type = DKShader::TypeUnknown;
					if (pType->value.String().CompareNoCase(L"VertexShader") == 0)
						src.type = DKShader::TypeVertexShader;
					else if (pType->value.String().CompareNoCase(L"FragmentShader") == 0)
						src.type = DKShader::TypeFragmentShader;
					else if (pType->value.String().CompareNoCase(L"GeometryShader") == 0)
						src.type = DKShader::TypeGeometryShader;

					return true;
				}
			}
			return false;
		}
		void GetShaderProperties(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypeArray);
			for (const ShaderSource& src : target->shaderProperties)
			{
				DKVariant::VArray::Index idx = v.Array().Add(DKVariant());
				GetShaderSourceVariant(v.Array().Value(idx), src);
			}
		}
		void SetShaderProperties(DKVariant& v)
		{
			target->shaderProperties.Clear();
			for (DKVariant& var : v.Array())
			{
				DKMaterial::ShaderSource ss;
				if (SetShaderSource(var, ss))
					target->shaderProperties.Add(ss);
			}
		}
		bool CheckShaderProperties(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeArray;
		}
		DKVariant& GetBlendStateVariant(DKVariant&& v, const DKBlendState& bs) const
		{
			v.SetValueType(DKVariant::TypePairs);
			v.Pairs().Insert(L"colorMaskR", (DKVariant::VInteger)bs.colorWriteR);
			v.Pairs().Insert(L"colorMaskG", (DKVariant::VInteger)bs.colorWriteG);
			v.Pairs().Insert(L"colorMaskB", (DKVariant::VInteger)bs.colorWriteB);
			v.Pairs().Insert(L"colorMaskA", (DKVariant::VInteger)bs.colorWriteA);
			v.Pairs().Insert(L"srcRGB", (const DKVariant::VString&)DKBlendState::ModeToString(bs.srcBlendRGB));
			v.Pairs().Insert(L"dstRGB", (const DKVariant::VString&)DKBlendState::ModeToString(bs.dstBlendRGB));
			v.Pairs().Insert(L"srcAlpha", (const DKVariant::VString&)DKBlendState::ModeToString(bs.srcBlendAlpha));
			v.Pairs().Insert(L"dstAlpha", (const DKVariant::VString&)DKBlendState::ModeToString(bs.dstBlendAlpha));
			v.Pairs().Insert(L"funcRGB", (const DKVariant::VString&)DKBlendState::FuncToString(bs.blendFuncRGB));
			v.Pairs().Insert(L"funcAlpha", (const DKVariant::VString&)DKBlendState::FuncToString(bs.blendFuncAlpha));
			v.Pairs().Insert(L"constantColor", DKVariant::VVector4(bs.constantColor.r, bs.constantColor.g, bs.constantColor.b, bs.constantColor.a));
			return v;
		}
		bool SetBlendState(const DKVariant& v, DKBlendState& bs)
		{
			if (v.ValueType() == DKVariant::TypePairs)
			{
				const DKVariant::VPairs::Pair* pColorMaskR = v.Pairs().Find(L"colorMaskR");
				const DKVariant::VPairs::Pair* pColorMaskG = v.Pairs().Find(L"colorMaskG");
				const DKVariant::VPairs::Pair* pColorMaskB = v.Pairs().Find(L"colorMaskB");
				const DKVariant::VPairs::Pair* pColorMaskA = v.Pairs().Find(L"colorMaskA");
				const DKVariant::VPairs::Pair* pSrcRGB = v.Pairs().Find(L"srcRGB");
				const DKVariant::VPairs::Pair* pDstRGB = v.Pairs().Find(L"dstRGB");
				const DKVariant::VPairs::Pair* pSrcAlpha = v.Pairs().Find(L"srcAlpha");
				const DKVariant::VPairs::Pair* pDstAlpha = v.Pairs().Find(L"dstAlpha");
				const DKVariant::VPairs::Pair* pFuncRGB = v.Pairs().Find(L"funcRGB");
				const DKVariant::VPairs::Pair* pFuncAlpha = v.Pairs().Find(L"funcAlpha");
				const DKVariant::VPairs::Pair* pConstColor = v.Pairs().Find(L"constantColor");
				if (pColorMaskR && pColorMaskR->value.ValueType() == DKVariant::TypeInteger &&
					pColorMaskG && pColorMaskG->value.ValueType() == DKVariant::TypeInteger &&
					pColorMaskB && pColorMaskB->value.ValueType() == DKVariant::TypeInteger &&
					pColorMaskA && pColorMaskA->value.ValueType() == DKVariant::TypeInteger &&
					pSrcRGB && pSrcRGB->value.ValueType() == DKVariant::TypeString &&
					pDstRGB && pDstRGB->value.ValueType() == DKVariant::TypeString &&
					pSrcAlpha && pSrcAlpha->value.ValueType() == DKVariant::TypeString &&
					pDstAlpha && pDstAlpha->value.ValueType() == DKVariant::TypeString &&
					pFuncRGB && pFuncRGB->value.ValueType() == DKVariant::TypeString &&
					pFuncAlpha && pFuncAlpha->value.ValueType() == DKVariant::TypeString &&
					pConstColor && pConstColor->value.ValueType() == DKVariant::TypeVector4)
				{
					bs.colorWriteR = pColorMaskR->value.Integer() != 0;
					bs.colorWriteG = pColorMaskG->value.Integer() != 0;
					bs.colorWriteB = pColorMaskB->value.Integer() != 0;
					bs.colorWriteA = pColorMaskA->value.Integer() != 0;
					bs.srcBlendRGB = DKBlendState::StringToMode(pSrcRGB->value.String());
					bs.dstBlendRGB = DKBlendState::StringToMode(pDstRGB->value.String());
					bs.srcBlendAlpha = DKBlendState::StringToMode(pSrcAlpha->value.String());
					bs.dstBlendAlpha = DKBlendState::StringToMode(pDstAlpha->value.String());
					bs.blendFuncRGB = DKBlendState::StringToFunc(pFuncRGB->value.String());
					bs.blendFuncAlpha = DKBlendState::StringToFunc(pFuncAlpha->value.String());
					bs.constantColor = DKColor(
						Clamp(pConstColor->value.Vector4().x, 0.0f, 1.0f),
						Clamp(pConstColor->value.Vector4().y, 0.0f, 1.0f),
						Clamp(pConstColor->value.Vector4().z, 0.0f, 1.0f),
						Clamp(pConstColor->value.Vector4().w, 0.0f, 1.0f)).RGBA32Value();

					return true;
				}
			}
			return false;
		}
		void GetRenderingProperties(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypeArray);
			for (const RenderingProperty& rp : target->renderingProperties)
			{
				DKVariant vnode(DKVariant::TypePairs);
				vnode.Pairs().Insert(L"name", (const DKVariant::VString&)rp.name);
				switch (rp.depthFunc)
				{
				case RenderingProperty::DepthFuncAlways:			vnode.Pairs().Insert(L"depthFunc", (const DKVariant::VString&)L"Always");			break;
				case RenderingProperty::DepthFuncLess:				vnode.Pairs().Insert(L"depthFunc", (const DKVariant::VString&)L"Less");			break;
				case RenderingProperty::DepthFuncLessEqual:			vnode.Pairs().Insert(L"depthFunc", (const DKVariant::VString&)L"LessEqual");		break;
				case RenderingProperty::DepthFuncEqual:				vnode.Pairs().Insert(L"depthFunc", (const DKVariant::VString&)L"Equal");			break;
				case RenderingProperty::DepthFuncGreater:			vnode.Pairs().Insert(L"depthFunc", (const DKVariant::VString&)L"Greater");		break;
				case RenderingProperty::DepthFuncGreaterEqual:		vnode.Pairs().Insert(L"depthFunc", (const DKVariant::VString&)L"GreaterEqual");	break;
				case RenderingProperty::DepthFuncNotEqual:			vnode.Pairs().Insert(L"depthFunc", (const DKVariant::VString&)L"NotEqual");		break;
				default:											vnode.Pairs().Insert(L"depthFunc", (const DKVariant::VString&)L"Never");			break;
				}
				vnode.Pairs().Insert(L"depthWrite", (DKVariant::VInteger)rp.depthWrite);
				vnode.Pairs().Insert(L"blendState", GetBlendStateVariant(DKVariant(), rp.blendState));
				DKVariant shaderArray;
				for (const ShaderSource& src : rp.shaders)
				{
					DKVariant::VArray::Index idx = shaderArray.Array().Add(DKVariant());
					GetShaderSourceVariant(shaderArray.Array().Value(idx), src);
				}
				vnode.Pairs().Insert(L"shaders", shaderArray);
				v.Array().Add(vnode);
			}
		}
		void SetRenderingProperties(DKVariant& v)
		{
			target->renderingProperties.Clear();

			for (const DKVariant& vnode : v.Array())
			{
				if (vnode.ValueType() == DKVariant::TypePairs)
				{
					const DKVariant::VPairs::Pair* pName = vnode.Pairs().Find(L"name");
					const DKVariant::VPairs::Pair* pDepthFunc = vnode.Pairs().Find(L"depthFunc");
					const DKVariant::VPairs::Pair* pDepthWrite = vnode.Pairs().Find(L"depthWrite");
					const DKVariant::VPairs::Pair* pBlendState = vnode.Pairs().Find(L"blendState");
					const DKVariant::VPairs::Pair* pShaders = vnode.Pairs().Find(L"shaders");

					DKBlendState blendState;
					if (pName && pName->value.ValueType() == DKVariant::TypeString &&
						pDepthFunc && pDepthFunc->value.ValueType() == DKVariant::TypeString &&
						pDepthWrite && pDepthWrite->value.ValueType() == DKVariant::TypeInteger &&
						pShaders && pShaders->value.ValueType() == DKVariant::TypeArray &&
						pBlendState && SetBlendState(pBlendState->value, blendState))
					{
						RenderingProperty rp;
						rp.name = pName->value.String();
						if (pDepthFunc->value.String().CompareNoCase(L"Always") == 0)
							rp.depthFunc = RenderingProperty::DepthFuncAlways;
						else if (pDepthFunc->value.String().CompareNoCase(L"Less") == 0)
							rp.depthFunc = RenderingProperty::DepthFuncLess;
						else if (pDepthFunc->value.String().CompareNoCase(L"LessEqual") == 0)
							rp.depthFunc = RenderingProperty::DepthFuncLessEqual;
						else if (pDepthFunc->value.String().CompareNoCase(L"Equal") == 0)
							rp.depthFunc = RenderingProperty::DepthFuncEqual;
						else if (pDepthFunc->value.String().CompareNoCase(L"Greater") == 0)
							rp.depthFunc = RenderingProperty::DepthFuncGreater;
						else if (pDepthFunc->value.String().CompareNoCase(L"GreaterEqual") == 0)
							rp.depthFunc = RenderingProperty::DepthFuncGreaterEqual;
						else if (pDepthFunc->value.String().CompareNoCase(L"NotEqual") == 0)
							rp.depthFunc = RenderingProperty::DepthFuncNotEqual;
						else
							rp.depthFunc = RenderingProperty::DepthFuncNever;
						rp.depthWrite = pDepthWrite->value.Integer() != 0;
						rp.blendState = blendState;

						for (const DKVariant& vs : pShaders->value.Array())
						{
							ShaderSource ss;
							if (SetShaderSource(vs, ss))
								rp.shaders.Add(ss);
						}

						target->renderingProperties.Add(rp);
					}
				}
			}
		}
		bool CheckRenderingProperties(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeArray;
		}
		void Callback(State s)
		{
			if (s == StateDeserializeBegin)
			{
				this->samplers.Clear();
				this->textureMap.Clear();
				this->texSamplerMap.Clear();
			}
			else if (s == StateDeserializeSucceed)
			{
				// update samplerProperties.
				target->samplerProperties.Clear();
				this->samplers.EnumerateForward([this](DKMap<DKString, SamplerData>::Pair& pair)
				{
					const SamplerData& sd = pair.value;
					DKMaterial::SamplerProperty sp = { sd.id, sd.type, DKMaterial::TextureArray(), NULL };

					// find sampler named (pair.key) from texSamplerMap
					auto pSampler = this->texSamplerMap.Find(pair.key);
					if (pSampler)
						sp.sampler = pSampler->value;

					for (const DKString& texId : sd.textures)
					{
						DKMap<DKString, DKObject<DKTexture>>::Pair* ptex = this->textureMap.Find(texId);
						if (ptex && ptex->value)
							sp.textures.Add(ptex->value);
					}
					target->samplerProperties.Insert(pair.key, sp);
				});
				this->samplers.Clear();
				this->textureMap.Clear();
				this->texSamplerMap.Clear();
			}
		}

		struct SamplerData
		{
			DKShaderConstant::Uniform	id;
			DKShaderConstant::Type		type;
			DKArray<DKString>			textures;
		};
		DKMap<DKString, SamplerData>				samplers;
		DKMap<DKString, DKObject<DKTexture>>		textureMap;
		DKMap<DKString, DKObject<DKTextureSampler>>	texSamplerMap;
		DKObject<DKMaterial>						target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
