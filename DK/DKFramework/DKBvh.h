//
//  File: DKBvh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKLine.h"
#include "DKAabb.h"

////////////////////////////////////////////////////////////////////////////////
// DKBvh
// implementation of BVH (Bounding volume hierarchy) to perform ray-test fast.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKBvh
	{
	public:
		struct VolumeInterface
		{
			virtual ~VolumeInterface(void) {}

			virtual int NumberOfObjects(void) const = 0;
			virtual DKAabb AabbForObjectAtIndex(int) = 0;

			virtual void Lock(void) {}
			virtual void Unlock(void) {}
		};

		DKBvh(void);
		~DKBvh(void);

		void Build(VolumeInterface*);
		void Rebuild(void);

		DKAabb Aabb(void) const;

		using RayCastResultCallback = DKFoundation::DKFunctionSignature<bool (int, const DKLine&)>;
		bool RayTest(const DKLine& ray, RayCastResultCallback*) const;

	private:
		struct QuantizedAabbNode // 16 bytes node
		{
			unsigned short aabbMin[3];
			unsigned short aabbMax[3];
			union {
				int triangleIndex;		// for leaf-node
				int negativeTreeSize;	// for sub-node
			};
		};
		void BuildInternal(void);
		void BuildTree(QuantizedAabbNode* nodes, int count);

		DKFoundation::DKObject<VolumeInterface> volume;
		DKFoundation::DKArray<QuantizedAabbNode> nodes;
		DKVector3 aabbOffset;
		DKVector3 aabbScale;
	};
}
