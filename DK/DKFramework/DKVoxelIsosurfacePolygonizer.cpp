//
//  File: DKVoxelIsosurfacePolygonizer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVoxelIsosurfacePolygonizer.h"

using namespace DKFoundation;
using namespace DKFramework;

DKVoxelIsosurfacePolygonizer::DKVoxelIsosurfacePolygonizer(void)
	: levels(NULL)
	, isoLevel(FLT_MIN)
{
}

DKVoxelIsosurfacePolygonizer::~DKVoxelIsosurfacePolygonizer(void)
{
}

void DKVoxelIsosurfacePolygonizer::PolygonizeSurface(const Cube& cube)
{
	int cubeIndex = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (cube.levels[i] < this->isoLevel)
			cubeIndex |= (1 << i);
	}
	this->levels = &(cube.levels[0]);

	DKVoxelPolygonizer::PolygonizeSurface(cubeIndex);
}

DKVector3 DKVoxelIsosurfacePolygonizer::Interpolate(const DKVector3& p1, const DKVector3& p2, CubeIndex c1, CubeIndex c2)
{
	float v1 = levels[c1];
	float v2 = levels[c2];
	float m = (isoLevel - v1) / (v2 - v1);
	return p1 + (p2 - p1) * m;
}
