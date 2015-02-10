//
//  File: DKCompoundShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKCollisionShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKCompoundShape
// a compound shape with one or more collision shapes with transform.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKCompoundShape : public DKCollisionShape
	{
	public:
		DKCompoundShape(void);
		~DKCompoundShape(void);

		size_t NumberOfShapes(void) const;
		void AddShape(DKCollisionShape* shape, const DKNSTransform& transform);
		void RemoveShapeAtIndex(long index);
		void RemoveShape(DKCollisionShape* shape);
		long IndexOfShape(DKCollisionShape* shape) const;

		DKCollisionShape* ShapeAtIndex(long index);
		const DKCollisionShape* ShapeAtIndex(long index) const;
		DKNSTransform TransformAtIndex(long index) const;
		void SetTransformAtIndex(const DKNSTransform& transform, long index);

	private:
		DKFoundation::DKArray<DKFoundation::DKObject<DKCollisionShape>> shapes;
	};
}
