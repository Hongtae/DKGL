//
//  File: DKConeShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKConeShape.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		inline btConeShape* CreateConeShape(float radius, float height, DKConeShape::UpAxis up)
		{
			switch (up)
			{
			case DKConeShape::UpAxis::Left:		return new btConeShapeX(radius, height); break;
			case DKConeShape::UpAxis::Forward:	return new btConeShapeZ(radius, height); break;
			}
			return new btConeShape(radius, height);
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKConeShape::DKConeShape(float radius, float height, UpAxis up)
	: DKConvexShape(ShapeType::Cone, CreateConeShape(radius, height, up))
{
}

DKConeShape::~DKConeShape(void)
{
}

float DKConeShape::Radius(void) const
{
	btConeShape* shape = static_cast<btConeShape*>(this->impl);

	int up = shape->getConeUpIndex();

	float r = shape->getRadius();
	const btVector3& scale = shape->getLocalScaling();

	float s = (scale[0] + scale[1] + scale[2] - scale[up]) * 0.5f;

	return r / s;
}

float DKConeShape::Height(void) const
{
	btConeShape* shape = static_cast<btConeShape*>(this->impl);

	int up = shape->getConeUpIndex();

	float r = shape->getHeight();
	const btVector3& scale = shape->getLocalScaling();

	return r / scale[up];
}

float DKConeShape::ScaledRadius(void) const
{
	return static_cast<btConeShape*>(this->impl)->getRadius();
}

float DKConeShape::ScaledHeight(void) const
{
	return static_cast<btConeShape*>(this->impl)->getHeight();
}

DKConeShape::UpAxis DKConeShape::BaseAxis(void) const
{
	int axis = static_cast<btConeShape*>(this->impl)->getConeUpIndex();
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
