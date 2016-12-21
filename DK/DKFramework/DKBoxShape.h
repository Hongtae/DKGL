//
//  File: DKBoxShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKMatrix3.h"
#include "DKQuaternion.h"
#include "DKPolyhedralConvexShape.h"

namespace DKFramework
{
	/// @brief box shape for detecting collision.
	class DKGL_API DKBoxShape : public DKPolyhedralConvexShape
	{
	public:
		DKBoxShape(float u, float v, float w);
		DKBoxShape(const DKVector3& halfExtents);
		~DKBoxShape(void);

		DKVector3 HalfExtents(void) const;
		DKVector3 HalfExtentsWithMargin(void) const;

		DKVector3 ScaledHalfExtents(void) const;
		DKVector3 ScaledHalfExtentsWithMargin(void) const;
	};
}
