//
//  File: DKTriangle.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKLine.h"
#include "DKSphere.h"
#include "DKCylinderShape.h"
#include "DKAabb.h"

#pragma pack(push, 4)
namespace DKFramework
{
	/// @brief a triangle class for calculate collision
	class DKGL_API DKTriangle
	{
	public:
		enum class Front
		{
			CCW,	///< counter clock wise is front. (front face order is p1,p2,p3)
			CW,		///< clock wise is front. (front face order p3,p2,p1)
			Both,	///< assume both faces are front.
		};

		DKTriangle(void);
		DKTriangle(const DKVector3& p1, const DKVector3& p2, const DKVector3& p3);
		~DKTriangle(void);

		DKVector3	position1;
		DKVector3	position2;
		DKVector3	position3;

		float Area(void) const;		///< area of triangle
		DKAabb Aabb(void) const;
		bool RayTest(const DKLine& ray, DKVector3* hitPoint = NULL, Front faces = Front::Both, float epsilon = 0.000001f) const;
	};
}
#pragma pack(pop)
