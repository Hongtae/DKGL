//
//  File: DKTriangleMesh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.


#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKTriangle.h"

namespace DKFramework
{
	/// @brief triangle mesh interface
	struct DKTriangleMesh
	{
		virtual ~DKTriangleMesh(void) {}

		virtual int NumberOfTriangles(void) const = 0;
		virtual bool GetTriangleAtIndex(int index, DKTriangle&) const = 0;

		virtual void Lock(void) {}
		virtual void Unlock(void) {}
	};
}
