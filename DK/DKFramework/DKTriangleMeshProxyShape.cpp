//
//  File: DKTriangleMeshProxyShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKTriangleMeshProxyShape.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		struct TriangleMeshProxyShape : public btConcaveShape
		{
			DKTriangleMeshProxyShape* shape;
			btVector3 localScale;

			TriangleMeshProxyShape(DKTriangleMeshProxyShape* s)
			: shape(s), localScale(1, 1, 1)
			{
				m_shapeType = CUSTOM_CONCAVE_SHAPE_TYPE;
			}

			// overrides
			void processAllTriangles(btTriangleCallback* callback,const btVector3& aabbMin,const btVector3& aabbMax) const override
			{
				btVector3 pos[3];
				auto forwardCb = [&](DKTriangle& tri, int pid, int tid)
				{
					pos[0] = BulletVector3(tri.position1);
					pos[1] = BulletVector3(tri.position2);
					pos[2] = BulletVector3(tri.position3);
					callback->processTriangle(pos, pid, tid);
				};
				DKAabb aabb = { BulletVector3(aabbMin), BulletVector3(aabbMax) };
				shape->QueryTrianglesInAabb(aabb, DKFunction(forwardCb));
			}
			void getAabb(const btTransform& trans,btVector3& aabbMin,btVector3& aabbMax) const override
			{
				DKAabb aabb = shape->Aabb();
				btVector3 localAabbMin = BulletVector3(aabb.positionMin);
				btVector3 localAabbMax = BulletVector3(aabb.positionMax);

				btVector3 localHalfExtents = (localAabbMax - localAabbMin) * localScale * btScalar(0.5);
				localHalfExtents += btVector3(getMargin(), getMargin(), getMargin());
				btVector3 localCenter = (localAabbMax + localAabbMin) * btScalar(0.5);

				btMatrix3x3 abs_b = trans.getBasis().absolute();

				btVector3 center = trans(localCenter);
				btVector3 extent = localHalfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);
				aabbMin = center - extent;
				aabbMax = center + extent;
			}
			void getBoundingSphere(btVector3& center,btScalar& radius) const override
			{
				btConcaveShape::getBoundingSphere(center, radius);
			}
			void setLocalScaling(const btVector3& scaling) override
			{
				localScale = scaling;
			}
			const btVector3& getLocalScaling(void) const override
			{
				return localScale;
			}
			void calculateLocalInertia(btScalar mass,btVector3& inertia) const override
			{
				btAssert(0);
				inertia.setValue(btScalar(0.),btScalar(0.),btScalar(0.));
			}
			const char*	getName(void) const override
			{
				return "DKTriangleMeshProxyShape";
			}
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKTriangleMeshProxyShape::DKTriangleMeshProxyShape(void)
: DKConcaveShape(ShapeType::Custom, new TriangleMeshProxyShape(this))
{
}

DKTriangleMeshProxyShape::~DKTriangleMeshProxyShape(void)
{
}
