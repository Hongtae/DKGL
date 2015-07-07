//
//  File: DKBvh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKTriangle.h"
#include "DKTriangleMesh.h"
#include "DKLine.h"
#include "DKRect.h"
#include "DKAabb.h"

////////////////////////////////////////////////////////////////////////////////
// DKBvh
// implementation of BVH (Bounding volume hierarchy) to perform ray-test fast.
//
// NOT IMPLEMENTED YET.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKBvh
	{
	public:
		DKBvh(void);
		~DKBvh(void);

		void Build(DKTriangleMesh* mesh);
		void Rebuild(void);

		bool RayTest(const DKLine& ray, DKVector3* hitPoint = NULL) const;

	private:
		struct QuantizedAabbNode
		{
			unsigned short aabbMin[3];
			unsigned short aabbMax[3];
			union {
				int triangleIndex;		// for leaf-node
				int negativeTreeSize;	// for sub-node
			};
		};
		struct LeafNode
		{
			DKAabb aabb;
			int triangleIndex;
		};
		void BuildInternal(void);
		void BuildTree(LeafNode* nodes, int count);

		DKFoundation::DKObject<DKTriangleMesh> mesh;
		DKFoundation::DKArray<QuantizedAabbNode> nodes;
		DKVector3 aabbOffset;
		DKVector3 aabbScale;
	};
}
