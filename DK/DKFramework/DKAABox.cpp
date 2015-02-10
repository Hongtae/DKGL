//
//  File: DKAABox.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKAABox.h"
#include "DKLine.h"
#include "DKBox.h"

using namespace DKFoundation;
using namespace DKFramework;

DKAABox::DKAABox(void)
	: positionMin(DKVector3(FLT_MAX, FLT_MAX, FLT_MAX))
	, positionMax(DKVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX))
{
}

DKAABox::DKAABox(const DKVector3& posMin, const DKVector3& posMax)
	: positionMin(posMin)
	, positionMax(posMax)
{
}

DKAABox DKAABox::Intersection(const DKAABox& b1, const DKAABox& b2)
{
	if (b1.IsValid() && b2.IsValid())
	{
		return DKAABox(
			DKVector3(Max(b1.positionMin.x, b2.positionMin.x), Max(b1.positionMin.y, b2.positionMin.y), Max(b1.positionMin.z, b2.positionMin.z)),
			DKVector3(Min(b1.positionMax.x, b2.positionMax.x), Min(b1.positionMax.y, b2.positionMax.y), Min(b1.positionMax.z, b2.positionMax.z))
			);
	}
	return DKAABox();
}

DKAABox DKAABox::Union(const DKAABox& b1, const DKAABox& b2)
{
	if (b1.IsValid() && b2.IsValid())
	{
		return DKAABox(
			DKVector3(Min(b1.positionMin.x, b2.positionMin.x), Min(b1.positionMin.y, b2.positionMin.y), Min(b1.positionMin.z, b2.positionMin.z)),
			DKVector3(Max(b1.positionMax.x, b2.positionMax.x), Max(b1.positionMax.y, b2.positionMax.y), Max(b1.positionMax.z, b2.positionMax.z))
			);
	}
	else if (b1.IsValid())
		return b1;
	else if (b2.IsValid())
		return b2;

	return DKAABox();
}

bool DKAABox::IsValid(void) const
{
	return positionMax.x >= positionMin.x && positionMax.y >= positionMin.y && positionMax.z >= positionMin.z;
}

bool DKAABox::IsPointInside(const DKVector3& pos) const
{
	if (pos.x >= positionMin.x && pos.x <= positionMax.x &&
		pos.y >= positionMin.y && pos.y <= positionMax.y &&
		pos.z >= positionMin.z && pos.z <= positionMax.z)
		return true;
	return false;
}

DKVector3 DKAABox::Center(void) const
{
	return (positionMin + positionMax) * 0.5f;
}

float DKAABox::Volume(void) const
{
	DKVector3 v = positionMax - positionMin;
	return v.x * v.y * v.z;
}

DKBox DKAABox::Box(void) const
{
	return DKBox(this->Center(), DKVector3(this->positionMax.x, 0, 0), DKVector3(0, this->positionMax.y, 0), DKVector3(0, 0, this->positionMax.z));
}

bool DKAABox::Intersect(const DKLine& line, DKVector3* p) const
{
	if (p)
	{
		// implemented based on: http://www.codercorner.com/RayAABB.cpp

		const DKVector3 dir = line.Direction();
		bool inside = true;
		DKVector3 maxT(-1,-1,-1);

		DKVector3 coord(0,0,0);

		for (int i = 0; i < 3; ++i)
		{
			if (line.begin.val[i] < this->positionMin.val[i])
			{
				coord.val[i] = this->positionMin.val[i];
				inside = false;
				if ((unsigned int&)dir.x)	maxT.val[i] = (this->positionMin.val[i] - line.begin.val[i]) / dir.val[i];
			}
			else
			{
				coord.val[i] = this->positionMax.val[i];
				inside = false;
				if ((unsigned int&)dir.y)	maxT.val[i] = (this->positionMax.val[i] - line.begin.val[i]) / dir.val[i];
			}
		}

		if (inside)
		{
			*p = line.begin;
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
				coord.val[i] = line.begin.val[i] + maxT.val[plane] * dir.val[i];

				//if(coord.val[i] < this->positionMin.val[i] - Private::epsilon || coord.val[i] > this->positionMax.val[i] + Private::epsilon)
				//	return false;
				if (coord.val[i] < this->positionMin.val[i] || coord.val[i] > this->positionMax.val[i])
					return false;
			}
		}

		*p = coord;
		return true;
	}
	else
	{
		// check intersect by x,y,z planes with pos.x,y,z
		DKVector3 lineMax = DKVector3(Max(line.begin.x, line.end.x), Max(line.begin.y, line.end.y), Max(line.begin.z, line.end.z));
		DKVector3 lineMin = DKVector3(Min(line.begin.x, line.end.x), Min(line.begin.y, line.end.y), Min(line.begin.z, line.end.z));

		if (this->positionMax.x >= lineMin.x && this->positionMin.x <= lineMax.x &&
			this->positionMax.y >= lineMin.y && this->positionMin.y <= lineMax.y &&
			this->positionMax.z >= lineMin.z && this->positionMin.z <= lineMax.z)
			return true;
	}
	return false;
}

bool DKAABox::Intersect(const DKAABox& box) const
{
	if (this->positionMin.x > box.positionMax.x || this->positionMin.y > box.positionMax.y || this->positionMin.z > box.positionMax.z ||
		this->positionMax.x < box.positionMin.x || this->positionMax.y < box.positionMin.y || this->positionMax.z < box.positionMin.z)
		return false;
	return true;

}
