//
//  File: DKSphereShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKSphereShape.h"

using namespace DKGL;
using namespace DKGL;
using namespace DKGL::Private;

DKSphereShape::DKSphereShape(float radius)
	: DKConvexShape(ShapeType::Sphere, new btSphereShape(radius))
{
}

DKSphereShape::~DKSphereShape(void)
{
}

float DKSphereShape::Radius(void) const
{
	btSphereShape* shape = static_cast<btSphereShape*>(this->impl);
	float r = shape->getRadius();
	float s = shape->getLocalScaling().x();

	return r / s;
}

float DKSphereShape::ScaledRadius(void) const
{
	return static_cast<btSphereShape*>(this->impl)->getRadius();
}
