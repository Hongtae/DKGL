//
//  File: DKTriangleMeshProxyShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConcaveShape.h"
#include "DKTriangleMesh.h"

namespace DKFramework
{
	/// @brief A proxy shape of dynamic triangle mesh.
	/// @note
	/// This class is not serializable. An instance should be created on rum-time.
	class DKGL_API DKTriangleMeshProxyShape : public DKConcaveShape
	{
	public:
		DKTriangleMeshProxyShape();
		virtual ~DKTriangleMeshProxyShape();

		/// callback function, reports AABB-overlapped triangle.
		/// parameters: (triangle, mesh-parts, triangle-index)
		using TriangleCallback = DKFunctionSignature<void (DKTriangle&, int, int)>;

		/// subclass should AABB-casting to all triangles and report overlaps.
		virtual void QueryTrianglesInAabb(const DKAabb&, TriangleCallback*) = 0;

		virtual DKAabb Aabb() const = 0;
	};
}
