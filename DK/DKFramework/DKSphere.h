//
//  File: DKSphere.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"

////////////////////////////////////////////////////////////////////////////////
// DKSphere
// a sphere class, can calculate collision.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLine;
	class DKLIB_API DKSphere
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

		bool Intersect(const DKLine& line, DKVector3* p = NULL) const;

		DKVector3	center;
		float		radius;
	};
}
