//
//  File: DKBoxShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKBoxShape.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;

DKBoxShape::DKBoxShape(float u, float v, float w)
	: DKPolyhedralConvexShape(ShapeType::Box, new btBoxShape(btVector3(u,v,w)))
{
}

DKBoxShape::DKBoxShape(const DKVector3& v)
	: DKPolyhedralConvexShape(ShapeType::Box, new btBoxShape(btVector3(v.x, v.y, v.z)))
{
}

DKBoxShape::~DKBoxShape(void)
{
}

DKVector3 DKBoxShape::HalfExtents(void) const
{
	btBoxShape* shape = static_cast<btBoxShape*>(this->impl);
	const btVector3& halfExt = shape->getHalfExtentsWithoutMargin();
	const btVector3& scale = shape->getLocalScaling();

	return BulletVector3(halfExt / scale);
}

DKVector3 DKBoxShape::HalfExtentsWithMargin(void) const
{
	btBoxShape* shape = static_cast<btBoxShape*>(this->impl);
	const btVector3& halfExt = shape->getHalfExtentsWithoutMargin();
	const btVector3& scale = shape->getLocalScaling();
	const btVector3 margin(shape->getMargin(), shape->getMargin(), shape->getMargin());

	return BulletVector3( halfExt / scale + margin );
}

DKVector3 DKBoxShape::ScaledHalfExtents(void) const
{
	return BulletVector3(static_cast<btBoxShape*>(this->impl)->getHalfExtentsWithoutMargin());
}

DKVector3 DKBoxShape::ScaledHalfExtentsWithMargin(void) const
{
	return BulletVector3(static_cast<btBoxShape*>(this->impl)->getHalfExtentsWithMargin());
}
