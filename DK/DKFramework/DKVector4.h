//
//  File: DKVector4.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

#pragma pack(push, 4)
namespace DKFramework
{
	class DKMatrix4;
	/// @brief a Euclidean vector for four-dimensional space.
	class DKGL_API DKVector4
	{
	public:
		DKVector4();
		DKVector4(float x_, float y_, float z_, float w_);

		static DKVector4 Cross(const DKVector4&v1, const DKVector4& v2, const DKVector4& v3);///< cross product
		static float Dot(const DKVector4& v1, const DKVector4& v2); ///< dot product

		bool operator == (const DKVector4& v) const;
		bool operator != (const DKVector4& v) const;

		DKVector4 operator + (const DKVector4& v) const;
		DKVector4 operator - (const DKVector4& v) const;
		DKVector4 operator - () const;
		DKVector4 operator * (float f) const;
		DKVector4 operator * (const DKVector4& v) const;
		DKVector4 operator * (const DKMatrix4& m) const;
		DKVector4 operator / (float f) const;
		DKVector4 operator / (const DKVector4& v) const;

		DKVector4& operator += (const DKVector4& v);
		DKVector4& operator -= (const DKVector4& v);
		DKVector4& operator *= (float f);
		DKVector4& operator *= (const DKVector4& v);
		DKVector4& operator *= (const DKMatrix4& m);
		DKVector4& operator /= (float f);
		DKVector4& operator /= (const DKVector4& v);

		float Length() const;
		float LengthSq() const; ///< length^2

		DKVector4& Transform(const DKMatrix4& m);
		DKVector4& Normalize();

		operator float* ()				{return val;}
		operator const float* () const	{return val;}

		union
		{
			struct
			{
				float x, y, z, w;
			};
			float val[4];
		};

		static const DKVector4 zero;
	};
}
#pragma pack(pop)
