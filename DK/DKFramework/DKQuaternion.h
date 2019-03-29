//
//  File: DKQuaternion.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

#pragma pack(push, 4)
namespace DKFramework
{
	class DKMatrix3;
	class DKMatrix4;
	class DKVector3;
	class DKVector4;
	/// @brief a quaternion utility class
	class DKGL_API DKQuaternion
	{
	public:
		DKQuaternion();
		DKQuaternion(const DKQuaternion& q);
		DKQuaternion(const DKVector3& axis, float angle); ///< angle is radian
		DKQuaternion(float pitch, float yaw, float roll); ///< radian
		DKQuaternion(const DKVector3& from, const DKVector3& to, float t = 1.0); ///< t > 1 for over-rotate, < 0 for inverse

		DKQuaternion(float _x, float _y, float _z, float _w);
		~DKQuaternion();

		/// spherical linear interpolation
		static DKQuaternion Slerp(const DKQuaternion& q1, const DKQuaternion& q2, float t);
		static float Dot(const DKQuaternion& q1, const DKQuaternion& q2);

		DKQuaternion& Zero();
		DKQuaternion& Identity();
		DKQuaternion& Normalize();
		bool IsIdentity() const;
		float Length() const;
		float LengthSq() const;
		float Roll() const;
		float Pitch() const;
		float Yaw() const;
		float Angle() const;

		DKQuaternion& Inverse();
		DKQuaternion& Conjugate();
		DKVector3 Axis() const;
		DKVector3 AxisAngle(float* pAngle) const;

		DKQuaternion& Multiply(const DKQuaternion& q);
		DKQuaternion& Multiply(float f);

		DKQuaternion operator + (const DKQuaternion& q) const;
		DKQuaternion operator - (const DKQuaternion& q) const;
		DKQuaternion operator - () const;
		DKQuaternion operator * (float f) const;
		DKQuaternion operator / (float f) const;
		DKQuaternion operator * (const DKQuaternion& q) const;

		DKQuaternion& operator = (const DKQuaternion& q);
		DKQuaternion& operator += (const DKQuaternion& q);
		DKQuaternion& operator -= (const DKQuaternion& q);
		DKQuaternion& operator *= (float f);
		DKQuaternion& operator /= (float f);
		DKQuaternion& operator *= (const DKQuaternion& q);

		bool operator == (const DKQuaternion& q) const;
		bool operator != (const DKQuaternion& q) const;

		DKMatrix3 Matrix3() const;
		DKMatrix4 Matrix4() const;

        union
		{
			struct {
				float x, y, z, w;
			};
			float val[4];
		};

		static const DKQuaternion identity;
	};
}
#pragma pack(pop)
