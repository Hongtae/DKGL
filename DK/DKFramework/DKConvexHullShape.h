//
//  File: DKConvexHullShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKPolyhedralConvexShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKConvexHullShape
//  convex hull collision shape.
//  You can create convex hull by points or
//  convex decomposition from triangle mesh.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKTriangle;
	class DKLIB_API DKConvexHullShape : public DKPolyhedralConvexShape
	{
	public:
		DKConvexHullShape(const DKVector3* vertices, size_t numVerts);
		~DKConvexHullShape(void);

		void AddPoint(const DKVector3& p);
		size_t NumberOfPoints(void) const;
		DKVector3 PointAtIndex(unsigned int index) const;
		DKVector3 ScaledPointAtIndex(unsigned int index) const;

		// create convex-hull using HACD convex decomposition library.
		// more info: https://code.google.com/p/v-hacd/
		static DKFoundation::DKObject<DKConvexHullShape> CreateHull(const DKTriangle* tri, size_t num);

		struct ConvexHull
		{
			DKFoundation::DKObject<DKConvexHullShape> shape;
			DKNSTransform offset;
		};
		typedef DKFoundation::DKArray<ConvexHull> ConvexHullArray;

		static ConvexHullArray DecomposeTriangleMesh(
			const DKVector3* verts,
			size_t numVerts,
			const long* indices,				// triangle indices
			size_t numIndices,					// number of indices ( number of triangles * 3 )
			size_t minClusters = 2,				// minimum number of clusters
			size_t maxVertsPerCH = 100,			// max vertices per convex-hull
			double maxConcavity = 100,			// maximum concavity
			bool addExtraDistPoints = false,
			bool addNeighboursDistPoints = false,
			bool addFacesPoints = false);

	protected:
		DKConvexHullShape(ShapeType t, class btConvexHullShape* context);
	};
}
