//
//  File: DKCylinderShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletPhysics.h"
#include "DKCylinderShape.h"

namespace DKFramework
{
	namespace Private
	{
		inline btCylinderShape* CreateCylinderShape(const btVector3& halfExtents, DKCylinderShape::UpAxis up)
		{
			switch (up)
			{
			case DKCylinderShape::UpAxis::Left:		return new btCylinderShapeX(halfExtents); break;
			case DKCylinderShape::UpAxis::Forward:	return new btCylinderShapeZ(halfExtents); break;
			}
			return new btCylinderShape(halfExtents);
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKCylinderShape::DKCylinderShape(float u, float v, float w, UpAxis up)
	: DKConvexShape(ShapeType::Cylinder, CreateCylinderShape(btVector3(u,v,w), up))
{
}

DKCylinderShape::DKCylinderShape(const DKVector3& v, UpAxis up)
	: DKConvexShape(ShapeType::Cylinder, CreateCylinderShape(BulletVector3(v), up))
{
}

DKCylinderShape::~DKCylinderShape()
{
}

DKVector3 DKCylinderShape::HalfExtents() const
{
	btCylinderShape* shape = static_cast<btCylinderShape*>(this->impl);
	const btVector3& halfExt = shape->getHalfExtentsWithoutMargin();
	const btVector3& scale = shape->getLocalScaling();

	return BulletVector3(halfExt / scale);
}

DKVector3 DKCylinderShape::HalfExtentsWithMargin() const
{
	btCylinderShape* shape = static_cast<btCylinderShape*>(this->impl);
	const btVector3& halfExt = shape->getHalfExtentsWithoutMargin();
	const btVector3& scale = shape->getLocalScaling();
	const btVector3 margin(shape->getMargin(), shape->getMargin(), shape->getMargin());

	return BulletVector3( halfExt / scale + margin );
}

DKVector3 DKCylinderShape::ScaledHalfExtents() const
{
	return BulletVector3(static_cast<btCylinderShape*>(this->impl)->getHalfExtentsWithoutMargin());
}

DKVector3 DKCylinderShape::ScaledHalfExtentsWithMargin() const
{
	return BulletVector3(static_cast<btCylinderShape*>(this->impl)->getHalfExtentsWithMargin());
}

DKCylinderShape::UpAxis DKCylinderShape::BaseAxis() const
{
	int axis = static_cast<btCylinderShape*>(this->impl)->getUpAxis();
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
