//
//  File: DKCapsuleShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKCapsuleShape.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		inline btCapsuleShape* CreateCapsuleShape(float radius, float height, DKCapsuleShape::UpAxis up)
		{
			switch (up)
			{
			case DKCapsuleShape::UpAxis::Left:		return new btCapsuleShapeX(radius, height); break;
			case DKCapsuleShape::UpAxis::Forward:	return new btCapsuleShapeZ(radius, height); break;
			}
			return new btCapsuleShape(radius, height);
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKCapsuleShape::DKCapsuleShape(float radius, float height, UpAxis up)
	: DKConvexShape(ShapeType::Capsule, CreateCapsuleShape(radius, height, up))
{
	switch (up)
	{
	case UpAxis::Left:
	case UpAxis::Top:
	case UpAxis::Forward:
		break;
	default:
		DKERROR_THROW_DEBUG("Invalid axis");
		break;
	}
}

DKCapsuleShape::~DKCapsuleShape(void)
{
}

float DKCapsuleShape::Radius(void) const
{
	btCapsuleShape* shape = static_cast<btCapsuleShape*>(this->impl);

	int axis = (shape->getUpAxis() + 2) % 3;		// radius-axis
	const btVector3& scale = shape->getLocalScaling();
	float r = shape->getRadius();

	return r / scale[axis];
}

float DKCapsuleShape::HalfHeight(void) const
{
	btCapsuleShape* shape = static_cast<btCapsuleShape*>(this->impl);

	int up = shape->getUpAxis();
	const btVector3& scale = shape->getLocalScaling();
	float h = shape->getHalfHeight();

	return (h / scale[up]);
}

float DKCapsuleShape::ScaledRadius(void) const
{
	return static_cast<btCapsuleShape*>(this->impl)->getRadius();
}

float DKCapsuleShape::ScaledHalfHeight(void) const
{
	return static_cast<btCapsuleShape*>(this->impl)->getHalfHeight();
}

DKCapsuleShape::UpAxis DKCapsuleShape::BaseAxis(void) const
{
	int axis = static_cast<btCapsuleShape*>(this->impl)->getUpAxis();
	switch (axis)
	{
	case 0:	return UpAxis::Left;
	case 1: return UpAxis::Top;
	case 2: return UpAxis::Forward;
	default:
		DKERROR_THROW_DEBUG("Invalid axis");
		break;
	}
	return UpAxis::Top;
}
