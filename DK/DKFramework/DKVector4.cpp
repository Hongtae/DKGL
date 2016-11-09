//
//  File: DKVector4.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVector4.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"

using namespace DKFramework;

const DKVector4 DKVector4::zero = DKVector4(0,0,0,0);

DKVector4::DKVector4(void)
	: x(0), y(0), z(0), w(0)
{
}

DKVector4::DKVector4(float x_, float y_, float z_, float w_)
	: x(x_), y(y_), z(z_), w(w_)
{
}

DKVector4 DKVector4::Cross(const DKVector4& v1, const DKVector4& v2, const DKVector4& v3)
{
	return DKVector4(
		v1.y * (v2.z * v3.w - v3.z * v2.w) - v1.z * (v2.y * v3.w - v3.y * v2.w) + v1.w * (v2.y * v3.z - v2.z *v3.y) ,
		-(v1.x * (v2.z * v3.w - v3.z * v2.w) - v1.z * (v2.x * v3.w - v3.x * v2.w) + v1.w * (v2.x * v3.z - v3.x * v2.z)) ,
		v1.x * (v2.y * v3.w - v3.y * v2.w) - v1.y * (v2.x *v3.w - v3.x * v2.w) + v1.w * (v2.x * v3.y - v3.x * v2.y) ,
		-(v1.x * (v2.y * v3.z - v3.y * v2.z) - v1.y * (v2.x * v3.z - v3.x *v2.z) + v1.z * (v2.x * v3.y - v3.x * v2.y)) );
}

float DKVector4::Dot(const DKVector4& v1, const DKVector4& v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}

bool DKVector4::operator == (const DKVector4& v) const
{
	return x == v.x && y == v.y && z == v.z && w == v.w;
}

bool DKVector4::operator != (const DKVector4& v) const
{
	return x != v.x || y != v.y || z != v.z || w != v.w;
}

DKVector4 DKVector4::operator + (const DKVector4& v) const
{
	return DKVector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

DKVector4 DKVector4::operator - (const DKVector4& v) const
{
	return DKVector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

DKVector4 DKVector4::operator - (void) const
{
	return DKVector4(-x, -y, -z, -w);
}

DKVector4 DKVector4::operator * (float f) const
{
	return DKVector4(x * f, y * f, z * f, w * f);
}

DKVector4 DKVector4::operator * (const DKVector4& v) const
{
	return DKVector4(x * v.x, y * v.y, z * v.z, w * v.w);
}

DKVector4 DKVector4::operator * (const DKMatrix4& m) const
{
	return DKVector4(*this).Transform(m);
}

DKVector4 DKVector4::operator / (float f) const
{
	float inv = 1.0f / f;
	return DKVector4(x * inv, y * inv, z * inv, w * inv);
}

DKVector4 DKVector4::operator / (const DKVector4& v) const
{
	return DKVector4(x / v.x, y / v.y, z / v.z, w / v.w);
}

DKVector4& DKVector4::operator += (const DKVector4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

DKVector4& DKVector4::operator -= (const DKVector4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

DKVector4& DKVector4::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

DKVector4& DKVector4::operator *= (const DKVector4& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}

DKVector4& DKVector4::operator *= (const DKMatrix4& m)
{
	return Transform(m);
}

DKVector4& DKVector4::operator /= (float f)
{
	float inv = 1.0f / f;
	x *= inv;
	y *= inv;
	z *= inv;
	w *= inv;
	return *this;
}

DKVector4& DKVector4::operator /= (const DKVector4& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
	return *this;
}

float DKVector4::Length(void) const
{
	return sqrt( x*x + y*y + z*z + w*w );
}

float DKVector4::LengthSq(void) const
{
	return x*x + y*y + z*z + w*w;
}

DKVector4& DKVector4::Normalize(void)
{
	float lengthSq = x*x + y*y + z*z + w*w;;
	if (lengthSq > 0.0)
	{
		float lenInv = 1.0f / sqrt(lengthSq);
		this->x *= lenInv;
		this->y *= lenInv;
		this->z *= lenInv;
		this->w *= lenInv;
	}
	return *this;
}

DKVector4& DKVector4::Transform(const DKMatrix4& m)
{
	DKVector4 vec(x, y, z, w);
	this->x = (vec.x * m.m[0][0]) + (vec.y * m.m[1][0]) + (vec.z * m.m[2][0]) + (vec.w * m.m[3][0]);
	this->y = (vec.x * m.m[0][1]) + (vec.y * m.m[1][1]) + (vec.z * m.m[2][1]) + (vec.w * m.m[3][1]);
	this->z = (vec.x * m.m[0][2]) + (vec.y * m.m[1][2]) + (vec.z * m.m[2][2]) + (vec.w * m.m[3][2]);
	this->w = (vec.x * m.m[0][3]) + (vec.y * m.m[1][3]) + (vec.z * m.m[2][3]) + (vec.w * m.m[3][3]);
	return *this;
}
