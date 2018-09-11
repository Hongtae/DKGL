//
//  File: DKConcaveShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCollisionShape.h"

namespace DKFramework
{
	/// @brief
	/// a concave shape, for collision object or static rigid body.
	class DKGL_API DKConcaveShape : public DKCollisionShape
	{
	public:
		~DKConcaveShape();

		struct Triangle
		{
			DKVector3 pos[3];
			int partId;
			int triangleIndex;
		};
		typedef DKFunctionSignature<void (const Triangle&)> TriangleEnumerator;

		void EnumerateTrianglesInsideAabb(TriangleEnumerator* e, const DKVector3 aabbMin, const DKVector3& aabbMax) const;

	protected:
		DKConcaveShape(ShapeType t, class btConcaveShape* context);
	};
}
