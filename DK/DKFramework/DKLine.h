//
//  File: DKLine.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"

////////////////////////////////////////////////////////////////////////////////
// DKLine
// line segment object.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKLine
	{
	public:
		DKLine(void);
		DKLine(const DKVector3& b, const DKVector3& e);

		DKLine operator * (const DKMatrix4& m) const;
		DKLine operator * (const DKQuaternion& q) const;

		DKVector3 Direction(void) const;
		float Length(void) const;
		bool IsPointInside(const DKVector3& pos) const;

		bool Intersect(const DKLine& line, DKVector3* p = NULL) const;

		// compute shortest line segment between two line segments.
		// result line's begin-point located on line1, end-point located on line2.
		static DKLine LineBetween(const DKLine& line1, const DKLine& line2);

		DKVector3	begin;
		DKVector3	end;
	};
}
