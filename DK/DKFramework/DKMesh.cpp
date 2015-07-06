//
//  File: DKMesh.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKMesh.h"
#include "DKTexture2D.h"
#include "DKTexture3D.h"
#include "DKTextureCube.h"
#include "DKOpenGLContext.h"
#include "DKRenderTarget.h"

using namespace DKFoundation;
using namespace DKFramework;

DKMesh::DKMesh(void)
: DKModel(TypeMesh)
, drawFace(DrawFaceBoth)
, primitiveType(DKPrimitive::TypePoints)
, scale(1, 1, 1)
, hidden(false)
, drawingGroupFlags(1)
{
}

DKMesh::~DKMesh(void)
{
}

void DKMesh::SetMaterial(DKMaterial* m)
{
	material = m;
}

void DKMesh::AppendSampler(const DKString& name, DKTexture* texture)
{
	if (texture)
	{
		samplers.Value(name).textures.Add(texture);
	}
}

void DKMesh::SetSampler(const DKFoundation::DKString& name, DKTexture* texture, DKTextureSampler* sampler)
{
	if (texture)
	{
		DKMaterial::Sampler& s = samplers.Value(name);
		s.textures.Clear();
		s.textures.Add(texture);
		s.sampler = sampler;
	}
	else
		samplers.Remove(name);
}

void DKMesh::SetSampler(const DKString& name, const TextureArray& textures, DKTextureSampler* sampler)
{
	if (textures.IsEmpty())
	{
		samplers.Remove(name);
	}
	else
	{
		int numTextures = 0;
		for (const DKTexture* tex : textures)
		{
			if (tex)
				numTextures++;
		}

		DKMaterial::Sampler& s = samplers.Value(name);
		s.textures.Clear();
		s.textures.Reserve(numTextures);
		for (const DKTexture* tex : textures)
		{
			if (tex)
				s.textures.Add(const_cast<DKTexture*>(tex));
		}
		s.sampler = sampler;
	}
}

const DKMesh::TextureSampler* DKMesh::Sampler(const DKString& name) const
{
	auto p = samplers.Find(name);
	if (p)		return &p->value;
	return NULL;
}

DKMesh::TextureSampler* DKMesh::Sampler(const DKString& name)
{
	auto p = samplers.Find(name);
	if (p)		return &p->value;
	return NULL;
}

size_t DKMesh::SamplerCount(void) const
{
	return samplers.Count();
}

void DKMesh::RemoveSampler(const DKFoundation::DKString& name)
{
	samplers.Remove(name);
}

void DKMesh::RemoveAllSamplers(void)
{
	samplers.Clear();
}

void DKMesh::SetMaterialProperty(const DKString& name, const PropertyArray& value)
{
	materialProperties.Update(name, value);
}

size_t DKMesh::MaterialPropertyCount(const DKString& name) const
{
	return materialProperties.Count();
}

const DKMesh::PropertyArray* DKMesh::MaterialProperty(const DKString& name) const
{
	const PropertyMap::Pair* p = materialProperties.Find(name);
	if (p)		return &p->value;
	return NULL;
}

DKMesh::PropertyArray* DKMesh::MaterialProperty(const DKString& name)
{
	PropertyMap::Pair* p = materialProperties.Find(name);
	if (p)		return &p->value;
	return NULL;
}

void DKMesh::RemoveMaterialProperty(const DKString& name)
{
	materialProperties.Remove(name);
}

void DKMesh::RemoveAllMaterialProperties(void)
{
	materialProperties.Clear();
}

DKAabb DKMesh::ScaledAabb(void) const
{
	const DKVector3& posMin = this->aabb.positionMin;
	const DKVector3& posMax = this->aabb.positionMax;
	return DKAabb(posMin * this->scale, posMax * this->scale);
}

DKSphere DKMesh::ScaledBoundingSphere(void) const
{
	const DKVector3& center = this->boundingSphere.center;
	float radius = this->boundingSphere.radius;
	float scaleMax = Max(this->scale.x, this->scale.y, this->scale.z);
	return DKSphere(center, radius * scaleMax);
}

const DKMatrix4& DKMesh::ScaledWorldTransformMatrix(void) const
{
	return scaledTransformMatrix;
}

bool DKMesh::BindTransform(DKSceneState& st) const
{
	st.modelMatrix = this->ScaledWorldTransformMatrix();
	st.modelMatrixInverse = DKMatrix4(st.modelMatrix).Inverse();
	st.modelViewMatrix = st.modelMatrix * st.viewMatrix;
	st.modelViewMatrixInverse = DKMatrix4(st.modelViewMatrix).Inverse();
	st.modelViewProjectionMatrix = st.modelMatrix * st.viewProjectionMatrix;
	st.modelViewProjectionMatrixInverse = DKMatrix4(st.modelViewProjectionMatrix).Inverse();

	return true;
}

void DKMesh::OnUpdateSceneState(const DKNSTransform& parentWorldTransform)
{
	DKModel::OnUpdateSceneState(parentWorldTransform);

	this->worldTransform = this->localTransform * parentWorldTransform;

	DKMatrix4 scaleMat(scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1);

	this->scaledTransformMatrix = scaleMat * this->worldTransform.Matrix4();
}

bool DKMesh::Bind(DKSceneState& ss, RenderInfo* info, const DKBlendState* blending) const
{
	if (material)
	{
		if (ss.sceneIndex < material->NumberOfRenderProperties())
		{
			DKRenderState& rs = DKOpenGLContext::RenderState();

			rs.FrontFace(DKRenderState::GLFrontFaceCCW);
			switch (drawFace)
			{
			case DrawFaceNone:
				rs.CullFace(DKRenderState::GLCullFaceBoth);
				rs.Enable(DKRenderState::GLStateCullFace);
				break;
			case DrawFaceCW:
				rs.CullFace(DKRenderState::GLCullFaceFront);
				rs.Enable(DKRenderState::GLStateCullFace);
				break;
			case DrawFaceCCW:
				rs.CullFace(DKRenderState::GLCullFaceBack);
				rs.Enable(DKRenderState::GLStateCullFace);
				break;
			case DrawFaceBoth:
				rs.Disable(DKRenderState::GLStateCullFace);
				break;
			}

			DKArray<DKVertexStream> streams = material->StreamArray(ss.sceneIndex);

			ss.ClearModel();
			if (BindTransform(ss))
			{
				ss.materialProperties = &this->materialProperties;
				ss.materialSamplers = &this->samplers;

				int minElements = 0x7fffffff;
				const DKArray<DKVertexStream>& streams = material->StreamArray(ss.sceneIndex);
				for (const DKVertexStream& vs : streams)
				{
					int verts = BindStream(vs);
					if (verts == 0)
					{
						DKLOG_DEBUG("BindStream(%ls) failed\n", (const wchar_t*)vs.name);
						return false;
					}
					minElements = Min(minElements, verts);
				}
				DKASSERT_DEBUG(minElements > 0);

				DKPrimitive::Type primType;
				DKIndexBuffer::Type indexType;
				int numIndices = 0;
				if (BindPrimitiveIndex(&primType, &numIndices, &indexType))
				{
					if (material->Bind(ss.sceneIndex, &ss, blending))
					{
						if (info)
						{
							info->primitive = primType;
							info->indexType = indexType;
							info->numVerts = minElements;
							info->numIndices = numIndices;
							info->numInstances = 1;
						}
						return true;
					}
					else
					{
						DKLOG_DEBUG("bind material failed.\n");
					}
				}
				else
				{
					DKLOG_DEBUG("BindPrimitiveIndex failed.\n");
				}
			}
			else
			{
				DKLOG_DEBUG("BindTransform failed\n");
			}
		}
	}
	return false;
}

DKMesh* DKMesh::Copy(UUIDObjectMap& uuids, const DKMesh* mesh)
{
	if (DKModel::Copy(uuids, mesh))
	{
		this->drawFace = mesh->drawFace;
		this->primitiveType = mesh->primitiveType;
		this->aabb = mesh->aabb;
		this->boundingSphere = mesh->boundingSphere;
		this->materialProperties = mesh->materialProperties;
		this->samplers = mesh->samplers;
		this->material = mesh->material;
		this->scale = mesh->scale;
		this->hidden = mesh->hidden;
		this->scaledTransformMatrix = mesh->scaledTransformMatrix;
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKMesh::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKSerializer* Init(DKMesh* p)
		{
			if (p == NULL)
				return NULL;
			target = p;

			this->SetResourceClass(L"DKMesh");
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"super", target->DKModel::Serializer(), NULL);

			this->Bind(L"defaultPrimitiveType",
				DKFunction(this, &LocalSerializer::GetDefaultPrimitiveType),
				DKFunction(this, &LocalSerializer::SetDefaultPrimitiveType),
				DKFunction(this, &LocalSerializer::CheckDefaultPrimitiveType),
				NULL);

			this->Bind(L"drawFace",
				DKFunction(this, &LocalSerializer::GetDrawFace),
				DKFunction(this, &LocalSerializer::SetDrawFace),
				DKFunction(this, &LocalSerializer::CheckDrawFace),
				NULL);

			this->Bind(L"boundingInfo",
				DKFunction(this, &LocalSerializer::GetBoundingInfo),
				DKFunction(this, &LocalSerializer::SetBoundingInfo),
				DKFunction(this, &LocalSerializer::CheckBoundingInfo),
				NULL);

			this->Bind(L"material",
				DKFunction(this, &LocalSerializer::GetMaterial),
				DKFunction(this, &LocalSerializer::SetMaterial),
				DKFunction(this, &LocalSerializer::CheckMaterial),
				ExternalResourceReferenceIfPossible,
				DKFunction(this, &LocalSerializer::ResetMaterial)->Invocation());

			this->Bind(L"materialProperties",
				DKFunction(this, &LocalSerializer::GetMaterialProperties),
				DKFunction(this, &LocalSerializer::SetMaterialProperties),
				DKFunction(this, &LocalSerializer::CheckMaterialProperties),
				NULL);

			this->Bind(L"samplers",
				DKFunction(this, &LocalSerializer::GetSamplers),
				DKFunction(this, &LocalSerializer::SetSamplers),
				DKFunction(this, &LocalSerializer::CheckSamplers),
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

			this->Bind(L"scale",
				DKFunction([p](ValueType& v){ v.SetVector3(p->Scale()); }),
				DKFunction([p](ValueType& v){ p->SetScale(v.Vector3()); }),
				DKFunction([](const ValueType& v)->bool{return v.ValueType() == DKVariant::TypeVector3; }),
				DKFunction([p]{ p->SetScale(1.0f); })->Invocation());

			this->Bind(L"hidden",
				DKFunction([p](ValueType& v){ v.SetInteger(p->IsHidden()); }),
				DKFunction([p](ValueType& v){ p->SetHidden(v.Integer() != 0); }),
				DKFunction([](const ValueType& v)->bool{return v.ValueType() == DKVariant::TypeInteger; }),
				DKFunction([p]{ p->SetHidden(false); })->Invocation());

			this->Bind(L"drawingGroupFlags",
			   DKFunction([p](ValueType& v){ v.SetInteger(p->drawingGroupFlags); }),
			   DKFunction([p](ValueType& v){ p->drawingGroupFlags = (unsigned int)v.Integer(); }),
			   DKFunction([](const ValueType& v)->bool{return v.ValueType() == DKVariant::TypeInteger; }),
			   DKValue(0));

			return this;
		}

		////////////////////////////////////////////////////////////////////////////////
		// primitiveType		
		void GetDefaultPrimitiveType(DKVariant& v) const
		{
			v = (const DKVariant::VString&)DKPrimitive::TypeToString(target->DefaultPrimitiveType());
		}
		void SetDefaultPrimitiveType(DKVariant& v)
		{
			target->SetDefaultPrimitiveType(DKPrimitive::StringToType(v.String()));
		}
		bool CheckDefaultPrimitiveType(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeString;
		}
		////////////////////////////////////////////////////////////////////////////////
		// drawFace
		void GetDrawFace(DKVariant& v) const
		{
			if (target->drawFace == DrawFaceCW)
				v = (const DKVariant::VString&)L"cw";
			else if (target->drawFace == DrawFaceCCW)
				v = (const DKVariant::VString&)L"ccw";
			else if (target->drawFace == DrawFaceNone)
				v = (const DKVariant::VString&)L"none";
			else
				v = (const DKVariant::VString&)L"both";
		}
		void SetDrawFace(DKVariant& v)
		{
			if (v.String().CompareNoCase(L"cw") == 0)
				target->drawFace = DrawFaceCW;
			else if (v.String().CompareNoCase(L"ccw") == 0)
				target->drawFace = DrawFaceCCW;
			else if (v.String().CompareNoCase(L"none") == 0)
				target->drawFace = DrawFaceNone;
			else
				target->drawFace = DrawFaceBoth;
		}
		bool CheckDrawFace(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeString;
		}
		////////////////////////////////////////////////////////////////////////////////
		// boundingAABox, boundingSphere
		void GetBoundingInfo(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypePairs);
			if (target->aabb.IsValid())
			{
				v.Pairs().Insert(L"aabb.max", (const DKVariant::VVector3&)target->aabb.positionMax);
				v.Pairs().Insert(L"aabb.min", (const DKVariant::VVector3&)target->aabb.positionMin);
			}
			if (target->boundingSphere.IsValid())
			{
				v.Pairs().Insert(L"bsphere.center", (const DKVariant::VVector3&)target->boundingSphere.center);
				v.Pairs().Insert(L"bsphere.radius", (DKVariant::VFloat)target->boundingSphere.radius);
			}
		}
		void SetBoundingInfo(DKVariant& v)
		{
			target->aabb = DKAabb();
			target->boundingSphere = DKSphere();

			const DKVariant::VPairs::Pair* aabbMax = v.Pairs().Find(L"aabb.max");
			const DKVariant::VPairs::Pair* aabbMin = v.Pairs().Find(L"aabb.min");
			if (aabbMax && aabbMax->value.ValueType() == DKVariant::TypeVector3 &&
				aabbMin && aabbMin->value.ValueType() == DKVariant::TypeVector3)
			{
				target->aabb.positionMax = aabbMax->value.Vector3();
				target->aabb.positionMin = aabbMin->value.Vector3();
			}
			const DKVariant::VPairs::Pair* bsphereCenter = v.Pairs().Find(L"bsphere.center");
			const DKVariant::VPairs::Pair* bsphereRadius = v.Pairs().Find(L"bsphere.radius");
			if (bsphereCenter && bsphereCenter->value.ValueType() == DKVariant::TypeVector3 &&
				bsphereRadius && bsphereRadius->value.ValueType() == DKVariant::TypeFloat)
			{
				target->boundingSphere.center = bsphereCenter->value.Vector3();
				target->boundingSphere.radius = bsphereRadius->value.Float();
			}
		}
		bool CheckBoundingInfo(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypePairs;
		}
		////////////////////////////////////////////////////////////////////////////////
		// materialProperties
		void GetMaterialProperties(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypePairs);
			target->materialProperties.EnumerateForward([&v](const PropertyMap::Pair& pair)
			{
				const PropertyArray& properties = pair.value;

				if (properties.integers.IsEmpty() && properties.floatings.IsEmpty())
				{
				}
				else
				{
					DKVariant vprop(DKVariant::TypePairs);

					// save as data format. (performance hit),
					// use external editors to modify.
					if (properties.integers.Count() > 0)
					{
						DKVariant vintegers(DKVariant::TypeData);
						vintegers.Data().SetContent((const int*)properties.integers, properties.integers.Count() * sizeof(int));
						vprop.Pairs().Insert(L"integers", vintegers);
					}
					if (properties.floatings.Count() > 0)
					{
						DKVariant vfloats(DKVariant::TypeData);
						vfloats.Data().SetContent((const float*)properties.floatings, properties.floatings.Count() * sizeof(float));
						vprop.Pairs().Insert(L"floats", vfloats);
					}

					v.Pairs().Insert(pair.key, vprop);
				}
			});
		}
		void SetMaterialProperties(DKVariant& v)
		{
			target->materialProperties.Clear();
			v.Pairs().EnumerateForward([this](const DKVariant::VPairs::Pair& pair)
			{
				if (pair.value.ValueType() == DKVariant::TypePairs)
				{
					const DKVariant::VPairs::Pair* pIntegers = pair.value.Pairs().Find(L"integers");
					const DKVariant::VPairs::Pair* pFloats = pair.value.Pairs().Find(L"floats");

					PropertyArray properties;

					if (pIntegers && pIntegers->value.ValueType() == DKVariant::TypeData)
					{
						const DKVariant::VData& data = pIntegers->value.Data();
						properties.integers.Add((const int*)data.LockShared(), data.Length() / sizeof(int));
						data.UnlockShared();
					}
					if (pFloats && pFloats->value.ValueType() == DKVariant::TypeData)
					{
						const DKVariant::VData& data = pFloats->value.Data();
						properties.floatings.Add((const float*)data.LockShared(), data.Length() / sizeof(float));
						data.UnlockShared();
					}

					if (properties.integers.IsEmpty() && properties.floatings.IsEmpty())
					{
					}
					else
					{
						target->materialProperties.Insert(pair.key, properties);
					}
				}
			});
		}
		bool CheckMaterialProperties(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypePairs;
		}
		////////////////////////////////////////////////////////////////////////////////
		// samplers
		void GetSamplers(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypePairs);
			target->samplers.EnumerateForward([&v](const TextureSamplerMap::Pair& pair)
			{
				DKVariant texArray(DKVariant::TypeArray);
				for (size_t i = 0; i < pair.value.textures.Count(); ++i)
				{
					const DKResource* res = pair.value.textures.Value(i).SafeCast<DKResource>();
					if (res)
					{
						DKString texKey = DKString::Format("Sampler:%ls:%u", (const wchar_t*)pair.key, i);
						texArray.Array().Add((const DKVariant::VString&)texKey);
					}
				}
				if (texArray.Array().Count() > 0)
					v.Pairs().Insert(pair.key, texArray);
			});
		}
		void SetSamplers(DKVariant& v)
		{
			this->samplerKeyMap.Clear();
			v.Pairs().EnumerateForward([this](DKVariant::VPairs::Pair& pair)
			{
				if (pair.value.ValueType() == DKVariant::TypeArray)
				{
					DKVariant::VArray& texArray = pair.value.Array();
					LocalTextureKeyArray texKeys;
					for (DKVariant& var : pair.value.Array())
					{
						if (var.ValueType() == DKVariant::TypeString)
							texKeys.Add(var.String());
					}
					if (texKeys.Count() > 0)
						this->samplerKeyMap.Insert(pair.key, texKeys);
				}
			});
		}
		bool CheckSamplers(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypePairs;
		}
		////////////////////////////////////////////////////////////////////////////////
		// textures		
		void GetTextureMap(ExternalMapType& v)
		{
			target->samplers.EnumerateForward([&v](TextureSamplerMap::Pair& pair)
			{
				for (size_t i = 0; i < pair.value.textures.Count(); ++i)
				{
					DKResource* res = pair.value.textures.Value(i).SafeCast<DKResource>();
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
		////////////////////////////////////////////////////////////////////////////////
		// textureSamplers	
		void GetTexSamplerMap(ExternalMapType& v)
		{
			target->samplers.EnumerateForward([&v](TextureSamplerMap::Pair& pair)
			{
				DKResource* res = pair.value.sampler.SafeCast<DKResource>();
				if (res)
				{
					v.Insert(pair.key, res);
				}
			});
		}
		void SetTexSamplerMap(ExternalMapType& v)
		{
			this->texSamplerMap.Clear();
			v.EnumerateForward([this](ExternalMapType::Pair& pair)
			{
				DKTextureSampler* p = pair.value.SafeCast<DKTextureSampler>();
				if (p)
					this->texSamplerMap.Insert(pair.key, p);
			});
		}
		////////////////////////////////////////////////////////////////////////////////
		// material
		void GetMaterial(ExternalType& v)
		{
			v = target->material.SafeCast<DKResource>();
		}
		void SetMaterial(ExternalType& v)
		{
			target->material = v.SafeCast<DKMaterial>();
		}
		void ResetMaterial(void)
		{
			target->material = NULL;
		}
		bool CheckMaterial(const ExternalType& v) const
		{
			return v.SafeCast<DKMaterial>() != NULL;
		}
		////////////////////////////////////////////////////////////////////////////////
		// callback
		void Callback(State s)
		{
			if (s == StateDeserializeBegin)
			{
				this->samplerKeyMap.Clear();
				this->textureMap.Clear();
				this->texSamplerMap.Clear();
			}
			else if (s == StateDeserializeSucceed)
			{
				target->samplers.Clear();
				this->samplerKeyMap.EnumerateForward([this](LocalSamplerKeyMap::Pair& pair)
				{
					TextureArray texArray;

					for (const DKString& texName : pair.value)
					{
						LocalTextureMap::Pair* pTex = this->textureMap.Find(texName);
						if (pTex && pTex->value)
						{
							texArray.Add(pTex->value);
						}
						else
						{
							DKLog("Error: Texture:%ls not found!\n", (const wchar_t*)texName);
							const unsigned char whitePixel[] = { 0xff, 0xff, 0xff };
							texArray.Add(DKTexture2D::Create(1, 1, DKTexture::FormatRGB, DKTexture::TypeUnsignedByte, whitePixel).SafeCast<DKTexture>());
						}
					}

					DKTextureSampler* texSampler = NULL;
					auto p = this->texSamplerMap.Find(pair.key);
					if (p)
						texSampler = p->value;

					if (texArray.Count() > 0)
					{
						TextureSampler& ts = target->samplers.Value(pair.key);
						ts.sampler = texSampler;
						ts.textures = texArray;
					}
				});
				this->samplerKeyMap.Clear();
				this->textureMap.Clear();
				this->texSamplerMap.Clear();
			}
		}

		typedef DKArray<DKString>							LocalTextureKeyArray;
		typedef DKMap<DKString, LocalTextureKeyArray>		LocalSamplerKeyMap;
		typedef DKMap<DKString, DKObject<DKTexture>>		LocalTextureMap;
		typedef DKMap<DKString, DKObject<DKTextureSampler>>	LocalTexSamplerMap;

		LocalSamplerKeyMap samplerKeyMap;
		LocalTextureMap textureMap;
		LocalTexSamplerMap texSamplerMap;
		DKObject<DKMesh> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
