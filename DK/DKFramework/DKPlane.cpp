//
//  File: DKPlane.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKPlane.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKLine.h"

using namespace DKFoundation;
using namespace DKFramework;

DKPlane::DKPlane(void)
	: a(0)
	, b(0)
	, c(0)
	, d(0)
{
}

DKPlane::DKPlane(const DKVector3& v1, const DKVector3& v2, const DKVector3& v3)
{
	// calculate plane with v1 and normal
	const DKVector3 n = DKVector3::Cross(v2-v1, v3-v1).Normalize();
	a = n.x;
	b = n.y;
	c = n.z;
	d = -DKVector3::Dot(n, v1);
}

DKPlane::DKPlane(const DKVector3& n, const DKVector3& p)
	: a(n.x)
	, b(n.y)
	, c(n.z)
	, d(-DKVector3::Dot(n, p))
{
}

DKPlane::~DKPlane(void)
{
}

float DKPlane::Dot(const DKVector3& v) const
{
	return a*v.x + b*v.y + c*v.z + d;
}

float DKPlane::Dot(const DKVector4& v) const
{
	return a*v.x + b*v.y + c*v.z + d*v.w;
}

DKVector3 DKPlane::Normal(void) const
{
	return DKVector3(a,b,c);
}

bool DKPlane::Intersect(const DKLine& line, DKVector3* p) const
{
	float len1 = this->Dot(line.begin);
	if (len1 == 0) // line segment's begin is connected to the plane.
	{
		if (p)		*p = line.begin;
		return true;
	}

	float len2 = this->Dot(line.end);
	if (len2 == 0) // line segment's end is connected to the plane.
	{
		if (p)		*p = line.end;
		return true;
	}

	if ((len1 > 0 && len2 < 0) || (len1 < 0 && len2 > 0)) // line segment passing through
	{
		if (p)
		{
			const DKVector3 dir = line.end - line.begin; // line's direction
			const DKVector3 n = this->Normal();			 // plane's direction

			float t = -len1 / DKVector3::Dot(this->Normal(), dir);

			*p = line.begin + line.Direction() * t;
		}
		return true;
	}
	return false;
}
