//
//  File: DKPoint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector2.h"

////////////////////////////////////////////////////////////////////////////////
// DKPoint
// a point unit. used by 2d graphics to represent a point.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKPoint
	{
	public:
		DKPoint(void)
			: x(0), y(0)
		{
		}
		DKPoint(const DKVector2& v)
			: x(v.x), y(v.y)
		{
		}
		DKPoint(float x1, float y1)
			: x(x1), y(y1)
		{
		}
		bool operator != (const DKPoint& pt) const
		{
			return x != pt.x || y != pt.y;
		}
		bool operator == (const DKPoint& pt) const
		{
			return x == pt.x && y == pt.y;
		}
		DKPoint operator - (const DKPoint& pt) const
		{
			return DKPoint(x - pt.x, y - pt.y);
		}
		DKPoint operator - (float p) const
		{
			return DKPoint(x - p, y - p);
		}
		DKPoint operator + (const DKPoint& pt) const
		{
			return DKPoint(x + pt.x, y + pt.y);
		}
		DKPoint operator + (float p) const
		{
			return DKPoint(x + p, y + p);
		}
		DKPoint operator * (const DKPoint& pt) const
		{
			return DKPoint(x * pt.x, y * pt.y);
		}
		DKPoint operator * (float p) const
		{
			return DKPoint(x * p, y * p);
		}
		DKPoint operator / (const DKPoint& pt) const
		{
			return DKPoint(x / pt.x, y / pt.y);
		}
		DKPoint operator / (float p) const
		{
			return DKPoint(x / p, y / p);
		}
		DKPoint& operator -= (const DKPoint& pt)
		{
			x -= pt.x;
			y -= pt.y;
			return *this;
		}
		DKPoint& operator -= (float p)
		{
			x -= p;
			y -= p;
			return *this;
		}
		DKPoint& operator += (const DKPoint& pt)
		{
			x += pt.x;
			y += pt.y;
			return *this;
		}
		DKPoint& operator += (float p)
		{
			x += p;
			y += p;
			return *this;
		}
		DKPoint& operator *= (const DKPoint& pt)
		{
			x *= pt.x;
			y *= pt.y;
			return *this;
		}
		DKPoint& operator *= (float p)
		{
			x *= p;
			y *= p;
			return *this;
		}
		DKPoint& operator /= (const DKPoint& pt)
		{
			x /= pt.x;
			y /= pt.y;
			return *this;
		}
		DKPoint& operator /= (float p)
		{
			x /= p;
			y /= p;
			return *this;
		}
		DKVector2 Vector(void) const
		{
			return DKVector2(x, y);
		}

		float x;
		float y;

		static const DKPoint zero;
	};
}
