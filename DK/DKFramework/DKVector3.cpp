//
//  File: DKVector3.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVector3.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"

using namespace DKFramework;

const DKVector3 DKVector3::zero = DKVector3(0,0,0);

DKVector3::DKVector3()
	: x(0), y(0), z(0)
{
}

DKVector3::DKVector3(float x_, float y_, float z_)
	: x(x_), y(y_), z(z_)
{
}

DKVector3 DKVector3::Cross(const DKVector3& v1, const DKVector3& v2)
{
	return DKVector3(v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x);
}

float DKVector3::Dot(const DKVector3& v1, const DKVector3& v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

bool DKVector3::operator == (const DKVector3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool DKVector3::operator != (const DKVector3& v) const
{
	return x != v.x || y != v.y || z != v.z;
}

DKVector3 DKVector3::operator + (const DKVector3& v) const
{
	return DKVector3(x + v.x, y + v.y, z + v.z);
}

DKVector3 DKVector3::operator - (const DKVector3& v) const
{
	return DKVector3(x - v.x, y - v.y, z - v.z);
}

DKVector3 DKVector3::operator - () const
{
	return DKVector3(-x, -y, -z);
}

DKVector3 DKVector3::operator * (float f) const
{
	return DKVector3(x * f, y * f, z * f);
}

DKVector3 DKVector3::operator * (const DKVector3& v) const
{
	return DKVector3(x * v.x, y * v.y, z * v.z);
}

DKVector3 DKVector3::operator * (const DKMatrix3& m) const
{
	return DKVector3(x, y, z).Transform(m);
}

DKVector3 DKVector3::operator * (const DKMatrix4& m) const
{
	return DKVector3(x, y, z).Transform(m);
}

DKVector3 DKVector3::operator * (const DKQuaternion& q) const
{
	return DKVector3(x, y, z).Rotate(q);
}

DKVector3 DKVector3::operator / (float f) const
{
	float inv = 1.0f / f;
	return DKVector3(x * inv, y * inv, z * inv);
}

DKVector3 DKVector3::operator / (const DKVector3& v) const
{
	return DKVector3(x / v.x, y / v.y, z / v.z);
}

DKVector3& DKVector3::operator += (const DKVector3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

DKVector3& DKVector3::operator -= (const DKVector3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

DKVector3& DKVector3::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

DKVector3& DKVector3::operator *= (const DKVector3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

DKVector3& DKVector3::operator *= (const DKMatrix3& m)
{
	return Transform(m);
}

DKVector3& DKVector3::operator *= (const DKMatrix4& m)
{
	return Transform(m);
}

DKVector3& DKVector3::operator *= (const DKQuaternion& q)
{
	return Rotate(q);
}

DKVector3& DKVector3::operator /= (float f)
{
	float inv = 1.0f / f;
	x *= inv;
	y *= inv;
	z *= inv;
	return *this;
}

DKVector3& DKVector3::operator /= (const DKVector3& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

float DKVector3::Length() const
{
	return sqrt( x*x + y*y + z*z );
}

float DKVector3::LengthSq() const
{
	return x*x + y*y + z*z;
}

DKVector3& DKVector3::RotateX(float radian)
{
	if (radian != 0.0f)
	{
		float fCos = cos(radian);
		float fSin = sin(radian);

		float y2 = y*fCos - z*fSin;
		float z2 = y*fSin + z*fCos;

		y = y2;
		z = z2;
	}
	return *this;
}

DKVector3& DKVector3::RotateY(float radian)
{
	if (radian != 0.0f)
	{
		float fCos = cos(radian);
		float fSin = sin(-radian);

		float x2 = x*fCos - z*fSin;
		float z2 = x*fSin + z*fCos;

		x = x2;
		z = z2;
	}
	return *this;
}

DKVector3& DKVector3::RotateZ(float radian)
{
	if (radian != 0.0f)
	{
		float fCos = cos(radian);
		float fSin = sin(radian);

		float x2 = x*fCos - y*fSin;
		float y2 = x*fSin + y*fCos;

		x = x2;
		y = y2;
	}
	return *this;
}

DKVector3& DKVector3::Rotate(const DKVector3& axis, float radian)
{
	if (radian == 0)
		return *this;
	/*
	DKVector3 au = axis;
	au.Normalize();

	float fCosR = cos(radian);
	float fSinR = sin(radian);

	DKVector3 vec(0, 0, 0);

	vec.x += (fCosR + (1 - fCosR) * au.x * au.x) * x;
	vec.x += ((1 - fCosR) * au.x * au.y - au.z * fSinR) * y;
	vec.x += ((1 - fCosR) * au.x * au.z + au.y * fSinR) * z;

	vec.y += ((1 - fCosR) * au.x * au.y + au.z * fSinR) * x;
	vec.y += (fCosR + (1 - fCosR) * au.y * au.y) * y;
	vec.y += ((1 - fCosR) * au.y * au.z - au.x * fSinR) * z;

	vec.z += ((1 - fCosR) * au.x * au.z - au.y * fSinR) * x;
	vec.z += ((1 - fCosR) * au.y * au.z + au.x * fSinR) * y;
	vec.z += (fCosR + (1 - fCosR) * au.z * au.z) * z;

	return *this = vec;
	*/
	return Rotate(DKQuaternion(axis, radian));
}

DKVector3& DKVector3::Rotate(const DKQuaternion& q)
{
	DKVector3 vec(q.x, q.y, q.z);
	DKVector3 uv = DKVector3::Cross(vec, *this);
	DKVector3 uuv = DKVector3::Cross(vec, uv);
	uv *= (2.0f * q.w);
	uuv *= 2.0f;

	x += uv.x + uuv.x;
	y += uv.y + uuv.y;
	z += uv.z + uuv.z;
	return *this;
}

DKVector3& DKVector3::Transform(const DKMatrix3& m)
{
	DKVector3 vec(x, y, z);
	this->x = (vec.x * m.m[0][0]) + (vec.y * m.m[1][0]) + (vec.z * m.m[2][0]);
	this->y = (vec.x * m.m[0][1]) + (vec.y * m.m[1][1]) + (vec.z * m.m[2][1]);
	this->z = (vec.x * m.m[0][2]) + (vec.y * m.m[1][2]) + (vec.z * m.m[2][2]);
	return *this;
}

DKVector3& DKVector3::Transform(const DKMatrix4& m)
{
	DKVector3 vec(x, y, z);
	this->x = (vec.x * m.m[0][0]) + (vec.y * m.m[1][0]) + (vec.z * m.m[2][0]) + m.m[3][0];
	this->y = (vec.x * m.m[0][1]) + (vec.y * m.m[1][1]) + (vec.z * m.m[2][1]) + m.m[3][1];
	this->z = (vec.x * m.m[0][2]) + (vec.y * m.m[1][2]) + (vec.z * m.m[2][2]) + m.m[3][2];
	float w = 1.0f / ((vec.x * m.m[0][3]) + (vec.y * m.m[1][3]) + (vec.z * m.m[2][3]) + m.m[3][3]);

	this->x *= w;
	this->y *= w;
	this->z *= w;
	return *this;
}

DKVector3& DKVector3::Normalize()
{
	float lengthSq = x*x + y*y + z*z;
	if (lengthSq > 0.0)
	{
		float lenInv = 1.0f / sqrt(lengthSq);
		this->x *= lenInv;
		this->y *= lenInv;
		this->z *= lenInv;
	}
	return *this;
}

