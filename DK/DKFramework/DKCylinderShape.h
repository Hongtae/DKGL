//
//  File: DKCylinderShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKConvexShape.h"

namespace DKFramework
{
	/// @brief cylinder shape for detecting collision.
	class DKGL_API DKCylinderShape : public DKConvexShape
	{
	public:
		DKCylinderShape(float u, float v, float w, UpAxis up = UpAxis::Top);
		DKCylinderShape(const DKVector3& halfExtents, UpAxis up = UpAxis::Top);
		~DKCylinderShape();

		DKVector3 HalfExtents() const;
		DKVector3 HalfExtentsWithMargin() const;

		DKVector3 ScaledHalfExtents() const;
		DKVector3 ScaledHalfExtentsWithMargin() const;

		UpAxis BaseAxis() const;
	};
}
