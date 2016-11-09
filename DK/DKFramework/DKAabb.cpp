//
//  File: DKAabb.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKAabb.h"
#include "DKLine.h"
#include "DKBox.h"

using namespace DKFramework;

DKAabb::DKAabb(void)
	: positionMin(DKVector3(FLT_MAX, FLT_MAX, FLT_MAX))
	, positionMax(DKVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX))
{
}

DKAabb::DKAabb(const DKVector3& posMin, const DKVector3& posMax)
	: positionMin(posMin)
	, positionMax(posMax)
{
}

void DKAabb::Expand(const DKVector3& pos)
{
	if (pos.x < positionMin.x) positionMin.x = pos.x;
	if (pos.y < positionMin.y) positionMin.y = pos.y;
	if (pos.z < positionMin.z) positionMin.z = pos.z;
	if (pos.x > positionMax.x) positionMax.x = pos.x;
	if (pos.y > positionMax.y) positionMax.y = pos.y;
	if (pos.z > positionMax.z) positionMax.z = pos.z;
}

DKAabb DKAabb::Intersection(const DKAabb& b1, const DKAabb& b2)
{
	if (b1.IsValid() && b2.IsValid())
	{
		return DKAabb(
			DKVector3(Max(b1.positionMin.x, b2.positionMin.x), Max(b1.positionMin.y, b2.positionMin.y), Max(b1.positionMin.z, b2.positionMin.z)),
			DKVector3(Min(b1.positionMax.x, b2.positionMax.x), Min(b1.positionMax.y, b2.positionMax.y), Min(b1.positionMax.z, b2.positionMax.z))
			);
	}
	return DKAabb();
}

DKAabb DKAabb::Union(const DKAabb& b1, const DKAabb& b2)
{
	if (b1.IsValid() && b2.IsValid())
	{
		return DKAabb(
			DKVector3(Min(b1.positionMin.x, b2.positionMin.x), Min(b1.positionMin.y, b2.positionMin.y), Min(b1.positionMin.z, b2.positionMin.z)),
			DKVector3(Max(b1.positionMax.x, b2.positionMax.x), Max(b1.positionMax.y, b2.positionMax.y), Max(b1.positionMax.z, b2.positionMax.z))
			);
	}
	else if (b1.IsValid())
		return b1;
	else if (b2.IsValid())
		return b2;

	return DKAabb();
}

bool DKAabb::IsValid(void) const
{
	return positionMax.x >= positionMin.x && positionMax.y >= positionMin.y && positionMax.z >= positionMin.z;
}

bool DKAabb::IsPointInside(const DKVector3& pos) const
{
	if (pos.x >= positionMin.x && pos.x <= positionMax.x &&
		pos.y >= positionMin.y && pos.y <= positionMax.y &&
		pos.z >= positionMin.z && pos.z <= positionMax.z)
		return true;
	return false;
}

DKVector3 DKAabb::Center(void) const
{
	return (positionMin + positionMax) * 0.5f;
}

float DKAabb::Volume(void) const
{
	DKVector3 v = positionMax - positionMin;
	return v.x * v.y * v.z;
}

DKBox DKAabb::Box(void) const
{
	return DKBox(this->Center(), DKVector3(this->positionMax.x, 0, 0), DKVector3(0, this->positionMax.y, 0), DKVector3(0, 0, this->positionMax.z));
}

bool DKAabb::RayTest(const DKLine& ray, DKVector3* p) const
{
	if (p)
	{
		// implemented based on: http://www.codercorner.com/RayAABB.cpp
		// Original code by Andrew Woo, from "Graphics Gems", Academic Press, 1990

		const DKVector3 dir = ray.Direction();
		bool inside = true;
		DKVector3 maxT(-1,-1,-1);

		DKVector3 coord(0,0,0);

		for (int i = 0; i < 3; ++i)
		{
			if (ray.begin.val[i] < this->positionMin.val[i])
			{
				coord.val[i] = this->positionMin.val[i];
				inside = false;
				if ((unsigned int&)dir.val[i])
					maxT.val[i] = (this->positionMin.val[i] - ray.begin.val[i]) / dir.val[i];
			}
			else if (ray.begin.val[i] > this->positionMax.val[i])
			{
				coord.val[i] = this->positionMax.val[i];
				inside = false;
				if ((unsigned int&)dir.val[i])
					maxT.val[i] = (this->positionMax.val[i] - ray.begin.val[i]) / dir.val[i];
			}
		}

		if (inside)
		{
			*p = ray.begin;
			return true;
		}
		// calculate maxT to find intersection point.
		unsigned int plane = 0;
		if (maxT.y > maxT.val[plane])	plane = 1;		// plane of axis Y
		if (maxT.z > maxT.val[plane])	plane = 2;		// plane of axis Z

		//	if (maxT.val[plane] < 0)
		//		return false;
		if (((unsigned int&)maxT.val[plane]) & 0x80000000)	// negative
			return false;

		for (int i = 0; i < 3; ++i)
		{
			if (i != plane)
			{
				coord.val[i] = ray.begin.val[i] + maxT.val[plane] * dir.val[i];

				//if(coord.val[i] < this->positionMin.val[i] - Private::epsilon || coord.val[i] > this->positionMax.val[i] + Private::epsilon)
				//	return false;
				if (coord.val[i] < this->positionMin.val[i] || coord.val[i] > this->positionMax.val[i])
					return false;
			}
		}

		// check length.
		if (ray.LengthSq() >= (coord - ray.begin).LengthSq())
		{
			*p = coord;
			return true;
		}
	}
	else
	{
		// check intersect by x,y,z planes with pos.x,y,z
		DKVector3 lineMax = DKVector3(Max(ray.begin.x, ray.end.x), Max(ray.begin.y, ray.end.y), Max(ray.begin.z, ray.end.z));
		DKVector3 lineMin = DKVector3(Min(ray.begin.x, ray.end.x), Min(ray.begin.y, ray.end.y), Min(ray.begin.z, ray.end.z));

		if (this->positionMax.x >= lineMin.x && this->positionMin.x <= lineMax.x &&
			this->positionMax.y >= lineMin.y && this->positionMin.y <= lineMax.y &&
			this->positionMax.z >= lineMin.z && this->positionMin.z <= lineMax.z)
			return true;
	}
	return false;
}

bool DKAabb::Intersect(const DKAabb& box) const
{
	if (this->positionMin.x > box.positionMax.x || this->positionMin.y > box.positionMax.y || this->positionMin.z > box.positionMax.z ||
		this->positionMax.x < box.positionMin.x || this->positionMax.y < box.positionMin.y || this->positionMax.z < box.positionMin.z)
		return false;
	return true;
}
