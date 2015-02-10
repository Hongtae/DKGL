//
//  File: DKAffineTransform2.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKVector2.h"
#include "DKMatrix2.h"
#include "DKMatrix3.h"
#include "DKLinearTransform2.h"

////////////////////////////////////////////////////////////////////////////////
// DKAffineTransform2
// 3x2 matrix for affine transform on 2 dimensional coordinates.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKAffineTransform2
	{
	public:
		DKAffineTransform2(const DKVector2& translate = DKVector2::zero);
		DKAffineTransform2(const DKLinearTransform2& linear, const DKVector2& translate = DKVector2::zero);
		DKAffineTransform2(const DKVector2& axisX, const DKVector2& axisY, const DKVector2& translate = DKVector2::zero);
		DKAffineTransform2(float x, float y);
		DKAffineTransform2(const DKMatrix3& m);

		DKAffineTransform2& Identity(void);
		bool IsIdentity(void) const;
		bool IsDiagonal(void) const;

		DKAffineTransform2& Translate(float x, float y);
		DKAffineTransform2& Translate(const DKVector2& v);

		DKAffineTransform2& Inverse(void);
		DKAffineTransform2& Multiply(const DKLinearTransform2& t);
		DKAffineTransform2& Multiply(const DKAffineTransform2& t);

		DKAffineTransform2 operator * (const DKLinearTransform2& t) const;
		DKAffineTransform2 operator * (const DKAffineTransform2& t) const;
		DKAffineTransform2& operator *= (const DKLinearTransform2& t);
		DKAffineTransform2& operator *= (const DKAffineTransform2& t);

		bool operator == (const DKAffineTransform2& t) const;
		bool operator != (const DKAffineTransform2& t) const;

		DKMatrix3 Matrix3(void) const;
		const DKMatrix2 Matrix2(void) const		{return matrix2;}

		DKMatrix2	matrix2;
		DKVector2	translation;
	};
	inline DKVector2 operator * (const DKVector2& v, const DKAffineTransform2& t)
	{
		return v * t.Matrix2() + t.translation;
	}
	inline DKVector2& operator *= (DKVector2& v, const DKAffineTransform2& t)
	{
		v *= t.Matrix2();
		v += t.translation;
		return v;
	}
}
