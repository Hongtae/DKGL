//
//  File: DKSpline.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKSpline.h"

namespace DKFramework
{
	namespace Private
	{
		inline float CatmullRom(float u, float u_2, float u_3, float cntrl0, float cntrl1, float cntrl2, float cntrl3)
		{
			return (
					(-1.0f*u_3 + 2.0f*u_2 - 1.0f*u + 0.0f) * cntrl0 +
					( 3.0f*u_3 - 5.0f*u_2 + 0.0f*u + 2.0f) * cntrl1 +
					(-3.0f*u_3 + 4.0f*u_2 + 1.0f*u + 0.0f) * cntrl2 +
					( 1.0f*u_3 - 1.0f*u_2 + 0.0f*u + 0.0f) * cntrl3 ) / 2.0f;
		}
		inline float UniformCubic(float u, float u_2, float u_3, float cntrl0, float cntrl1, float cntrl2, float cntrl3)
		{
			return (
					(-1.0f*u_3 + 3.0f*u_2 - 3.0f*u + 1.0f) * cntrl0 +
					( 3.0f*u_3 - 6.0f*u_2 + 0.0f*u + 4.0f) * cntrl1 +
					(-3.0f*u_3 + 3.0f*u_2 + 3.0f*u + 1.0f) * cntrl2 +
					( 1.0f*u_3 + 0.0f*u_2 + 0.0f*u + 0.0f) * cntrl3 ) / 6.0f;
		}
		inline float Hermite(float u, float u_2, float u_3, float cntrl0, float cntrl1, float cntrl2, float cntrl3)
		{
			return (
					( 2.0f*u_3 - 3.0f*u_2 + 0.0f*u + 1.0f) * cntrl0 +
					(-2.0f*u_3 + 3.0f*u_2 + 0.0f*u + 0.0f) * cntrl1 +
					( 1.0f*u_3 - 2.0f*u_2 + 1.0f*u + 0.0f) * cntrl2 +
					( 1.0f*u_3 - 1.0f*u_2 + 0.0f*u + 0.0f) * cntrl3 );
		}
		inline float Bezier(float u, float u_2, float u_3, float cntrl0, float cntrl1, float cntrl2, float cntrl3)
		{
			return (
					(-1.0f*u_3 + 3.0f*u_2 - 3.0f*u + 1.0f) * cntrl0 +
					( 3.0f*u_3 - 6.0f*u_2 + 3.0f*u + 0.0f) * cntrl1 +
					(-3.0f*u_3 + 3.0f*u_2 + 0.0f*u + 0.0f) * cntrl2 +
					( 1.0f*u_3 + 0.0f*u_2 + 0.0f*u + 0.0f) * cntrl3 );
		}
	}
}
using namespace DKFramework;

DKSpline::DKSpline() : DKSpline(0.0f, 0.0f, 0.0f, 0.0f)
{
}

DKSpline::DKSpline(float p0, float p1, float p2, float p3)
: point0(p0), point1(p1), point2(p2), point3(p3)
{
}

float DKSpline::Interpolate(float t, Type c) const
{
	float t2 = t*t;
	float t3 = t2*t;

	float result = 0.0f;

	switch (c)
	{
	case CatmullRom:
		result = Private::CatmullRom(t, t2, t3, point0, point1, point2, point3);
		break;
	case UniformCubic:
		result = Private::UniformCubic(t, t2, t3, point0, point1, point2, point3);
		break;
	case Hermite:
		result = Private::Hermite(t, t2, t3, point0, point1, point2, point3);
		break;
	case Bezier:
		result = Private::Bezier(t, t2, t3, point0, point1, point2, point3);
		break;
	}
	return result;
}

