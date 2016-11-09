//
//  File: DKQuaternion.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKQuaternion.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"
#include "DKVector3.h"
#include "DKVector4.h"

using namespace DKFramework;

const DKQuaternion DKQuaternion::identity = DKQuaternion().Identity();

DKQuaternion::DKQuaternion(void)
	: x(0), y(0), z(0), w(1)
{
}

DKQuaternion::DKQuaternion(const DKQuaternion& q)
	: x(q.x), y(q.y), z(q.z), w(q.w)
{
}

DKQuaternion::DKQuaternion(const DKVector3& axis, float angle)
	: x(0), y(0), z(0), w(1)
{
	if (axis.Length() > 0)
	{
		DKVector3 au = DKVector3(axis).Normalize();

		angle *= 0.5f;
		float sinR = sin(angle);

		x = sinR * au.x;
		y = sinR * au.y;
		z = sinR * au.z;
		w = cos(angle);
	}
}

DKQuaternion::DKQuaternion(float pitch, float yaw, float roll)
	: x(0), y(0), z(0), w(1)
{
	float p = pitch * 0.5f;
	float y = yaw * 0.5f;
	float r = roll * 0.5f;

	float sinp = sin(p);
	float cosp = cos(p);
	float siny = sin(y);
	float cosy = cos(y);
	float sinr = sin(r);
	float cosr = cos(r);

	this->x = cosr * sinp * cosy + sinr * cosp * siny;
	this->y = cosr * cosp * siny - sinr * sinp * cosy;
	this->z = sinr * cosp * cosy - cosr * sinp * siny;
	this->w = cosr * cosp * cosy + sinr * sinp * siny;

	Normalize();
}

DKQuaternion::DKQuaternion(const DKVector3& from, const DKVector3& to, float t)
	: x(0), y(0), z(0), w(1)
{
	float len1 = from.Length();
	float len2 = to.Length();
	if (len1 > 0 && len2 > 0)
	{
		DKVector3 axis = DKVector3::Cross(from, to);
		float angle = acos(DKVector3::Dot(DKVector3(from).Normalize(), DKVector3(to).Normalize()));
		angle = angle * t;

		DKQuaternion q = DKQuaternion(axis, angle);
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
	}
}

DKQuaternion::DKQuaternion(float _x, float _y, float _z, float _w)
	: x(_x), y(_y), z(_z), w(_w)
{
}

DKQuaternion::~DKQuaternion(void)
{
}

DKQuaternion DKQuaternion::Slerp(const DKQuaternion& q1, const DKQuaternion& q2, float t)
{
	// dot-product of two quaternions (angle of two quats)
	double cosHalfTheta = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
	bool flip = cosHalfTheta < 0.0;
	if (flip)
		cosHalfTheta = -cosHalfTheta;

	if (cosHalfTheta >= 1.0) // q1 = q2 or q1 = -q2
		return q1;

	float halfTheta = acos(cosHalfTheta);
	float oneOverSinHalfTheta = 1.0 / sin(halfTheta);

	float t2 = 1.0 - t;

	float ratio1 = sin(halfTheta * t2) * oneOverSinHalfTheta;
	float ratio2 = sin(halfTheta * t) * oneOverSinHalfTheta;

	if (flip)
		ratio2 = -ratio2;

	return DKQuaternion(ratio1 * q1.x + ratio2 * q2.x,
		ratio1 * q1.y + ratio2 * q2.y,
		ratio1 * q1.z + ratio2 * q2.z,
		ratio1 * q1.w + ratio2 * q2.w);
}

float DKQuaternion::Dot(const DKQuaternion& q1, const DKQuaternion& q2)
{
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

DKQuaternion& DKQuaternion::Zero(void)
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
	return *this;
}

DKQuaternion& DKQuaternion::Identity(void)
{
	x = 0;
	y = 0;
	z = 0;
	w = 1;
	return *this;
}

bool DKQuaternion::IsIdentity(void) const
{
	return x == 0 && y == 0 && z == 0 && w == 1;
}

DKQuaternion& DKQuaternion::Normalize(void)
{
	float lengthSq = (x * x + y * y + z * z + w * w);
	if (lengthSq > 0.0f)
	{
		float lenInv = 1.0f / sqrt(lengthSq);
		this->x *= lenInv;
		this->y *= lenInv;
		this->z *= lenInv;
		this->w *= lenInv;
	}
	return *this;
}

DKQuaternion& DKQuaternion::Inverse(void)
{
	float n = (x * x + y * y + z * z + w * w);
	if (n > 0.0f)
	{
		float inv = 1.0f / n;
		this->x *= -inv;
		this->y *= -inv;
		this->z *= -inv;
		this->w *= inv;
	}
	return *this;
}

DKQuaternion& DKQuaternion::Conjugate(void)
{
	return *this = DKQuaternion( -x, -y, -z, w );
}

float DKQuaternion::Length(void) const
{
	return sqrt(x * x + y * y + z * z + w * w);
}

float DKQuaternion::LengthSq(void) const
{
	return (x * x + y * y + z * z + w * w);
}

float DKQuaternion::Roll(void) const
{
	return atan2(2*(x*y + w*z), w*w + x*x - y*y - z*z);
}

float DKQuaternion::Pitch(void) const
{
	return atan2(2*(y*z + w*x), w*w - x*x - y*y + z*z);
}

float DKQuaternion::Yaw(void) const
{
	return asin(-2*(x*z - w*y));
}

float DKQuaternion::Angle(void) const
{
	float lenSq = (x * x + y * y + z * z + w * w);
	if (lenSq > 0.0f && fabs(w) < 1.0f)
	{
		return 2.0f * acos(w);
	}
	return 0.0f;
}

DKVector3 DKQuaternion::Axis(void) const
{
	float lenSq = (x * x + y * y + z * z + w * w);
	if (lenSq > 0.0f)
	{
		float inv = 1.0f / sqrt(lenSq);
		return DKVector3(x * inv, y * inv, z * inv);
	}
	return DKVector3(1,0,0);
}

DKVector3 DKQuaternion::AxisAngle(float* pAngle) const
{
	DKVector3 axis;
	float angle = 0.0f;
	float n = sqrt(x * x + y * y + z * z + w * w);
	if (n > FLT_EPSILON)
	{
		float inv = 1.0f / n;
		axis.x = x * inv;
		axis.y = y * inv;
		axis.z = z * inv;
		if (fabs(w) < 1.0f)
			angle = 2.0f * acos(w);
	}
	else
	{
		axis.x = 1.0f;
		axis.y = 0.0f;
		axis.z = 0.0f;
	}

	if (pAngle)
		*pAngle = angle;

	return axis;
}


DKQuaternion& DKQuaternion::Multiply(const DKQuaternion& q)
{
	DKQuaternion quat(x, y, z, w);
	x =	q.w * quat.x + q.x * quat.w + q.y * quat.z - q.z * quat.y;		// x
	y =	q.w * quat.y + q.y * quat.w + q.z * quat.x - q.x * quat.z;		// y
	z =	q.w * quat.z + q.z * quat.w + q.x * quat.y - q.y * quat.x;		// z
	w =	q.w * quat.w - q.x * quat.x - q.y * quat.y - q.z * quat.z;		// w
	return *this;
}

DKQuaternion& DKQuaternion::Multiply(float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

DKQuaternion& DKQuaternion::operator = (const DKQuaternion& q)
{
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
	return *this;
}

DKQuaternion DKQuaternion::operator + (const DKQuaternion& q) const
{
	return DKQuaternion(x + q.x, y + q.y, z + q.z, w + q.w);
}

DKQuaternion DKQuaternion::operator - (const DKQuaternion& q) const
{
	return DKQuaternion(x - q.x, y - q.y, z - q.z, w - q.w);
}

DKQuaternion DKQuaternion::operator - (void) const
{
	return DKQuaternion(-x, -y, -z, -w);
}

DKQuaternion DKQuaternion::operator * (float f) const
{
	return DKQuaternion(x * f, y * f, z * f, w * f);
}

DKQuaternion DKQuaternion::operator / (float f) const
{
	return DKQuaternion(x / f, y / f, z / f, w / f);
}

DKQuaternion DKQuaternion::operator * (const DKQuaternion& q) const
{
	return DKQuaternion(
		q.w * x + q.x * w + q.y * z - q.z * y,		// x
		q.w * y + q.y * w + q.z * x - q.x * z,		// y
		q.w * z + q.z * w + q.x * y - q.y * x,		// z
		q.w * w - q.x * x - q.y * y - q.z * z		// w
		);
}

DKQuaternion& DKQuaternion::operator += (const DKQuaternion& q)
{
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;
	return *this;
}

DKQuaternion& DKQuaternion::operator -= (const DKQuaternion& q)
{
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;
	return *this;
}

DKQuaternion& DKQuaternion::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

DKQuaternion& DKQuaternion::operator /= (float f)
{
	x /= f;
	y /= f;
	z /= f;
	w /= f;
	return *this;
}

DKQuaternion& DKQuaternion::operator *= (const DKQuaternion& q)
{
	return Multiply(q);
}

bool DKQuaternion::operator == (const DKQuaternion& q) const
{
	return this->x == q.x && this->y == q.y && this->z == q.z && this->w == q.w;
}

bool DKQuaternion::operator != (const DKQuaternion& q) const
{
	return this->x != q.x || this->y != q.y || this->z != q.z || this->w != q.w;
}

DKMatrix4 DKQuaternion::Matrix4(void) const
{
	DKMatrix4 mat;
	mat.Identity();

	mat.m[0][0] = 1.0f - 2.0f * (y * y + z * z);
	mat.m[0][1] = 2.0f * (x * y + z * w);
	mat.m[0][2] = 2.0f * (x * z - y * w);

	mat.m[1][0] = 2.0f * (x * y - z * w);
	mat.m[1][1] = 1.0f - 2.0f * (x * x + z * z);
	mat.m[1][2] = 2.0f * (y * z + x * w);

	mat.m[2][0] = 2.0f * (x * z + y * w);
	mat.m[2][1] = 2.0f * (y * z - x * w);
	mat.m[2][2] = 1.0f - 2.0f * (x * x + y * y);
	return mat;
}

DKMatrix3 DKQuaternion::Matrix3(void) const
{
	DKMatrix3 mat;
	mat.Identity();

	mat.m[0][0] = 1.0f - 2.0f * (y * y + z * z);
	mat.m[0][1] = 2.0f * (x * y + z * w);
	mat.m[0][2] = 2.0f * (x * z - y * w);

	mat.m[1][0] = 2.0f * (x * y - z * w);
	mat.m[1][1] = 1.0f - 2.0f * (x * x + z * z);
	mat.m[1][2] = 2.0f * (y * z + x * w);

	mat.m[2][0] = 2.0f * (x * z + y * w);
	mat.m[2][1] = 2.0f * (y * z - x * w);
	mat.m[2][2] = 1.0f - 2.0f * (x * x + y * y);
	return mat;
}
