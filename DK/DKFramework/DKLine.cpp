//
//  File: DKLine.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKLine.h"

using namespace DKFramework;

DKLine::DKLine(void)
	: begin(DKVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX))
	, end(DKVector3(FLT_MAX, FLT_MAX, FLT_MAX))
{
}

DKLine::DKLine(const DKVector3& b, const DKVector3& e)
	: begin(b)
	, end(e)
{
}

DKLine DKLine::operator * (const DKMatrix4& m) const
{
	return DKLine(begin * m, end * m);
}

DKLine DKLine::operator * (const DKQuaternion& q) const
{
	return DKLine(begin * q, end * q);
}

DKVector3 DKLine::Direction(void) const
{
	return (end - begin).Normalize();
}

float DKLine::Length(void) const
{
	return (end - begin).Length();
}

float DKLine::LengthSq(void) const
{
	return (end - begin).LengthSq();
}

bool DKLine::IsPointInside(const DKVector3& pos) const
{
	if (pos == begin)
		return true;
	if (pos == end)
		return true;

	float len = Length();
	if ((pos - begin).Length() > len)
		return false;

	const DKVector3 dir1 = Direction();
	const DKVector3 dir2 = (pos - begin).Normalize();

	return DKVector3::Dot(dir1, dir2) == 1.0;
}

bool DKLine::RayTest(const DKLine& ray, DKVector3* p, float epsilon) const
{
//	const float epsilon = 0.000001f;

	DKLine s = DKLine::LineBetween(*this, ray); // shortest line between 'this' and 'line'
	if (s.Length() < epsilon)
	{
		if (p)	*p = s.begin;
		return true;
	}
	return false;
}

DKLine DKLine::LineBetween(const DKLine& line1, const DKLine& line2, float epsilon)
{
	// compute shortest line between two lines, algorithm based on
	// http://softsurfer.com/Archive/algorithm_0106/algorithm_0106.htm#dist3D_Segment_to_Segment

	//const float epsilon = 0.000001f;

	const DKVector3 u = line1.end - line1.begin;
	const DKVector3 v = line2.end - line2.begin;
	const DKVector3 w = line1.begin - line2.begin;

	float a = DKVector3::Dot(u,u);	// always >= 0
	float b = DKVector3::Dot(u,v);
	float c = DKVector3::Dot(v,v);	// always >= 0
	float d = DKVector3::Dot(u,w);
	float e = DKVector3::Dot(v,w);

	float D = a*c - b*b;       // always >= 0
	float sc, sN, sD = D;      // sc = sN / sD, default sD = D >= 0
	float tc, tN, tD = D;      // tc = tN / tD, default tD = D >= 0

	// compute the line parameters of the two closest points
	if (D < epsilon)	// the lines are almost parallel
	{
		sN = 0.0;		// force using point P0 on segment S1
		sD = 1.0;		// to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else			// get the closest points on the infinite lines
	{
		sN = (b*e - c*d);
		tN = (a*e - b*d);
		if (sN < 0.0)		// sc < 0 => the s=0 edge is visible
		{
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if (sN > sD)	// sc > 1 => the s=1 edge is visible
		{
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0)		// tc < 0 => the t=0 edge is visible
	{
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else if (-d > a)
			sN = sD;
		else {
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD)	// tc > 1 => the t=1 edge is visible
	{
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else if ((-d + b) > a)
			sN = sD;
		else
		{
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (fabs(sN) < epsilon ? 0.0 : sN / sD);
	tc = (fabs(tN) < epsilon ? 0.0 : tN / tD);

	return DKLine(line1.begin + (u * sc), line2.begin + (v * tc));
}
