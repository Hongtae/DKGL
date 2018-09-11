//
//  File: DKAffineTransform2.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector2.h"
#include "DKMatrix2.h"
#include "DKMatrix3.h"
#include "DKLinearTransform2.h"

#pragma pack(push, 4)
namespace DKFramework
{
	/// 3x2 matrix for affine transform on 2 dimensional coordinates.
	class DKGL_API DKAffineTransform2
	{
	public:
		DKAffineTransform2(const DKVector2& translate = DKVector2::zero);
		DKAffineTransform2(const DKLinearTransform2& linear, const DKVector2& translate = DKVector2::zero);
		DKAffineTransform2(const DKVector2& axisX, const DKVector2& axisY, const DKVector2& translate = DKVector2::zero);
		DKAffineTransform2(float x, float y);
		DKAffineTransform2(const DKMatrix3& m);

		DKAffineTransform2& Identity();
		bool IsIdentity() const;
		bool IsDiagonal() const;

		DKAffineTransform2& Translate(float x, float y);
		DKAffineTransform2& Translate(const DKVector2& v);

		DKAffineTransform2& Inverse();
		DKAffineTransform2& Multiply(const DKLinearTransform2& t);
		DKAffineTransform2& Multiply(const DKAffineTransform2& t);

		DKAffineTransform2 operator * (const DKLinearTransform2& t) const;
		DKAffineTransform2 operator * (const DKAffineTransform2& t) const;
		DKAffineTransform2& operator *= (const DKLinearTransform2& t);
		DKAffineTransform2& operator *= (const DKAffineTransform2& t);

		bool operator == (const DKAffineTransform2& t) const;
		bool operator != (const DKAffineTransform2& t) const;

		DKMatrix3 Matrix3() const;
		const DKMatrix2 Matrix2() const		{return matrix2;}

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
#pragma pack(pop)
