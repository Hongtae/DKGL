//
//  File: DKVector3.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKVector3
// a Euclidean vector for three-dimensional space.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKMatrix3;
	class DKMatrix4;
	class DKQuaternion;

	class DKLIB_API DKVector3
	{
	public:
		DKVector3(void);
		DKVector3(float x_, float y_, float z_);

		static DKVector3 Cross(const DKVector3&v1, const DKVector3& v2); // cross product
		static float Dot(const DKVector3& v1, const DKVector3& v2);      // dot product

		bool operator == (const DKVector3& v) const;
		bool operator != (const DKVector3& v) const;

		DKVector3 operator + (const DKVector3& v) const;
		DKVector3 operator - (const DKVector3& v) const;	// binary
		DKVector3 operator - (void) const;					// unary
		DKVector3 operator * (float f) const;
		DKVector3 operator * (const DKVector3& v) const;
		DKVector3 operator * (const DKMatrix3& m) const;
		DKVector3 operator * (const DKMatrix4& m) const;
		DKVector3 operator * (const DKQuaternion& q) const;
		DKVector3 operator / (float f) const;
		DKVector3 operator / (const DKVector3& v) const;

		DKVector3& operator += (const DKVector3& v);
		DKVector3& operator -= (const DKVector3& v);
		DKVector3& operator *= (float f);
		DKVector3& operator *= (const DKVector3& v);
		DKVector3& operator *= (const DKMatrix3& m);
		DKVector3& operator *= (const DKMatrix4& m);
		DKVector3& operator *= (const DKQuaternion& m);
		DKVector3& operator /= (float f);
		DKVector3& operator /= (const DKVector3& v);

		float Length(void) const;
		float LengthSq(void) const;			// length^2

		DKVector3& RotateX(float radian);
		DKVector3& RotateY(float radian);
		DKVector3& RotateZ(float radian);
		DKVector3& Rotate(const DKVector3& axis, float radian);
		DKVector3& Rotate(const DKQuaternion& q);
		DKVector3& Transform(const DKMatrix3& m);	// Linear Transform
		DKVector3& Transform(const DKMatrix4& m);	// Homogeneous Transform
		DKVector3& Normalize(void);

		operator float* (void)				{return val;}
		operator const float* (void) const	{return val;}

		union
		{
			struct
			{
				float x, y, z;
			};
			float val[3];
		};

		static const DKVector3 zero;
	};
}
