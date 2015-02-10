//
//  File: DKTriangle.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKLine.h"
#include "DKSphere.h"
#include "DKCylinderShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKTriangle
// a triangle class for calculate collision
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKTriangle
	{
	public:
		enum class Front
		{
			CCW,		// counter clock wise is front. (front face order is p1,p2,p3)
			CW,			// clock wise is front. (front face order p3,p2,p1)
			Both,		// assume both faces are front.
		};

		DKTriangle(void);
		DKTriangle(const DKVector3& p1, const DKVector3& p2, const DKVector3& p3);
		~DKTriangle(void);

		DKVector3	position1;
		DKVector3	position2;
		DKVector3	position3;

		bool Intersect(const DKLine& line, Front front = Front::Both, DKVector3* p = NULL) const;
	};
}
