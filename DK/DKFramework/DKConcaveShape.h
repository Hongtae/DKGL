//
//  File: DKConcaveShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKCollisionShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKConcaveShape
// a concave shape, for collision object or static rigid body.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKConcaveShape : public DKCollisionShape
	{
	public:
		~DKConcaveShape(void);

		struct Triangle
		{
			DKVector3 pos[3];
			int partId;
			int triangleIndex;
		};
		typedef DKFoundation::DKFunctionSignature<void (const Triangle&)> TriangleEnumerator;

		void EnumerateTrianglesInsideAABB(TriangleEnumerator* e, const DKVector3 aabbMin, const DKVector3& aabbMax) const;

	protected:
		DKConcaveShape(ShapeType t, class btConcaveShape* context);
	};
}
