//
//  File: DKSphere.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKSphere.h"
#include "DKLine.h"

using namespace DKFramework;

DKSphere::DKSphere(void)
: center(DKVector3(0,0,0))
, radius(-FLT_MAX)
{
}

DKSphere::DKSphere(const DKVector3& c, float r)
: center(c), radius(r)
{
}

DKSphere DKSphere::Union(const DKSphere& s1, const DKSphere& s2)
{
	if (s1.IsValid() && s2.IsValid())
	{
		float distance = (s1.center - s2.center).Length();
		if (s1.radius - s2.radius >= distance)		// s1 includes s2
		{
			return s1;
		}
		else if (s2.radius - s1.radius >= distance)	// s2 includes s1
		{
			return s2;
		}
		else	// should be bigger.
		{
			// new sphere's radius: distance between two sphere centers + each radius/2
			float r = (distance + s1.radius + s2.radius) * 0.5f;
			// new sphere's center: move from s2 to s1 with offset (new sphere's radius - radius of s2)
			DKVector3 center = ((s1.center - s2.center).Normalize() * (r - s2.radius)) + s2.center;
			return DKSphere(center, r);
		}
	}
	else if (s1.IsValid())
	{
		return s1;
	}
	else if (s2.IsValid())
	{
		return s2;
	}
	// both are invalid.
	return DKSphere();
}

DKSphere DKSphere::Intersection(const DKSphere&s1, const DKSphere& s2)
{
	if (s1.IsValid() && s2.IsValid())
	{
		float distance = (s1.center - s2.center).Length();
		if (distance <= s1.radius + s2.radius)
		{
			float newRadius = (s1.radius + s2.radius - distance) / 2;
			DKVector3 newCenter = s1.center + (s1.center - s2.center).Normalize() * (s1.radius - newRadius);

			return DKSphere(newCenter, newRadius);
		}
	}
	return DKSphere();
}

bool DKSphere::IsValid(void) const
{
	return radius >= 0;
}

bool DKSphere::IsPointInside(const DKVector3& pos) const
{
	return (pos - center).LengthSq() <= (radius * radius);
}

float DKSphere::Volume(void) const
{
	return (4.0f / 3.0f) * radius * radius * radius * static_cast<float>(DKGL_PI);	// (4/3 PI * R cubed )
}

bool DKSphere::RayTest(const DKLine& ray, DKVector3* p) const
{
	if (this->IsValid())
	{
		const DKVector3 l = this->center - ray.begin; // from line's begin to sphere center
		const DKVector3 d = ray.Direction();

		float s = DKVector3::Dot(l, d);
		float s2 = s*s;
		float l2 = DKVector3::Dot(l, l);				// l-vector length^2
		float r2 = this->radius * this->radius;

		if (s < 0 && l2 > r2)		// sphere located behind of line, no-intersection
			return false;

		float m2 = l2 - s2;			// calculate intersect length
		if (m2 > r2)				// intersect length^2 is bigger than radius^2, no-intersection
			return false;


		float t = 0;
		float q = sqrt(r2 - m2);
		if (l2 > r2)				// line begins outside of sphere, length of intersection (t = s-q)
			t = s-q;
		else						// line begins inside of sphere, length of intersection (t = s+q)
			t = s+q;

		if ( t >= 0 && t <= ray.Length())
		{
			if (p)		*p = ray.begin + d*t;
			return true;
		}
	}
	return false;
}
