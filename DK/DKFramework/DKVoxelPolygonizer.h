//
//  File: DKVoxelPolygonizer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2009-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector3.h"

////////////////////////////////////////////////////////////////////////////////
// DKVoxelPolygonizer
// abstract class, interface for polygonize from voxels.
// default polygonize algorithm based on 'Marching Cubes'.
// generating polygons from cube, you need to override how surface can be
// interpolated between each voxels.
// You also need to override function that can generate triangle and store
// vertex buffer with given positions. (calculated from this class)
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKVoxelPolygonizer
	{
	public:
		enum CubeIndex
		{
			CI_X0_Y0_Z0 = 0,	// rear-left-bottom   (0,0,0)
			CI_X1_Y0_Z0,		// rear-right-bottom  (1,0,0)
			CI_X1_Y0_Z1,		// front-right-bottom (1,0,1)
			CI_X0_Y0_Z1,		// front-left-bottom  (0,0,1)
			CI_X0_Y1_Z0,		// rear-left-top      (0,1,0)
			CI_X1_Y1_Z0,		// rear-right-top     (1,1,0)
			CI_X1_Y1_Z1,		// front-right-top    (1,1,1)
			CI_X0_Y1_Z1,		// front-left-top     (0,1,1)
		};
		enum CubicBitMask
		{
			CB_X0_Y0_Z0 = 1 << CI_X0_Y0_Z0,
			CB_X1_Y0_Z0 = 1 << CI_X1_Y0_Z0,
			CB_X1_Y0_Z1 = 1 << CI_X1_Y0_Z1,
			CB_X0_Y0_Z1 = 1 << CI_X0_Y0_Z1,
			CB_X0_Y1_Z0 = 1 << CI_X0_Y1_Z0,
			CB_X1_Y1_Z0 = 1 << CI_X1_Y1_Z0,
			CB_X1_Y1_Z1 = 1 << CI_X1_Y1_Z1,
			CB_X0_Y1_Z1 = 1 << CI_X0_Y1_Z1,
		};
		struct Vertex // polygon output
		{
			DKVector3 pos;   // value in range of 0.0 ~ 1.0.
			CubeIndex idx1;  // index of affected voxel-1
			CubeIndex idx2;  // index of affected voxel-2
		};

		DKVoxelPolygonizer(void);
		virtual ~DKVoxelPolygonizer(void);

		void PolygonizeSurface(int cubicBits);  // cubicBits: bitmask CubicBitMask combinations for each cube
		virtual void GenerateTriangle(Vertex&, Vertex&, Vertex&) = 0;	// polygonized result
		virtual DKVector3 Interpolate(const DKVector3& p1, const DKVector3& p2, CubeIndex c1, CubeIndex c2) = 0;
	};
}
