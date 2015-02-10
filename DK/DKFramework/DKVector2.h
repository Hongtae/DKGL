//
//  File: DKVector2.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKVector2
// a Euclidean vector for two-dimensional space.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKMatrix2;
	class DKMatrix3;

	class DKLIB_API DKVector2
	{
	public:
		DKVector2(void);
		DKVector2(float x_, float y_);

		static float Dot(const DKVector2& v1, const DKVector2& v2); // dot product

		bool operator == (const DKVector2& v) const;
		bool operator != (const DKVector2& v) const;

		DKVector2 operator + (const DKVector2& v) const;
		DKVector2 operator - (const DKVector2& v) const;	// binary
		DKVector2 operator - (void) const;					// unary
		DKVector2 operator * (float f) const;
		DKVector2 operator * (const DKVector2& v) const;
		DKVector2 operator * (const DKMatrix2& m) const;
		DKVector2 operator * (const DKMatrix3& m) const;
		DKVector2 operator / (float f) const;
		DKVector2 operator / (const DKVector2& v) const;

		DKVector2& operator += (const DKVector2& v);
		DKVector2& operator -= (const DKVector2& v);
		DKVector2& operator *= (float f);
		DKVector2& operator *= (const DKVector2& v);
		DKVector2& operator *= (const DKMatrix2& m);
		DKVector2& operator *= (const DKMatrix3& m);
		DKVector2& operator /= (float f);
		DKVector2& operator /= (const DKVector2& v);

		float Length(void) const;
		float LengthSq(void) const;			// length^2

		DKVector2& Transform(const DKMatrix2& m);	// Linear Transform
		DKVector2& Transform(const DKMatrix3& m);	// Homogeneous Transform
		DKVector2& Normalize(void);

		operator float* (void)				{return val;}
		operator const float* (void) const	{return val;}

		union
		{
			struct
			{
				float x, y;
			};
			float val[2];
		};

		static const DKVector2 zero;
	};
}
