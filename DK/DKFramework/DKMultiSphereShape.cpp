//
//  File: DKMultiSphereShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKMultiSphereShape.h"

namespace DKFramework
{
	namespace Private
	{
		inline btMultiSphereShape* CreateMultiSphereShape(const DKSphere* spheres, size_t numSpheres)
		{
			DKArray<btVector3> positions;
			DKArray<btScalar> radii;
			positions.Reserve(numSpheres);
			radii.Reserve(numSpheres);
			for (size_t i = 0; i < numSpheres; ++i)
			{
				positions.Add(btVector3(spheres[i].center.x, spheres[i].center.y, spheres[i].center.z));
				radii.Add(spheres[i].radius);
			}
			return new btMultiSphereShape((const btVector3*)positions, radii, (int)numSpheres);
		}
		inline btMultiSphereShape* CreateMultiSphereShape(const DKVector3* centers, const float* radii, size_t numSpheres)
		{
			DKArray<btVector3> positions;
			positions.Reserve(numSpheres);
			for (size_t i = 0; i < numSpheres; ++i)
				positions.Add(btVector3(centers[i].x, centers[i].y, centers[i].z));
			return new btMultiSphereShape((const btVector3*)positions, radii, (int)numSpheres);
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKMultiSphereShape::DKMultiSphereShape(const DKSphere* spheres, size_t numSpheres)
	: DKConvexShape(ShapeType::MultiSphere, CreateMultiSphereShape(spheres, numSpheres))
{
}

DKMultiSphereShape::DKMultiSphereShape(const DKVector3* centers, const float* radii, size_t numSpheres)
: DKConvexShape(ShapeType::MultiSphere, CreateMultiSphereShape(centers, radii, numSpheres))
{
}

DKMultiSphereShape::~DKMultiSphereShape(void)
{
}

size_t DKMultiSphereShape::NumberOfSpheres(void) const
{
	btMultiSphereShape* shape = static_cast<btMultiSphereShape*>(this->impl);
	int c = shape->getSphereCount();
	DKASSERT_DEBUG( c >= 0);
	return c;
}

DKVector3 DKMultiSphereShape::CenterOfSphereAtIndex(unsigned int index) const
{
	btMultiSphereShape* shape = static_cast<btMultiSphereShape*>(this->impl);
	DKASSERT_DEBUG( index < shape->getSphereCount() );

	return BulletVector3(shape->getSpherePosition(index));
}

float DKMultiSphereShape::RadiusOfSphereAtIndex(unsigned int index) const
{
	btMultiSphereShape* shape = static_cast<btMultiSphereShape*>(this->impl);
	DKASSERT_DEBUG( index < shape->getSphereCount() );

	return shape->getSphereRadius(index);
}
