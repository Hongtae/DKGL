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

		virtual int NumberOfTriangles(void) const = 0;
		virtual bool GetTriangleAtIndex(int index, DKTriangle&) const = 0;

		virtual void Lock(void) {}
		virtual void Unlock(void) {}
	};
}
