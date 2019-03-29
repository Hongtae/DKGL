//
//  File: DKStaticTriangleMeshShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConcaveShape.h"
#include "DKTriangle.h"

namespace DKFramework
{
	/// @brief Collision shape for concave triangle mesh.
	///
	/// It can only be used for fixed objects.
	/// If you want to use mesh to movable object, it is recommended to
	/// perform convex decomposition with DKConvexHullShape.
	/// (see DKConvexHullShape.h)
	/// If you need collision shape for dynamic triangle mesh,
	/// use DKTriangleMeshProxyShape class.
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

		~DKStaticTriangleMeshShape();


		size_t NumberOfVertices() const;
		size_t NumberOfIndices() const;
		size_t IndexSize() const;	///< Index size in bytes
		size_t NumberOfTriangles() const;

		const DKVector3& VertexAtIndex(int index) const;
		DKTriangle TriangleAtIndex(int index) const;

		DKAabb Aabb() const;

		const DKVector3* VertexData() const;
		const void* IndexData() const;

	private:
		class IndexedTriangleData;
		DKStaticTriangleMeshShape(IndexedTriangleData*);

		IndexedTriangleData* meshData;
	};
}
