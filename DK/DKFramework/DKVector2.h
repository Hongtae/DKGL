//
//  File: DKVector2.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

#pragma pack(push, 4)
namespace DKFramework
{
	class DKMatrix2;
	class DKMatrix3;
	/// @brief a Euclidean vector for two-dimensional space.
	class DKGL_API DKVector2
	{
	public:
		DKVector2();
		DKVector2(float x_, float y_);

		static float Dot(const DKVector2& v1, const DKVector2& v2); ///< dot product

		bool operator == (const DKVector2& v) const;
		bool operator != (const DKVector2& v) const;

		DKVector2 operator + (const DKVector2& v) const;
		DKVector2 operator - (const DKVector2& v) const;	// binary
		DKVector2 operator - () const;					// unary
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

		float Length() const;
		float LengthSq() const;			///< length^2

		DKVector2& Transform(const DKMatrix2& m);	///< Linear Transform
		DKVector2& Transform(const DKMatrix3& m);	///< Homogeneous Transform
		DKVector2& Normalize();

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
#pragma pack(pop)
