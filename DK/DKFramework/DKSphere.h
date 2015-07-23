//
//  File: DKSphere.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"

////////////////////////////////////////////////////////////////////////////////
// DKSphere
// a sphere class, can calculate collision.
////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 4)
namespace DKFramework
{
	class DKLine;
	class DKGL_API DKSphere
	{
	public:
		DKSphere(void);
		DKSphere(const DKVector3& c, float r);

		// bigger sphere, union of s1, s2 merged sphere.
		static DKSphere Union(const DKSphere& s1, const DKSphere& s2);
		// smaller sphere, intersection between s1, s2.
		static DKSphere Intersection(const DKSphere&s1, const DKSphere&s2);

		bool IsValid(void) const;
		bool IsPointInside(const DKVector3& pos) const;
		float Volume(void) const;

		bool RayTest(const DKLine& ray, DKVector3* hitPoint = NULL) const;

		DKVector3	center;
		float		radius;
	};
}
#pragma pack(pop)
