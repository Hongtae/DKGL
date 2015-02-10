//
//  File: DKRect.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKRect.h"

using namespace DKFoundation;
using namespace DKFramework;

const DKPoint DKPoint::zero = DKPoint(0,0);
const DKSize DKSize::zero = DKSize(0,0);
const DKRect DKRect::zero = DKRect(0,0,0,0);

DKRect::DKRect(void)
	: origin(DKPoint(0,0))
	, size(-FLT_MAX, -FLT_MAX)
{
}

DKRect::DKRect(const DKPoint& p, const DKSize& s)
	: origin(p)
	, size(s)
{
}

DKRect::DKRect(float x, float y, float width, float height)
	: origin(x, y)
	, size(width, height)
{
}

bool DKRect::operator == (const DKRect& rc) const
{
	return origin == rc.origin && size == rc.size;
}

bool DKRect::operator != (const DKRect& rc) const
{
	return origin != rc.origin || size != rc.size;
}

DKPoint DKRect::Center(void) const
{
	if (IsValid())
	{
		return DKPoint(origin.x + size.width * 0.5f, origin.y + size.height * 0.5f);
	}
	return origin;
}

DKRect& DKRect::SetCenter(const DKPoint& p)
{
	if (IsValid())
	{
		this->origin.x = p.x - size.width * 0.5f;
		this->origin.y = p.y - size.height * 0.5f;
	}
	return *this;
}

DKRect& DKRect::SetCenter(float x, float y)
{
	if (IsValid())
	{
		this->origin.x = x - size.width * 0.5f;
		this->origin.y = y - size.height * 0.5f;
	}
	return *this;
}

bool DKRect::IsPointInside(const DKPoint& pt) const
{
	return (pt.x >= origin.x && pt.x <= origin.x + size.width && pt.y >= origin.y && pt.y <= origin.y + size.height);
}

bool DKRect::IsValid(void) const
{
	return size.width >= 0 && size.height >= 0;
}

bool DKRect::Intersect(const DKRect& rc) const
{
	return Intersection(*this, rc).IsValid();
}

bool DKRect::IntersectLine(const DKPoint& pt1, const DKPoint& pt2) const
{
	if (IsValid() == false)
		return false;

	DKVector2 pos[2] = {
		DKVector2((pt1.x - origin.x) / size.width, (pt1.y - origin.y) / size.height),
		DKVector2((pt2.x - origin.x) / size.width, (pt2.y - origin.y) / size.height)
	};

	// check each point located over each axis
	if (pos[0].x < 0.0f && pos[1].x < 0.0f)
		return false;
	if (pos[0].x > 1.0f && pos[1].x > 1.0f)
		return false;
	if (pos[0].y < 0.0f && pos[1].y < 0.0f)
		return false;
	if (pos[0].y > 1.0f && pos[1].y > 1.0f)
		return false;
	// check point inside of 0~1
	if (pos[0].x >= 0.0f && pos[0].x <= 1.0f && pos[0].y >= 0.0f && pos[0].y <= 1.0f)
		return true;
	if (pos[1].x >= 0.0f && pos[1].x <= 1.0f && pos[1].y >= 0.0f && pos[1].y <= 1.0f)
		return true;

	float dX = pos[1].x - pos[0].x;	// x amount
	float dY = pos[1].y - pos[0].y;	// y amount

	if (fabs(dX) == 0)	// vertical aligned
		return true;
	if (fabs(dY) == 0)	// horizontal aligned
		return true;

	// formula y = (dY * x) / dX + b
	float b = pos[0].y - ((dY * pos[0].x) / dX); // y-intercept (y for x = 0)
	if ((dX * dY) > 0)	// positive slope
	{
		float y0 = (-b * dX) / dY;               // x-intercept (x for y = 0)

		if (b > 1.0f || y0 > 1.0f)
			return false;
	}
	else		// negative slope
	{
		float x1 = dY / dX + b;    // y for x = 1

		if (b < 0.0f || x1 > 1.0f)
			return false;
	}
	return true;
}

bool DKRect::IntersectTriangle(const DKPoint& pt1, const DKPoint& pt2, const DKPoint& pt3) const
{
	if (IsValid() == false)
		return false;

	DKVector2 pos[3] = {
		DKVector2((pt1.x - origin.x) / size.width, (pt1.y - origin.y) / size.height),
		DKVector2((pt2.x - origin.x) / size.width, (pt2.y - origin.y) / size.height),
		DKVector2((pt3.x - origin.x) / size.width, (pt3.y - origin.y) / size.height)
	};

	// check each point located over each axis
	if (pos[0].x < 0.0f && pos[1].x < 0.0f && pos[2].x < 0.0f)
		return false;
	if (pos[0].x > 1.0f && pos[1].x > 1.0f && pos[2].x > 1.0f)
		return false;
	if (pos[0].y < 0.0f && pos[1].y < 0.0f && pos[2].y < 0.0f)
		return false;
	if (pos[0].y > 1.0f && pos[1].y > 1.0f && pos[2].y > 1.0f)
		return false;

	// check point inside of 0~1
	if (pos[0].x >= 0.0f && pos[0].x <= 1.0f && pos[0].y >= 0.0f && pos[0].y <= 1.0f)
		return true;
	if (pos[1].x >= 0.0f && pos[1].x <= 1.0f && pos[1].y >= 0.0f && pos[1].y <= 1.0f)
		return true;
	if (pos[2].x >= 0.0f && pos[2].x <= 1.0f && pos[2].y >= 0.0f && pos[2].y <= 1.0f)
		return true;

	// all points located outside of rect (0~1,0~1).
	// test intersect with four line-segments of rect.
	struct LineIntersect
	{
		LineIntersect(const DKVector2& pt1, const DKVector2& pt2) : result(false)
		{
			if ((pt1.x < 0.0f && pt2.x < 0.0f) || (pt1.x > 1.0f && pt2.x > 1.0f) ||
				(pt1.y < 0.0f && pt2.y < 0.0f) || (pt1.y > 1.0f && pt2.y > 1.0f))
			{
			}
			else if (pt1.x >= 0.0f && pt1.x <= 1.0f && pt1.y >= 0.0f && pt1.y <= 1.0f)	// pt1 is inside
			{
				result = true;
			}
			else if (pt2.x >= 0.0f && pt2.x <= 1.0f && pt2.y >= 0.0f && pt2.y <= 1.0f)	// pt2 is inside
			{
				result = true;
			}
			else
			{
				float dx = pt2.x - pt1.x;
				float dy = pt2.y - pt1.y;

				// no intersection with vertical, horizontal (calculated above)
				if (fabs(dx) == 0.0f)			// vertical aligned
				{
				}
				else if (fabs(dy) == 0.0f)		// horizontal aligned
				{
				}
				else
				{
					// formula y = a * x + b, (a = dy/dx)
					float a = dy / dx;
					float b = pt1.y - a * pt1.x;		// y-intercept (y for x = 0)
					if (a > 0)							// positive slope
					{
						float y0 = (-b) / a;			// x-intercept (x for y = 0)
						if (b <= 1.0f && y0 <= 1.0f)
							result = true;
					}
					else
					{
						float x1 = a + b;				// y for x = 1
						if (b >= 0.0f && x1 <= 1.0f)
							result = true;
					}
				}
			}
		}
		bool result;
	};
	if (LineIntersect(pos[0], pos[1]).result)
		return true;
	if (LineIntersect(pos[1], pos[2]).result)
		return true;
	if (LineIntersect(pos[2], pos[0]).result)
		return true;

	// test intersect with point of triangle are included.
	// using cross-products for triangle direction (+z or -z),
	// (0,0),(0,1),(1,1),(1,0) and each points makes triangle direction for
	// intersection if direction is matched.
	struct Triangle
	{
		Triangle(const DKVector2& v1, const DKVector2& v2, const DKVector2& v3)
			: orientation((v1.x * (v2.y - v3.y) + v2.x * (v3.y - v1.y) + v3.x * (v1.y - v2.y)) > 0 ? 1 : -1) {}
		int orientation;	// 1 or -1
	};

	if (abs(
		Triangle(pos[0], pos[1], DKVector2(0,0)).orientation +
		Triangle(pos[1], pos[2], DKVector2(0,0)).orientation +
		Triangle(pos[2], pos[0], DKVector2(0,0)).orientation) == 3)
		return true;
	if (abs(
		Triangle(pos[0], pos[1], DKVector2(0,1)).orientation +
		Triangle(pos[1], pos[2], DKVector2(0,1)).orientation +
		Triangle(pos[2], pos[0], DKVector2(0,1)).orientation) == 3)
		return true;
	if (abs(
		Triangle(pos[0], pos[1], DKVector2(1,1)).orientation +
		Triangle(pos[1], pos[2], DKVector2(1,1)).orientation +
		Triangle(pos[2], pos[0], DKVector2(1,1)).orientation) == 3)
		return true;
	if (abs(
		Triangle(pos[0], pos[1], DKVector2(1,0)).orientation +
		Triangle(pos[1], pos[2], DKVector2(1,0)).orientation +
		Triangle(pos[2], pos[0], DKVector2(1,0)).orientation) == 3)
		return true;

	return false;
}

bool DKRect::IntersectRect(const DKRect& rc, const DKMatrix3& tm) const
{
	if (IsValid() == false)
		return false;
	if (rc.IsValid() == false)
		return false;

	DKVector2 pos[4] = {
		DKVector2(rc.origin.x, rc.origin.y).Transform(tm),
		DKVector2(rc.origin.x, rc.origin.y + rc.size.height).Transform(tm),
		DKVector2(rc.origin.x + rc.size.width, rc.origin.y + rc.size.height).Transform(tm),
		DKVector2(rc.origin.x + rc.size.width, rc.origin.y).Transform(tm)
	};

	if (IntersectTriangle(pos[0], pos[1], pos[3]) || IntersectTriangle(pos[3], pos[1], pos[2]))
		return true;
	return false;
}

DKRect DKRect::Intersection(const DKRect& rc1, const DKRect& rc2)
{
	if (rc1.IsValid() && rc2.IsValid())
	{
		float minWidth = DKFoundation::Min(rc1.origin.x + rc1.size.width, rc2.origin.x + rc2.size.width);
		float minHeight = DKFoundation::Min(rc1.origin.y + rc1.size.height, rc2.origin.y + rc2.size.height);

		DKPoint newOrigin = DKPoint(DKFoundation::Max(rc1.origin.x, rc2.origin.x), DKFoundation::Max(rc1.origin.y, rc2.origin.y));
		DKSize newSize = DKSize(minWidth - newOrigin.x, minHeight - newOrigin.y);

		return DKRect(newOrigin, newSize);
	}
	return DKRect();
}

DKRect DKRect::Union(const DKRect& rc1, const DKRect& rc2)
{
	if (rc1.IsValid() && rc2.IsValid())
	{
		float maxWidth = DKFoundation::Max(rc1.origin.x + rc1.size.width, rc2.origin.x + rc2.size.width);
		float maxHeight = DKFoundation::Max(rc1.origin.y + rc1.size.height, rc2.origin.x + rc2.size.height);

		DKPoint newOrigin = DKPoint(DKFoundation::Min(rc1.origin.x, rc2.origin.x), DKFoundation::Max(rc1.origin.y, rc2.origin.y));
		DKSize newSize = DKSize(maxWidth - newOrigin.x, maxHeight - newOrigin.y);

		return DKRect(newOrigin, newSize);	
	}
	else if (rc1.IsValid())
		return rc1;
	else if (rc2.IsValid())
		return rc2;
	return DKRect();
}
