//
//  File: DKRect.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKPoint.h"
#include "DKSize.h"

////////////////////////////////////////////////////////////////////////////////
// DKRect
// represents rectangle on 2D graphics.
// origin of rect is left-bottom corner and origin is DKPoint object.
// size of rect is DKSize object and can not be negative.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKRect
	{
	public:
		DKRect(void);
		DKRect(const DKPoint& p, const DKSize& s);
		DKRect(float x, float y, float width, float height);

		DKPoint Center(void) const;
		DKRect& SetCenter(const DKPoint& p);
		DKRect& SetCenter(float x, float y);
		
		bool IsPointInside(const DKPoint& pt) const;
		bool IsValid(void) const;
		bool Intersect(const DKRect& rc) const;
		bool IntersectLine(const DKPoint& pt1, const DKPoint& pt2) const;
		bool IntersectTriangle(const DKPoint& pt1, const DKPoint& pt2, const DKPoint& pt3) const;
		bool IntersectRect(const DKRect& rc, const DKMatrix3& tm) const;

		bool operator == (const DKRect& rc) const;
		bool operator != (const DKRect& rc) const;

		static DKRect Intersection(const DKRect& rc1, const DKRect& rc2);
		static DKRect Union(const DKRect& rc1, const DKRect& rc2);

		DKPoint origin;
		DKSize size;

		static const DKRect zero;
	};
}
