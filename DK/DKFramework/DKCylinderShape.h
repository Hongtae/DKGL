//
//  File: DKCylinderShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKConvexShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKCylinderShape
//  cylinder shape for detecting collision.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKCylinderShape : public DKConvexShape
	{
	public:
		DKCylinderShape(float u, float v, float w, UpAxis up = UpAxis::Top);
		DKCylinderShape(const DKVector3& halfExtents, UpAxis up = UpAxis::Top);
		~DKCylinderShape(void);

		DKVector3 HalfExtents(void) const;
		DKVector3 HalfExtentsWithMargin(void) const;

		DKVector3 ScaledHalfExtents(void) const;
		DKVector3 ScaledHalfExtentsWithMargin(void) const;

		UpAxis BaseAxis(void) const;
	};
}
