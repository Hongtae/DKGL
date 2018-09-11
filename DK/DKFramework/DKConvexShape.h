//
//  File: DKConvexShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCollisionShape.h"

namespace DKFramework
{
	class DKLine;
	/// @brief convex shape for detecting collision.
	class DKGL_API DKConvexShape : public DKCollisionShape
	{
	public:
		~DKConvexShape();

		void Project(const DKNSTransform& t, const DKVector3& dir, float& min, float& max) const;

		static bool Intersect(const DKConvexShape* shapeA, const DKNSTransform& ta, const DKConvexShape* shapeB, const DKNSTransform& tb);
		bool RayTest(const DKNSTransform& trans, const DKLine& line, DKVector3* p = NULL) const;

	protected:
		DKConvexShape(ShapeType t, class btConvexShape* context);
	};
}
