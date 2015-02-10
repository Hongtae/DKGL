//
//  File: DKStaticTriangleMeshShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKConcaveShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKStaticTriangleMeshShape
// collision shape for concave triangle mesh. it can only be used for fixed
// objects. If you want to use mesh to movable object, it is recommended to
// perform convex decomposition with DKConvexHullShape.
// (see DKConvexHullShape.h)
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKStaticTriangleMeshShape : public DKConcaveShape
	{
	public:
		DKStaticTriangleMeshShape(
			const DKVector3* vertices, size_t numVerts,
			const unsigned int* indices, size_t numIndices,
			const DKAABox& precalculatedAABB = DKAABox(),
			bool rebuildIndex = false, float weldingThreshold = 0);

		DKStaticTriangleMeshShape(
			const DKVector3* vertices, size_t numVerts,
			const unsigned short* indices, size_t numIndices,
			const DKAABox& precalculatedAABB = DKAABox(),
			bool rebuildIndex = false, float weldingThreshold = 0);

		~DKStaticTriangleMeshShape(void);

		// refit bounding volume hierarchy
		void RefitBVH(const DKAABox& aabb);
		void PartialRefitBVH(const DKAABox& aabb);

		DKVector3* VertexBuffer(size_t* numVerts);
		const DKVector3* VertexBuffer(size_t* numVerts) const;
		const void* IndexBuffer(size_t* numIndices, size_t* indexSize) const;
		DKAABox MeshAABB(void) const;

		size_t NumberOfTriangles(void) const;
		bool GetTriangleVertexIndices(int triangle, unsigned int* index) const;
		bool GetTriangleFace(int index, DKTriangle& triangle) const;
		bool SetTriangleFace(int index, const DKTriangle& triangle);

	private:
		class IndexedTriangleData;
		DKStaticTriangleMeshShape(IndexedTriangleData*);

		IndexedTriangleData* meshData;
	};
}
