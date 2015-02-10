//
//  File: DKSpline.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector2.h"
#include "DKVector3.h"

////////////////////////////////////////////////////////////////////////////////
// DKSpline
// spline class, calculates spline curve.
// used to get smooth interpolated curve between points.
//
// CatmullRom: curve of intersecting two connecting points. this spline
//     guaranteeing that each point will be hit exactly.
//     with given 4 points (p0,p1,p2,p3), curve between p1,p2
//
// UniformCubic: basis spline (B-spline), this is not interpolation curve.
//     this curve does not pass through the points.
//     the curve is completely contained in the convex hull of its control
//     points.
//     with given 4 points (p0,p1,p2,p3), curve between p1,p2
//
// Hermite: calculate curve with tangent vectors.
//     with given 4 points (p0,p1,p2,p3), curve between p0,p1
//     and p2 is tangent vector or p0, p3 is tangent vector of p1.
//     a tangent vector (p2, p3) can control curve direction and speed.
//
// Bezier: 3 degree bezier curve.
//     the curve is completely contained in the convex hull of its control
//     points.
//     with given 4 points (p0,p1,p2,p3), curve between p0,p1,p2,p3.
//     p1,p2 is control points. (curve not pass this points)
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKSpline
	{
	public:
		enum Type
		{
			CatmullRom = 0,
			UniformCubic,
			Hermite,
			Bezier,
		};

		DKSpline(void);
		DKSpline(float p0, float p1, float p2, float p3);

		float Interpolate(float t, Type c) const;

		union
		{
			struct
			{
				float point0;
				float point1;
				float point2;
				float point3;
			};
			float points[4];
		};
	};

	class DKSpline2
	{
	public:
		DKSpline2(void)
			:point0(0, 0), point1(0, 0), point2(0, 0), point3(0, 0)
		{
		}
		DKSpline2(const DKVector2& p0, const DKVector2& p1, const DKVector2& p2, const DKVector2& p3)
			: point0(p0), point1(p1), point2(p2), point3(p3)
		{
		}

		DKVector2 Interpolate(float t, DKSpline::Type c) const
		{
			return DKVector2(
				DKSpline(point0.x, point1.x, point2.x, point3.x).Interpolate(t, c),
				DKSpline(point0.y, point1.y, point2.y, point3.y).Interpolate(t, c)
				);
		}

		DKVector2 point0;
		DKVector2 point1;
		DKVector2 point2;
		DKVector2 point3;
	};

	class DKSpline3
	{
	public:
		DKSpline3(void)
			:point0(0, 0, 0), point1(0, 0, 0), point2(0, 0, 0), point3(0, 0, 0)
		{
		}
		DKSpline3(const DKVector3& p0, const DKVector3& p1, const DKVector3& p2, const DKVector3& p3)
			: point0(p0), point1(p1), point2(p2), point3(p3)
		{
		}

		DKVector3 Interpolate(float t, DKSpline::Type c) const
		{
			return DKVector3(
				DKSpline(point0.x, point1.x, point2.x, point3.x).Interpolate(t, c),
				DKSpline(point0.y, point1.y, point2.y, point3.y).Interpolate(t, c),
				DKSpline(point0.z, point1.z, point2.z, point3.z).Interpolate(t, c)
				);
		}

		DKVector3 point0;
		DKVector3 point1;
		DKVector3 point2;
		DKVector3 point3;
	};
}
