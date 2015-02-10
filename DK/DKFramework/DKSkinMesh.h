//
//  File: DKSkinMesh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKStaticMesh.h"

////////////////////////////////////////////////////////////////////////////////
// DKSkinMesh
// a skinning mesh object which can be bound with bone node.
// using static-mesh based data, transfer skinning bone transforms to GPU when
// bind to context.
//
// Note:
//  all nodes (DKModel) can be used as bone.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKSkinMesh : public DKStaticMesh
	{
	public:
		typedef DKFoundation::DKString NodeId;
		struct Bone
		{
			NodeId id;
			DKMatrix4 tm;		// bone's world transform
		};
		typedef DKFoundation::DKArray<Bone> BoneArray;
		typedef DKFoundation::DKArray<DKModel*> NodeArray;
		typedef DKFoundation::DKMap<NodeId, DKModel*> NodeMap;

		DKSkinMesh(void);
		~DKSkinMesh(void);

		void SetBones(const BoneArray& bones);
		size_t NumberOfBones(void) const;
		Bone& BoneAtIndex(unsigned int index);
		const Bone& BoneAtIndex(unsigned int index) const;

		void ClearTransformNodes(void);
		bool ResolveTransformNodes(void);
		bool ResolveTransformNodes(const NamedObjectMap&);

		bool NodeResolved(void) const	{ return transformNodeResolved; }

		DKFoundation::DKObject<DKSerializer> Serializer(void) override;

	protected:
		void OnUpdateTreeReferences(NamedObjectMap&, UUIDObjectMap&) override;
		void OnUpdateSceneState(const DKNSTransform&) override;

		bool BindTransform(DKSceneState&) const override;

		DKFoundation::DKObject<DKModel> Clone(UUIDObjectMap&) const;
		DKSkinMesh* Copy(UUIDObjectMap&, const DKSkinMesh*);

	private:
		struct TransformData
		{
			Bone bone;
			const DKModel* node;

			DKMatrix4 initInvTM;	// inverse of initial transform
			DKMatrix4 nodeTM;
		};
		typedef DKFoundation::DKArray<TransformData> TransformDataArray;
		TransformDataArray transformData;
		bool transformNodeResolved;
	};
}
