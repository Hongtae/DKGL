﻿//
//  File: DKCapsuleShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKConvexShape.h"

///////////////////////////////////////////////////////////////////////////////
// DKCapsuleShape
// capsule shape for detecting collision.
///////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKCapsuleShape : public DKConvexShape
	{
	public:
		DKCapsuleShape(float radius, float height, UpAxis up = UpAxis::Top);
		~DKCapsuleShape(void);

		float Radius(void) const;
		float HalfHeight(void) const;

		float ScaledRadius(void) const;
		float ScaledHalfHeight(void) const;

		UpAxis BaseAxis(void) const;
	};
}
