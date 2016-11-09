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

///////////////////////////////////////////////////////////////////////////////
// DKSphereShape
// an implicit sphere class. centered local origin with radius.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKGL_API DKSphereShape : public DKConvexShape
	{
	public:
		DKSphereShape(float radius);
		~DKSphereShape(void);

		float Radius(void) const;
		float ScaledRadius(void) const;
	};
}
