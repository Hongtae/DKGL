//
//  File: DKSphereShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKConvexShape.h"

///////////////////////////////////////////////////////////////////////////////
// DKSphereShape
// an implicit sphere class. centered local origin with radius.
///////////////////////////////////////////////////////////////////////////////

namespace DKGL
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
