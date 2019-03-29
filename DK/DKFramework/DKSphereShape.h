//
//  File: DKSphereShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKConvexShape.h"

namespace DKFramework
{
	/// @brief an implicit sphere class. centered local origin with radius.
	class DKGL_API DKSphereShape : public DKConvexShape
	{
	public:
		DKSphereShape(float radius);
		~DKSphereShape();

		float Radius() const;
		float ScaledRadius() const;
	};
}
