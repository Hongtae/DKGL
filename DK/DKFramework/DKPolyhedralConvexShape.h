//
//  File: DKPolyhedralConvexShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConvexShape.h"
#include "DKPlane.h"
#include "DKLine.h"

namespace DKFramework
{
	/// @brief interface class for polyhedral convex shapes.
	class DKGL_API DKPolyhedralConvexShape : public DKConvexShape
	{
	public:
		~DKPolyhedralConvexShape(void);
		
		size_t NumberOfVertices(void) const;
		size_t NumberOfEdges(void) const;
		size_t NumberOfPlanes(void) const;

		DKLine EdgeAtIndex(unsigned int index) const;
		DKVector3 VertexAtIndex(unsigned int index) const;
		DKPlane PlaneAtIndex(unsigned int index) const;

		bool IsPointInside(const DKVector3& p, float tolerance) const;

	protected:
		DKPolyhedralConvexShape(ShapeType t, class btPolyhedralConvexShape* context);
	};
}
