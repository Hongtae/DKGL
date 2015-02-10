//
//  File: DKConcaveShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKConcaveShape.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;

DKConcaveShape::DKConcaveShape(ShapeType t, btConcaveShape* impl)
	: DKCollisionShape(t, impl)
{
	DKASSERT_DEBUG( this->impl->isConcave() );
}

DKConcaveShape::~DKConcaveShape(void)
{
	DKASSERT_DEBUG( this->impl->isConcave() );
}

void DKConcaveShape::EnumerateTrianglesInsideAABB(TriangleEnumerator* e, const DKVector3 aabbMin, const DKVector3& aabbMax) const
{
	struct Callback : public btTriangleCallback
	{
		void processTriangle (btVector3 *triangle, int partId, int triangleIndex)
		{
			Triangle tri = {
				{BulletVector3(triangle[0]), BulletVector3(triangle[1]), BulletVector3(triangle[2])},
				partId,
				triangleIndex
			};
			e->Invoke(tri);
		}
		const TriangleEnumerator* e;
	};

	if (e)
	{
		Callback c;
		c.e = e;

		static_cast<const btConcaveShape*>(this->impl)->processAllTriangles(&c, BulletVector3(aabbMin), BulletVector3(aabbMax));
	}
}

