//
//  File: DKAabb.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKQuaternion.h"

////////////////////////////////////////////////////////////////////////////////
// DKAabb
// a simple AABB (Axis Aligned Bounding Box) utility class.
////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 4)
namespace DKFramework
{
	class DKLine;
	class DKBox;
	class DKGL_API DKAabb
	{
	public:
		DKAabb(void);
		DKAabb(const DKVector3& posMin, const DKVector3& posMax);
		void Expand(const DKVector3&);

		static DKAabb Intersection(const DKAabb& b1, const DKAabb& b2);
		static DKAabb Union(const DKAabb& b1, const DKAabb& b2);

		bool IsValid(void) const;
		bool IsPointInside(const DKVector3& pos) const;
		DKVector3 Center(void) const;
		float Volume(void) const;
		DKBox Box(void) const;

		bool Intersect(const DKAabb& box) const;
		bool RayTest(const DKLine& ray, DKVector3* hitPoint = NULL) const;

		DKVector3	positionMax;
		DKVector3	positionMin;
	};
}
#pragma pack(pop)
