//
//  File: DKBox.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKQuaternion.h"

////////////////////////////////////////////////////////////////////////////////
// DKBox
// Oriented bounding box, a utility class.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLine;
	class DKLIB_API DKBox
	{
	public:
		DKBox(void);
		// c,x,y,z is absolute position value.
		DKBox(const DKVector3& c, const DKVector3& x, const DKVector3& y, const DKVector3& z);
		// c: position of center
		// u,v,w: unit vector of box direction.
		// hu, hv, hw: box extent
		DKBox(const DKVector3& c, const DKVector3& u, const DKVector3& v, float hu, float hv, float hw);
		DKBox(const DKVector3& c, const DKVector3& u, const DKVector3& v, const DKVector3& w, float hu, float hv, float hw);
		// box from matrix
		DKBox(const DKMatrix4& m);

		static DKBox Intersection(const DKBox& b1, const DKBox& b2);
		static DKBox Union(const DKBox& b1, const DKBox& b2);

		bool IsValid(void) const;
		bool IsPointInside(const DKVector3& pos) const;

		float Volume(void) const;
		DKMatrix4 LocalTransform(void) const;  // transform of box
		DKMatrix4 AffineTransform(void) const; // affine transform of box
		DKQuaternion Orientation(void) const;  // orientation of box
		DKBox& RotateOrientation(const DKQuaternion& q);

		bool Intersect(const DKLine& line, DKVector3* p = NULL) const;

		DKVector3 center;
		DKVector3 u,v,w;	// unit vectors of each extent direction.
		float hu, hv, hw;
	};
}
