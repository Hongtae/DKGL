//
//  File: DKAffineTransform2.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVector2.h"
#include "DKVector3.h"
#include "DKAffineTransform2.h"
#include "DKLinearTransform2.h"

#ifdef _WIN32
#define copysign _copysign
#endif

using namespace DKFoundation;
using namespace DKFramework;

DKAffineTransform2::DKAffineTransform2(const DKVector2& trans)
	: matrix2(DKMatrix2::identity)
	, translation(trans)
{
}

DKAffineTransform2::DKAffineTransform2(const DKLinearTransform2& linear, const DKVector2& trans)
	: matrix2(linear.Matrix2())
	, translation(trans)
{
}

DKAffineTransform2::DKAffineTransform2(const DKVector2& x, const DKVector2& y, const DKVector2& t)
	: matrix2(x, y)
	, translation(t)
{
}

DKAffineTransform2::DKAffineTransform2(float x, float y)
	: matrix2(DKMatrix2::identity)
	, translation(x, y)
{
}

DKAffineTransform2::DKAffineTransform2(const DKMatrix3& m)
	: matrix2(m.m[0][0], m.m[0][1], m.m[1][0], m.m[1][1])
	, translation(m.m[2][0], m.m[2][1])
{
}

DKAffineTransform2& DKAffineTransform2::Identity(void)
{
	matrix2.Identity();
	translation.x = 0.0f;
	translation.y = 0.0f;
	return *this;
}

bool DKAffineTransform2::IsIdentity(void) const
{
	if (translation.x == 0.0f && translation.y == 0.0f)
		return matrix2.IsIdentity();
	return false;
}

bool DKAffineTransform2::IsDiagonal(void) const
{
	if (translation.x == 0.0f && translation.y == 0.0f)
		return matrix2.IsDiagonal();
	return false;
}

DKAffineTransform2& DKAffineTransform2::Translate(float x, float y)
{
	/*
	Translate
	|1  0  0|
	|0  1  0|
	|X  Y  1|
	*/
	translation.x += x;
	translation.y += y;
	return *this;
}

DKAffineTransform2& DKAffineTransform2::Translate(const DKVector2& v)
{
	return Translate(v.x, v.y);
}

DKAffineTransform2& DKAffineTransform2::Inverse(void)
{
	matrix2.Inverse();
	translation = -translation * matrix2;
	return *this;
}

DKAffineTransform2& DKAffineTransform2::Multiply(const DKLinearTransform2& t)
{
	translation *= t.matrix2;
	matrix2 *= t.matrix2;
	return *this;
}

DKAffineTransform2& DKAffineTransform2::Multiply(const DKAffineTransform2& t)
{
	translation = translation * t.matrix2 + t.translation;
	matrix2 *= t.matrix2;
	return *this;
}

DKAffineTransform2 DKAffineTransform2::operator * (const DKLinearTransform2& t) const
{
	return DKAffineTransform2(*this).Multiply(t);
}

DKAffineTransform2 DKAffineTransform2::operator * (const DKAffineTransform2& t) const
{
	return DKAffineTransform2(*this).Multiply(t);
}

DKAffineTransform2& DKAffineTransform2::operator *= (const DKLinearTransform2& t)
{
	return Multiply(t);
}

DKAffineTransform2& DKAffineTransform2::operator *= (const DKAffineTransform2& t)
{
	return Multiply(t);
}

bool DKAffineTransform2::operator == (const DKAffineTransform2& t) const
{
	if (translation == t.translation)
		return matrix2 == t.matrix2;
	return false;
}

bool DKAffineTransform2::operator != (const DKAffineTransform2& t) const
{
	if (translation != t.translation)
		return true;
	return matrix2 != t.matrix2;
}

DKMatrix3 DKAffineTransform2::Matrix3(void) const
{
	return DKMatrix3( 
		matrix2.m[0][0], matrix2.m[0][1], 0.0f, 
		matrix2.m[1][0], matrix2.m[1][1], 0.0f, 
		translation.x, translation.y, 1.0f); 
}
