//
//  File: DKSkinMesh.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKSkinMesh.h"
#include "DKModel.h"
#include "DKAffineTransform3.h"

using namespace DKGL;
using namespace DKGL;

DKSkinMesh::DKSkinMesh(void)
: transformNodeResolved(false)
{
}

DKSkinMesh::~DKSkinMesh(void)
{
}

DKObject<DKModel> DKSkinMesh::Clone(UUIDObjectMap& uuids) const
{
	return DKObject<DKSkinMesh>::New()->Copy(uuids, this);
}

DKSkinMesh* DKSkinMesh::Copy(UUIDObjectMap& uuids, const DKSkinMesh* mesh)
{
	if (DKStaticMesh::Copy(uuids, mesh))
	{
		this->transformData = mesh->transformData;
		this->transformNodeResolved = false;
		return this;
	}
	return NULL;
}

void DKSkinMesh::SetBones(const BoneArray& bones)
{
	this->transformData.Clear();
	this->transformData.Reserve(bones.Count());
	for (const Bone& bone : bones)
	{
		TransformData data = {bone, NULL, DKMatrix4(bone.tm).Inverse(), DKMatrix4::identity};
		this->transformData.Add(data);
	}
}

size_t DKSkinMesh::NumberOfBones(void) const
{
	return this->transformData.Count();
}

DKSkinMesh::Bone& DKSkinMesh::BoneAtIndex(unsigned int index)
{
	DKASSERT_DEBUG(index < this->transformData.Count());
	return this->transformData.Value(index).bone;
}

const DKSkinMesh::Bone& DKSkinMesh::BoneAtIndex(unsigned int index) const
{
	DKASSERT_DEBUG(index < this->transformData.Count());
	return this->transformData.Value(index).bone;
}

void DKSkinMesh::ClearTransformNodes(void)
{
	for (TransformData& data : this->transformData)
	{
		data.node = NULL;
		data.nodeTM.Identity();
	}
	if (this->transformData.Count() > 0)
		transformNodeResolved = false;
	else
		transformNodeResolved = true;
}

bool DKSkinMesh::ResolveTransformNodes(void)
{
	ClearTransformNodes();
	transformNodeResolved = false;

	DKModel* root = RootObject();
	for (TransformData& data : this->transformData)
	{
		DKModel* node = root->FindDescendant(data.bone.id);
		if (node)
		{
			data.node = node;
			data.nodeTM.Identity();
		}
		else
		{
			DKLog("DKSkinMesh::ResolveNode ERROR: Node(%ls) missing!\n", (const wchar_t*)data.bone.id);
			ClearTransformNodes();
			return false;
		}
	}
	transformNodeResolved = true;
	return true;
}

bool DKSkinMesh::ResolveTransformNodes(const NamedObjectMap& map)
{
	ClearTransformNodes();
	transformNodeResolved = false;

	for (TransformData& data : this->transformData)
	{
		auto p = map.Find(data.bone.id);
		if (p)
		{
			data.node = p->value;
			data.nodeTM.Identity();
		}
		else
		{
			DKLog("DKSkinMesh::ResolveNode ERROR: Node(%ls) missing!\n", (const wchar_t*)data.bone.id);
			ClearTransformNodes();
			return false;
		}
	}
	transformNodeResolved = true;
	return true;
}

void DKSkinMesh::OnUpdateTreeReferences(NamedObjectMap& names, UUIDObjectMap& uuids)
{
	ResolveTransformNodes(names);
}

void DKSkinMesh::OnUpdateSceneState(const DKNSTransform& parentWorldTransform)
{
	DKStaticMesh::OnUpdateSceneState(parentWorldTransform);
	// bone's world-transform should apply local-transform of baseObject.
	if (transformNodeResolved)
	{
		DKNSTransform invWorldTrans = this->worldTransform;
		invWorldTrans.Inverse();

		for (TransformData& data : this->transformData)
		{
			DKNSTransform nodeTrans = data.node->WorldTransform() * invWorldTrans;
			data.nodeTM = data.initInvTM * nodeTrans.Matrix4();
		}
	}
}

bool DKSkinMesh::BindTransform(DKSceneState& st) const
{
	if (DKStaticMesh::BindTransform(st))
	{
		st.linearTransformMatrixArray.Clear();
		st.affineTransformMatrixArray.Clear();
		st.positionArray.Clear();

		st.affineTransformMatrixArray.Reserve(this->transformData.Count());
		st.linearTransformMatrixArray.Reserve(this->transformData.Count());
		st.positionArray.Reserve(this->transformData.Count());

		for (const TransformData& data : this->transformData)
		{
			DKAffineTransform3 trans(data.nodeTM);
			st.affineTransformMatrixArray.Add(data.nodeTM);
			st.linearTransformMatrixArray.Add(trans.matrix3);
			st.positionArray.Add(trans.translation);
		}
		return true;
	}
	return false;
}

DKObject<DKSerializer> DKSkinMesh::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		BoneArray bones;
		DKObject<DKSkinMesh> target;

		////////////////////////////////////////////////////////////////////////////////
		// Bones
		void GetBones(DKVariant& v) const
		{
			DKVariant::VArray& varray = v.SetValueType(DKVariant::TypeArray).Array();

			for (size_t i = 0; i < target->NumberOfBones(); ++i)
			{
				const Bone& bone = target->BoneAtIndex(i);

				size_t idx = varray.Add(DKVariant::TypePairs);
				DKVariant::VPairs& pairs = varray.Value(idx).Pairs();

				pairs.Insert(L"id", (const DKVariant::VString&)bone.id);
				pairs.Insert(L"tm", (const DKVariant::VMatrix4&)bone.tm);
			}
		}
		void SetBones(DKVariant& v)
		{
			this->bones.Clear();
			this->bones.Reserve(v.Array().Count());

			for (DKVariant var : v.Array())
			{
				auto id = var.Pairs().Find(L"id");
				auto tm = var.Pairs().Find(L"tm");
				
				Bone bone = { id->value.String(), tm->value.Matrix4() };
				this->bones.Add(bone);
			}
		}
		bool CheckBones(const DKVariant& v)
		{
			if (v.ValueType() == DKVariant::TypeArray)
			{
				for (DKVariant var : v.Array())
				{
					if (var.ValueType() != DKVariant::TypePairs)
						return false;

					auto id = var.Pairs().Find(L"id");
					auto tm = var.Pairs().Find(L"tm");
					if (id && id->value.ValueType() == DKVariant::TypeString &&
						tm && tm->value.ValueType() == DKVariant::TypeMatrix4)
					{
					}
					else
						return false;
				}
				return true;
			}
			return false;
		}
		////////////////////////////////////////////////////////////////////////////////
		// callback
		void Callback(State s)
		{
			if (s == StateDeserializeSucceed)
			{
				target->SetBones(this->bones);
			}
			this->bones.Clear();
		}

		DKSerializer* Init(DKSkinMesh* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKSkinMesh");
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"super", target->DKStaticMesh::Serializer(), NULL);

			this->Bind(L"bones",
				DKFunction(this, &LocalSerializer::GetBones),
				DKFunction(this, &LocalSerializer::SetBones),
				DKFunction(this, &LocalSerializer::CheckBones),
				DKFunction([this] {this->bones.Clear();})->Invocation());

			return this;
		}
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
