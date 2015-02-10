//
//  File: DKLinearTransform2.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector2.h"
#include "DKMatrix2.h"
#include "DKMatrix3.h"

////////////////////////////////////////////////////////////////////////////////
// DKLinearTransform2
// 2x2 matrix for linear transform on 2 dimensional coordinates.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKLinearTransform2
	{
	public:
		DKLinearTransform2(void);
		DKLinearTransform2(float rotate);
		DKLinearTransform2(float scaleX, float scaleY);
		DKLinearTransform2(const DKVector2& axisX, const DKVector2& axisY);
		DKLinearTransform2(const DKMatrix2& m);

		DKLinearTransform2& Identity(void);
		bool IsIdentity(void) const;
		bool IsDiagonal(void) const;

		DKLinearTransform2& HorizontalShear(float s);
		DKLinearTransform2& VerticalShear(float s);
		DKLinearTransform2& HorizontalFlip(void);
		DKLinearTransform2& VerticalFlip(void);
		DKLinearTransform2& Squeeze(float s);
		DKLinearTransform2& Scale(const DKVector2& v);
		DKLinearTransform2& Scale(float x, float y);
		DKLinearTransform2& Scale(float s);
		DKLinearTransform2& Rotate(float angle);

		DKLinearTransform2& Inverse(void);
		DKLinearTransform2& Multiply(const DKMatrix2& t);
		DKLinearTransform2& Multiply(const DKLinearTransform2& t);

		DKLinearTransform2 operator * (const DKLinearTransform2& t) const;
		DKLinearTransform2& operator *= (const DKLinearTransform2& t);

		bool operator == (const DKLinearTransform2& t) const;
		bool operator != (const DKLinearTransform2& t) const;

		DKMatrix2& Matrix2(void)					{return matrix2;}
		const DKMatrix2& Matrix2(void) const		{return matrix2;}

		DKMatrix2 matrix2;
	};
	inline DKVector2 operator * (const DKVector2& v, const DKLinearTransform2& t)
	{
		return v * t.Matrix2();
	}
	inline DKVector2& operator *= (DKVector2& v, const DKLinearTransform2& t)
	{
		v *= t.Matrix2();
		return v;
	}
}
