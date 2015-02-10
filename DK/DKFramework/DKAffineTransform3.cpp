//
//  File: DKAffineTransform3.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKQuaternion.h"
#include "DKAffineTransform3.h"
#include "DKLinearTransform3.h"

#ifdef _WIN32
#define copysign _copysign
#endif

using namespace DKFoundation;
using namespace DKFramework;

DKAffineTransform3::DKAffineTransform3(const DKVector3& trans)
	: matrix3(DKMatrix3::identity)
	, translation(trans)
{
}

DKAffineTransform3::DKAffineTransform3(const DKLinearTransform3& linear, const DKVector3& trans)
	: matrix3(linear.Matrix3())
	, translation(trans)
{
}

DKAffineTransform3::DKAffineTransform3(const DKVector3& l, const DKVector3& u, const DKVector3& f, const DKVector3& t)
	: matrix3(l, u, f)
	, translation(t)
{
}

DKAffineTransform3::DKAffineTransform3(float x, float y, float z)
	: matrix3(DKMatrix3::identity)
	, translation(x, y, z)
{
}

DKAffineTransform3::DKAffineTransform3(const DKMatrix4& m)
	: matrix3(
	m.m[0][0], m.m[0][1], m.m[0][2],
	m.m[1][0], m.m[1][1], m.m[1][2],
	m.m[2][0], m.m[2][1], m.m[2][2])
	, translation(m.m[3][0], m.m[3][1], m.m[3][2])
{
}

DKAffineTransform3& DKAffineTransform3::Identity(void)
{
	matrix3.Identity();
	translation.x = 0.0f;
	translation.y = 0.0f;
	translation.z = 0.0f;
	return *this;
}

bool DKAffineTransform3::IsIdentity(void) const
{
	if (translation.x == 0.0f && translation.y == 0.0f && translation.z == 0.0f)
		return matrix3.IsIdentity();
	return false;
}

bool DKAffineTransform3::IsDiagonal(void) const
{
	if (translation.x == 0.0f && translation.y == 0.0f && translation.z == 0.0f)
		return matrix3.IsDiagonal();
	return false;
}

DKAffineTransform3& DKAffineTransform3::Translate(float x, float y, float z)
{
	/*
	| 1 0 0 0 |
	| 0 1 0 0 |
	| 0 0 1 0 |
	| X Y Z 1 |
	*/
	translation.x += x;
	translation.y += y;
	translation.z += z;
	return *this;
}

DKAffineTransform3& DKAffineTransform3::Translate(const DKVector3& v)
{
	return Translate(v.x, v.y, v.z);
}

DKAffineTransform3 DKAffineTransform3::operator * (const DKLinearTransform3& t) const
{
	return DKAffineTransform3(*this).Multiply(t);
}

DKAffineTransform3 DKAffineTransform3::operator * (const DKAffineTransform3& t) const
{
	return DKAffineTransform3(*this).Multiply(t);
}

DKAffineTransform3& DKAffineTransform3::operator *= (const DKLinearTransform3& t)
{
	return Multiply(t);
}

DKAffineTransform3& DKAffineTransform3::operator *= (const DKAffineTransform3& t)
{
	return Multiply(t);
}

bool DKAffineTransform3::operator == (const DKAffineTransform3& t) const
{
	if (translation == t.translation)
		return matrix3 == t.matrix3;
	return false;
}

bool DKAffineTransform3::operator != (const DKAffineTransform3& t) const
{
	if (translation != t.translation)
		return true;
	return matrix3 != t.matrix3;
}

DKAffineTransform3& DKAffineTransform3::Inverse(void)
{
	matrix3.Inverse();
	translation = -translation * matrix3;
	return *this;
}

DKAffineTransform3& DKAffineTransform3::Multiply(const DKLinearTransform3& t)
{
	translation *= t.matrix3;
	matrix3 *= t.matrix3;
	return *this;
}

DKAffineTransform3& DKAffineTransform3::Multiply(const DKAffineTransform3& t)
{
	translation = translation * t.matrix3 + t.translation;
	matrix3 *= t.matrix3;
	return *this;
}

DKMatrix4 DKAffineTransform3::Matrix4(void) const
{
	return DKMatrix4( 
		matrix3.m[0][0], matrix3.m[0][1], matrix3.m[0][2], 0.0f, 
		matrix3.m[1][0], matrix3.m[1][1], matrix3.m[1][2], 0.0f, 
		matrix3.m[2][0], matrix3.m[2][1], matrix3.m[2][2], 0.0f, 
		translation.x, translation.y, translation.z, 1.0f);
}
