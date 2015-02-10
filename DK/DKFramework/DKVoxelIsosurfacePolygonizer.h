//
//  File: DKVoxelIsosurfacePolygonizer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2009-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVoxelPolygonizer.h"

////////////////////////////////////////////////////////////////////////////////
// DKVoxelIsosurfacePolygonizer
// voxel polygonizer class, using 'Marching Cubes' algorithm.
// generating polygon surfaces with interpolate between each voxel's
// isosurface level.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKVoxelIsosurfacePolygonizer : public DKVoxelPolygonizer
	{
	public:
		struct Cube
		{
			float levels[8]; // surface level from 8 voxels (cubic)
		};
		DKVoxelIsosurfacePolygonizer(void);
		virtual ~DKVoxelIsosurfacePolygonizer(void);

		float isoLevel;		// iso-surface level
		void PolygonizeSurface(const Cube& cube);
	protected:
		DKVector3 Interpolate(const DKVector3& p1, const DKVector3& p2, CubeIndex c1, CubeIndex c2);
		const float* levels;
	};
}
