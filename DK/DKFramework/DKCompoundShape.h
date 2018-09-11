//
//  File: DKCompoundShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCollisionShape.h"

namespace DKFramework
{
	/// @brief
	/// a compound shape with one or more collision shapes with transform.
	class DKGL_API DKCompoundShape : public DKCollisionShape
	{
	public:
		DKCompoundShape();
		~DKCompoundShape();

		size_t NumberOfShapes() const;
		void AddShape(DKCollisionShape* shape, const DKNSTransform& transform);
		void RemoveShapeAtIndex(long index);
		void RemoveShape(DKCollisionShape* shape);
		long IndexOfShape(DKCollisionShape* shape) const;

		DKCollisionShape* ShapeAtIndex(long index);
		const DKCollisionShape* ShapeAtIndex(long index) const;
		DKNSTransform TransformAtIndex(long index) const;
		void SetTransformAtIndex(const DKNSTransform& transform, long index);

	private:
		DKArray<DKObject<DKCollisionShape>> shapes;
	};
}
