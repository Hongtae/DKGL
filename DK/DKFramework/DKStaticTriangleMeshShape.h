//
//  File: DKStaticTriangleMeshShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKConcaveShape.h"
#include "DKTriangle.h"

////////////////////////////////////////////////////////////////////////////////
// DKStaticTriangleMeshShape
// Collision shape for concave triangle mesh. it can only be used for fixed
// objects. If you want to use mesh to movable object, it is recommended to
// perform convex decomposition with DKConvexHullShape.
// (see DKConvexHullShape.h)
// If you need collision shape for dynamic triangle mesh,
// use DKTriangleMeshProxyShape class.
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKStaticTriangleMeshShape : public DKConcaveShape
	{
	public:
		DKStaticTriangleMeshShape(const DKVector3* vertices,
								  size_t numVertices,
								  const unsigned int* indices,
								  size_t numIndices,
								  const DKAabb& precalculatedAabb = DKAabb());
		DKStaticTriangleMeshShape(const DKVector3* vertices,
								  size_t numVertices,
								  const unsigned short* indices,
								  size_t numIndices,
								  const DKAabb& precalculatedAabb = DKAabb());

		~DKStaticTriangleMeshShape(void);


		size_t NumberOfVertices(void) const;
		size_t NumberOfIndices(void) const;
		size_t IndexSize(void) const;	// In Byte
		size_t NumberOfTriangles(void) const;

		const DKVector3& VertexAtIndex(int index) const;
		DKTriangle TriangleAtIndex(int index) const;

		DKAabb Aabb(void) const;

		const DKVector3* VertexData(void) const;
		const void* IndexData(void) const;

	private:
		class IndexedTriangleData;
		DKStaticTriangleMeshShape(IndexedTriangleData*);

		IndexedTriangleData* meshData;
	};
}
