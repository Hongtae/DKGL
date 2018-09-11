//
//  File: DKConeShape.h
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
	/// a cone shape for detecting collision.
	class DKGL_API DKConeShape : public DKConvexShape
	{
	public:
		DKConeShape(float radius, float height, UpAxis up = UpAxis::Top);
		~DKConeShape();

		float Radius() const;
		float Height() const;

		float ScaledRadius() const;
		float ScaledHeight() const;

		UpAxis BaseAxis() const;
	};
}
