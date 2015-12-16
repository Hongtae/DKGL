//
//  File: DKStaticPlaneShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKStaticPlaneShape.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;

DKStaticPlaneShape::DKStaticPlaneShape(const DKVector3& planeNormal, float planeConstant)
	: DKConcaveShape(ShapeType::StaticPlane, new btStaticPlaneShape(btVector3(planeNormal.x, planeNormal.y, planeNormal.z), planeConstant))
{
}

DKStaticPlaneShape::~DKStaticPlaneShape(void)
{
}

DKVector3 DKStaticPlaneShape::PlaneNormal(void) const
{
	return BulletVector3(static_cast<btStaticPlaneShape*>(this->impl)->getPlaneNormal());
}

float DKStaticPlaneShape::PlaneConstant(void) const
{
	return static_cast<btStaticPlaneShape*>(this->impl)->getPlaneConstant();
}
