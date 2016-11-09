//
//  File: DKTriangleMeshBvh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKFoundation.h"
#include "DKBvh.h"
#include "DKLine.h"
#include "DKAabb.h"
#include "DKVector3.h"
#include "DKTriangleMesh.h"
#include "DKTriangle.h"

////////////////////////////////////////////////////////////////////////////////
// DKTriangleMeshBvh
// a triangle mesh class, using BVH tree internally
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKGL_API DKTriangleMeshBvh
	{
	public:
		DKTriangleMeshBvh(void);
		~DKTriangleMeshBvh(void);
		
		void Build(DKTriangleMesh* mesh);
		void Rebuild(void);

		DKAabb Aabb(void) const;
		bool RayTest(const DKLine& ray, DKVector3* hitPoint = NULL) const;

		const DKBvh& Bvh(void) const { return bvh; }

	private:
		DKObject<DKTriangleMesh> mesh;
		DKBvh bvh;
	};
}
