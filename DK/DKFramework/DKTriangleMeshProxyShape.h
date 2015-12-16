//
//  File: DKTriangleMeshProxyShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKGL_API DKTriangleMeshProxyShape : public DKConcaveShape
	{
	public:
		DKTriangleMeshProxyShape(void);
		virtual ~DKTriangleMeshProxyShape(void);

		// TriangleCallback : reports AABB-overlapped triangle.
		// parameter: (triangle, mesh-parts, triangle-index)
		using TriangleCallback = DKFoundation::DKFunctionSignature<void (DKTriangle&, int, int)>;

		// subclass should AABB-casting to all triangles and report overlaps.
		virtual void QueryTrianglesInAabb(const DKAabb&, TriangleCallback*) = 0;

		virtual DKAabb Aabb(void) const = 0;
	};
}
