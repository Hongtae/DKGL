//
//  File: DKAABox.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKQuaternion.h"

////////////////////////////////////////////////////////////////////////////////
// DKAABox
// a simple AABB (Axis Aligned Bounding Box) utility class.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLine;
	class DKBox;
	class DKLIB_API DKAABox
	{
	public:
		DKAABox(void);
		DKAABox(const DKVector3& posMin, const DKVector3& posMax);

		static DKAABox Intersection(const DKAABox& b1, const DKAABox& b2);
		static DKAABox Union(const DKAABox& b1, const DKAABox& b2);

		bool IsValid(void) const;
		bool IsPointInside(const DKVector3& pos) const;
		DKVector3 Center(void) const;
		float Volume(void) const;
		DKBox Box(void) const;

		bool Intersect(const DKLine& line, DKVector3* p = NULL) const;
		bool Intersect(const DKAABox& box) const;

		DKVector3	positionMax;
		DKVector3	positionMin;
	};
}
