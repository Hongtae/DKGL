//
//  File: DKLinearTransform2.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVector2.h"
#include "DKVector3.h"
#include "DKLinearTransform2.h"

using namespace DKFramework;

DKLinearTransform2::DKLinearTransform2()
	: matrix2(DKMatrix2::identity)
{
}

DKLinearTransform2::DKLinearTransform2(float rotate)
	: matrix2(DKMatrix2::identity)
{
	Rotate(rotate);
}

DKLinearTransform2::DKLinearTransform2(float scaleX, float scaleY)
	: matrix2(scaleX, 0.0f, 0.0f, scaleY)
{
}

DKLinearTransform2::DKLinearTransform2(const DKVector2& axisX, const DKVector2& axisY)
	: matrix2(axisX, axisY)
{
}

DKLinearTransform2::DKLinearTransform2(const DKMatrix2& m)
	: matrix2(m)
{
}

DKLinearTransform2& DKLinearTransform2::Identity()
{
	matrix2.SetIdentity();
	return *this;
}

bool DKLinearTransform2::IsIdentity() const
{
	return matrix2.IsIdentity();
}

bool DKLinearTransform2::IsDiagonal() const
{
	return matrix2.IsDiagonal();
}

DKLinearTransform2& DKLinearTransform2::HorizontalShear(float s)
{
	/*
	Horizontal Shear
	|1  S|
	|0  1|
	*/
	matrix2.m[0][1] += matrix2.m[0][0] * s;
	matrix2.m[1][1] += matrix2.m[1][0] * s;
	return *this;
}

DKLinearTransform2& DKLinearTransform2::VerticalShear(float s)
{
	/*
	Vertical Shear
	|1  0|
	|S  1|
	*/
	matrix2.m[0][0] += matrix2.m[0][1] * s;
	matrix2.m[1][0] += matrix2.m[1][1] * s;
	return *this;
}

DKLinearTransform2& DKLinearTransform2::HorizontalFlip()
{
	/*
	Horizontal flip
	|-1  0|
	| 0  1|
	*/
	matrix2.m[0][0] *= -1.0f;
	matrix2.m[1][0] *= -1.0f;
	return *this;
}

DKLinearTransform2& DKLinearTransform2::VerticalFlip()
{
	/*
	Vertical flip
	|1  0|
	|0 -1|
	*/
	matrix2.m[0][1] *= -1.0f;
	matrix2.m[1][1] *= -1.0f;
	return *this;
}

DKLinearTransform2& DKLinearTransform2::Squeeze(float s)
{
	/*
	Squeeze
	|S  0  |
	|0  1/S|
	*/
	float s2 = 1.0f/s;
	matrix2.m[0][0] *= s;
	matrix2.m[0][1] *= s2;
	matrix2.m[1][0] *= s;
	matrix2.m[1][1] *= s2;
	return *this;
}

DKLinearTransform2& DKLinearTransform2::Scale(float x, float y)
{
	/*
	Scale
	|X  0|
	|0  Y|
	*/
	matrix2.m[0][0] *= x;
	matrix2.m[0][1] *= y;
	matrix2.m[1][0] *= x;
	matrix2.m[1][1] *= y;
	return *this;
}

DKLinearTransform2& DKLinearTransform2::Scale(const DKVector2& v)
{
	return Scale(v.x, v.y);
}

DKLinearTransform2& DKLinearTransform2::Scale(float s)
{
	return Scale(s, s);
}

DKLinearTransform2& DKLinearTransform2::Rotate(float angle)
{
	/*
	Rotate
	| cos  sin|
	|-sin  cos|
	*/
	float cosR = cos(angle);
	float sinR = sin(angle);

	DKMatrix2 mat(cosR, sinR, -sinR, cosR);
	matrix2.Multiply(mat);
	return *this;
}

DKLinearTransform2& DKLinearTransform2::Inverse()
{
	matrix2.Inverse();
	return *this;
}

DKLinearTransform2& DKLinearTransform2::Multiply(const DKMatrix2& t)
{
	matrix2.Multiply(t);
	return *this;
}

DKLinearTransform2& DKLinearTransform2::Multiply(const DKLinearTransform2& t)
{
	matrix2.Multiply(t.matrix2);
	return *this;
}

DKLinearTransform2 DKLinearTransform2::operator * (const DKLinearTransform2& t) const
{
	return DKLinearTransform2(matrix2 * t.matrix2);
}

DKLinearTransform2& DKLinearTransform2::operator *= (const DKLinearTransform2& t)
{
	matrix2 *= t.matrix2;
	return *this;
}

bool DKLinearTransform2::operator == (const DKLinearTransform2& t) const
{
	return matrix2 == t.matrix2;
}

bool DKLinearTransform2::operator != (const DKLinearTransform2& t) const
{
	return matrix2 != t.matrix2;
}
