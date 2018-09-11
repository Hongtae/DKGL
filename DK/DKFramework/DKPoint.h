//
//  File: DKPoint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector2.h"

#pragma pack(push, 4)
namespace DKFramework
{
	/// @brief a point unit. used by 2d graphics to represent a point.
	class DKGL_API DKPoint
	{
	public:
		DKPoint()
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
		DKVector2 Vector() const
		{
			return DKVector2(x, y);
		}

		float x;
		float y;

		static const DKPoint zero;
	};
}
#pragma pack(pop)
