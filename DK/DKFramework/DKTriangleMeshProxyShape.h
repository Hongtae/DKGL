//
//  File: DKTriangleMeshProxyShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKConcaveShape.h"
#include "DKTriangleMesh.h"

////////////////////////////////////////////////////////////////////////////////
// DKTriangleMeshProxyShape
// This class is proxy shape of dynamic triangle mesh.
// This class is not serializable. An instance can be created on Rumtime only.
//
// Vertex position data should be float[3] (DKVector3)
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKGL_API DKTriangleMeshProxyShape : public DKConcaveShape
	{
	public:
		DKTriangleMeshProxyShape(void);
		virtual ~DKTriangleMeshProxyShape(void);

		void Build(void);
		void Refit(const DKAabb&);
		void PartialRefit(const DKAabb&);
		DKAabb Aabb(void) const;

		struct MeshInfo
		{
			size_t numVertices;
			size_t numTriangles;
			size_t vertexStride;	// sizeof vertex (stride of position vector)
			size_t indexStride;		// stride of triangle indices
			size_t indexSize;		// 2 for USHORT, 4 for UINT
			const void* vertexBuffer;	// address of first position vector
			const void* indexBuffer;	// address of first triangle index
		};

		virtual int NumberOfMeshParts(void) const = 0;

	protected:
		virtual void LockMeshPart(int, MeshInfo&) const = 0;
		virtual void UnlockMeshPart(int) const = 0;

	private:
		class MeshInterface;
		MeshInterface* meshInterface;
		DKTriangleMeshProxyShape(MeshInterface*);
	};
}
