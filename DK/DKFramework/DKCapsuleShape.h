//
//  File: DKCapsuleShape.h
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
	/// @brief
	/// capsule shape for detecting collision.
	class DKGL_API DKCapsuleShape : public DKConvexShape
	{
	public:
		DKCapsuleShape(float radius, float height, UpAxis up = UpAxis::Top);
		~DKCapsuleShape();

		float Radius() const;
		float HalfHeight() const;

		float ScaledRadius() const;
		float ScaledHalfHeight() const;

		UpAxis BaseAxis() const;
	};
}
