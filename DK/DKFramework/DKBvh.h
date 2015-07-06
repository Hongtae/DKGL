//
//  File: DKBvh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKTriangle.h"
#include "DKTriangleMesh.h"
#include "DKLine.h"
#include "DKRect.h"
#include "DKAabb.h"

////////////////////////////////////////////////////////////////////////////////
// DKBvh
// implementation of BVH (Bounding volume hierarchy) to perform ray-test fast.
//
// NOT IMPLEMENTED YET.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKBvh
	{
	public:
		DKBvh(void);
		~DKBvh(void);

		void Build(DKTriangleMesh* mesh);
		void Rebuild(void);
		
		bool RayTest(const DKLine& ray, DKVector3* hitPoint = NULL) const;

	private:
		DKFoundation::DKObject<DKTriangleMesh> mesh;
		DKAabb aabb;
	};
}
