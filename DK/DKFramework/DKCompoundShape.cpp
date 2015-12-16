//
//  File: DKCompoundShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKCompoundShape.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;

DKCompoundShape::DKCompoundShape(void)
	: DKCollisionShape(ShapeType::Compound, new btCompoundShape())
{
	DKASSERT_DEBUG( this->impl->isCompound() );
}

DKCompoundShape::~DKCompoundShape(void)
{
	DKASSERT_DEBUG( this->impl->isCompound() );
	DKASSERT_DEBUG( dynamic_cast<btCompoundShape*>(this->impl) != NULL );

#ifdef DKGL_DEBUG_ENABLED	
	btCompoundShape* compund = static_cast<btCompoundShape*>(this->impl);
	DKASSERT( this->shapes.Count() == compund->getNumChildShapes() );
	for (size_t i = 0; i < this->shapes.Count(); ++i)
	{
		DKCollisionShape* shape1 = this->shapes.Value(i);
		btCollisionShape* shape2 = compund->getChildShape(i);

		DKASSERT( BulletCollisionShape(shape1) == shape2 );
		DKASSERT( shape2->getUserPointer() == shape1 );
	}
#endif
}

void DKCompoundShape::AddShape(DKCollisionShape* shape, const DKNSTransform& transform)
{
	if (shape)
	{
		static_cast<btCompoundShape*>(this->impl)->addChildShape(BulletTransform(transform), BulletCollisionShape(shape));
		this->shapes.Add(shape);
	}
}

void DKCompoundShape::RemoveShapeAtIndex(long index)
{
	if (index >= 0 && index < shapes.Count())
	{
		static_cast<btCompoundShape*>(this->impl)->removeChildShapeByIndex(index);
		shapes.Remove(index);
	}
}

void DKCompoundShape::RemoveShape(DKCollisionShape* shape)
{
	long index = IndexOfShape(shape);
	if (index >= 0)
		RemoveShapeAtIndex(index);
}

long DKCompoundShape::IndexOfShape(DKCollisionShape* shape) const
{
	for (size_t i = 0; i < shapes.Count(); ++i)
	{
		if (shapes.Value(i) == shape)
		{
			return (long)i;
		}
	}
	return -1;
}

size_t DKCompoundShape::NumberOfShapes(void) const
{
	DKASSERT_DEBUG(this->impl->isCompound());
	DKASSERT_DEBUG(static_cast<const btCompoundShape*>(this->impl)->getNumChildShapes() == this->shapes.Count());
	return shapes.Count();
}

DKCollisionShape* DKCompoundShape::ShapeAtIndex(long index)
{
	return const_cast<DKCollisionShape*>(static_cast<const DKCompoundShape&>(*this).ShapeAtIndex(index));
}

const DKCollisionShape* DKCompoundShape::ShapeAtIndex(long index) const
{
	DKASSERT_DEBUG(this->impl->isCompound());
	DKASSERT_DEBUG(static_cast<const btCompoundShape*>(this->impl)->getNumChildShapes() == this->shapes.Count());
	if (index >= 0 && index < this->shapes.Count())
		return this->shapes.Value(index);
	return NULL;
}

DKNSTransform DKCompoundShape::TransformAtIndex(long index) const
{
	DKASSERT_DEBUG(this->impl->isCompound());
	const btCompoundShape* compund = static_cast<const btCompoundShape*>(this->impl);
	DKASSERT_DEBUG(compund->getNumChildShapes() == this->shapes.Count());
	DKASSERT_DEBUG(index >= 0);
	DKASSERT_DEBUG(index < compund->getNumChildShapes());

	return BulletTransform(compund->getChildTransform(index));
}

void DKCompoundShape::SetTransformAtIndex(const DKNSTransform& transform, long index)
{
	DKASSERT_DEBUG(this->impl->isCompound());
	btCompoundShape* compund = static_cast<btCompoundShape*>(this->impl);
	DKASSERT_DEBUG(compund->getNumChildShapes() == this->shapes.Count());
	DKASSERT_DEBUG(index >= 0);
	DKASSERT_DEBUG(index < compund->getNumChildShapes());

	compund->updateChildTransform(index, BulletTransform(transform));
}
