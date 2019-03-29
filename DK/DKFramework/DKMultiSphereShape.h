//
//  File: DKMultiSphereShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConvexShape.h"
#include "DKSphere.h"

namespace DKFramework
{
	/// @brief a collision shape, convex hull of a collection of spheres.
	class DKGL_API DKMultiSphereShape : public DKConvexShape
	{
	public:
		DKMultiSphereShape(const DKSphere* spheres, size_t numSpheres);
		DKMultiSphereShape(const DKVector3* centers, const float* radii, size_t numSpheres);
		~DKMultiSphereShape();

		size_t NumberOfSpheres() const;
		DKVector3 CenterOfSphereAtIndex(unsigned int index) const;
		float RadiusOfSphereAtIndex(unsigned int index) const;
	};
}
