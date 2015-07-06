//
//  File: DKTriangleMesh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKTriangle.h"

////////////////////////////////////////////////////////////////////////////////
// DKTriangleMesh
// an abstract class, defines triangle-mesh.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	struct DKTriangleMesh
	{
		virtual ~DKTriangleMesh(void) {}

		virtual size_t NumberOfTriangles(void) const;
		virtual DKTriangle TriangleAtIndex(unsigned long index) const;
	};
}
