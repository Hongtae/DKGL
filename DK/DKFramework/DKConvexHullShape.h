//
//  File: DKConvexHullShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKPolyhedralConvexShape.h"

namespace DKFramework
{
	class DKTriangle;
	/// @brief
	/// convex hull collision shape.
	/// @details
	///  You can create convex hull by points or
	///  convex decomposition from triangle mesh.
	/// @note
	///  Use V-HACD for better quality. (https://github.com/kmammou/v-hacd)
	class DKGL_API DKConvexHullShape : public DKPolyhedralConvexShape
	{
	public:
		DKConvexHullShape(const DKVector3* vertices, size_t numVerts);
		~DKConvexHullShape(void);

		void AddPoint(const DKVector3& p);
		size_t NumberOfPoints(void) const;
		DKVector3 PointAtIndex(unsigned int index) const;
		DKVector3 ScaledPointAtIndex(unsigned int index) const;

		static DKObject<DKConvexHullShape> CreateHull(const DKTriangle* tri, size_t num);

	protected:
		DKConvexHullShape(ShapeType t, class btConvexHullShape* context);
	};
}
