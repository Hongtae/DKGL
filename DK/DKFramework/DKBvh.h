//
//  File: DKBvh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKLine.h"
#include "DKAabb.h"

#pragma pack(push, 4)
namespace DKFramework
{
	/// @brief
	/// implementation of BVH (Bounding volume hierarchy) to perform ray-test faster.
	class DKGL_API DKBvh
	{
	public:
		struct VolumeInterface
		{
			virtual ~VolumeInterface() {}

			virtual int NumberOfObjects() const = 0;
			virtual DKAabb AabbForObjectAtIndex(int) = 0;

			virtual void Lock() {}
			virtual void Unlock() {}
		};

		DKBvh();
		~DKBvh();

		void Build(VolumeInterface*);
		void Rebuild();

		VolumeInterface* Volume() { return volume;}
		const VolumeInterface* Volume() const { return volume;}

		DKAabb Aabb() const;

		/// RayCastResultCallback : filter-callback function,
		///   return false if ray-test no longer necessary.
		///   return true if callback needs next ray hit object continuously.
		/// All filter callback returns false, this function returns false even if one or more
		/// ray hits detected. In this case, you need to get ray-test result from the callback.
		/// parameter: (object-index, line)
		using RayCastResultCallback = DKFunctionSignature<bool (int, const DKLine&)>;
		bool RayTest(const DKLine& ray, RayCastResultCallback*) const;

		/// AabbCastResultCallback : filter-callback function.
		///   return false if aabb-overlap test no longer necessary.
		///   return true if callback needs next overlapped object continously.
		/// All filter callback returns false, this function returns false even if one or more
		/// overlap detected. In this case, you need to get ray-test result from the callback.
		/// parameter: (object-index, aabb)
		using AabbOverlapResultCallback = DKFunctionSignature<bool (int, const DKAabb&)>;
		bool AabbOverlapTest(const DKAabb& aabb, AabbOverlapResultCallback*) const;

	private:
		struct QuantizedAabbNode	// 16 bytes node
		{
			unsigned short aabbMin[3];
			unsigned short aabbMax[3];
			union {
				int32_t objectIndex;		// for leaf-node
				int32_t negativeTreeSize;	// for sub-node
			};
		};

		void BuildInternal();
		void BuildTree(QuantizedAabbNode* nodes, int count);

		DKObject<VolumeInterface> volume;
		DKArray<QuantizedAabbNode> nodes;
		DKVector3 aabbOffset;
		DKVector3 aabbScale;
	};
}
#pragma pack(pop)
