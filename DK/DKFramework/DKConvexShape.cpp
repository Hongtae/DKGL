//
//  File: DKConvexShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKConvexShape.h"

using namespace DKGL;
using namespace DKGL::Private;

DKConvexShape::DKConvexShape(ShapeType t, btConvexShape* impl)
	: DKCollisionShape(t, impl)
{
	DKASSERT_DEBUG( this->impl->isConvex() );
}

DKConvexShape::~DKConvexShape(void)
{
	DKASSERT_DEBUG( this->impl->isConvex() );
}

void DKConvexShape::Project(const DKNSTransform& t, const DKVector3& dir, float& min, float& max) const
{
	static_cast<const btConvexShape*>(this->impl)->project(
		BulletTransform(t), BulletVector3(dir), min, max);
}

bool DKConvexShape::Intersect(const DKConvexShape* shapeA, const DKNSTransform& ta, const DKConvexShape* shapeB, const DKNSTransform& tb)
{
	btConvexShape* convexA = (btConvexShape*)shapeA->impl;
	btConvexShape* convexB = (btConvexShape*)shapeB->impl;

	DKASSERT_DEBUG(convexA && convexA->getUserPointer() == shapeA);
	DKASSERT_DEBUG(convexB && convexB->getUserPointer() == shapeB);

	btDiscreteCollisionDetectorInterface::ClosestPointInput input;
	input.m_transformA = BulletTransform(ta);
	input.m_transformB = BulletTransform(tb);
	btPointCollector result;

	btVoronoiSimplexSolver simplexSolver;
	btMinkowskiPenetrationDepthSolver penetrationDepthSolver;

	btGjkPairDetector(convexA, convexB, &simplexSolver, &penetrationDepthSolver).getClosestPointsNonVirtual(input, result, NULL);

	return result.m_distance <= 0;
}

bool DKConvexShape::RayTest(const DKNSTransform& trans, const DKLine& line, DKVector3* p) const
{
	const btConvexShape* shape = (const btConvexShape*)this->impl;
	DKASSERT_DEBUG(shape && shape->getUserPointer() == this);

	const btTransform transform = BulletTransform(trans);
	const btVector3 rayFrom = BulletVector3(line.begin);
	const btVector3 rayTo = BulletVector3(line.end);

	btTransform rayFromTrans;
	btTransform rayToTrans;

	rayFromTrans.setIdentity();
	rayFromTrans.setOrigin(rayFrom);
	rayToTrans.setIdentity();
	rayToTrans.setOrigin(rayTo);

	btVector3 aabbMin,aabbMax;
	shape->getAabb(transform, aabbMin, aabbMax);
	btScalar hitLambda = 1.f;
	btVector3 hitNormal;

	if (btRayAabb(rayFrom, rayTo, aabbMin, aabbMax, hitLambda, hitNormal))
	{
#if 0
		btScalar closestHitResults = 1.f;
		btConvexCast::CastResult rayResult;
		btSphereShape pointShape(0.0f);
		btVoronoiSimplexSolver	simplexSolver;

		btSubsimplexConvexCast convexCaster(&pointShape, shape, &simplexSolver);
		if (convexCaster.calcTimeOfImpact(rayFromTrans, rayToTrans, transform, transform, rayResult))
		{
			if (rayResult.m_fraction < closestHitResults)
			{
				if (p)
					*p = BulletVector3( rayResult.m_hitPoint );

				return true;
			}
		}
#else
		btCollisionObject tmpObj;
		tmpObj.setWorldTransform(transform);

		btScalar closestHitResults = 1.f;
		btCollisionWorld::ClosestRayResultCallback resultCallback(rayFrom, rayTo);

		btConvexCast::CastResult rayResult;
		btSphereShape pointShape(0.0f);

		btCollisionWorld::rayTestSingle(rayFromTrans, rayToTrans, &tmpObj, shape, transform, resultCallback);
		if (resultCallback.hasHit())
		{
			if (p)
				*p = BulletVector3( resultCallback.m_hitPointWorld );

			return true;
		}
#endif
	}
	return false;
}
