//
//  File: DKMultiSphereShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKConvexShape.h"
#include "DKSphere.h"

///////////////////////////////////////////////////////////////////////////////
// DKMultiSphereShape
// a collision shape, convex hull of a collection of spheres.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKMultiSphereShape : public DKConvexShape
	{
	public:
		DKMultiSphereShape(const DKSphere* spheres, size_t numSpheres);
		DKMultiSphereShape(const DKVector3* centers, const float* radii, size_t numSpheres);
		~DKMultiSphereShape(void);

		size_t NumberOfSpheres(void) const;
		DKVector3 CenterOfSphereAtIndex(unsigned int index) const;
		float RadiusOfSphereAtIndex(unsigned int index) const;
	};
}
