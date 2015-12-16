//
//  File: DKVector2.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVector2.h"
#include "DKMatrix2.h"
#include "DKMatrix3.h"

using namespace DKFoundation;
using namespace DKFramework;

const DKVector2 DKVector2::zero = DKVector2(0,0);

DKVector2::DKVector2(void)
	: x(0), y(0)
{
}

DKVector2::DKVector2(float x_, float y_)
	: x(x_), y(y_)
{
}

float DKVector2::Dot(const DKVector2& v1, const DKVector2& v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y);
}

bool DKVector2::operator == (const DKVector2& v) const
{
	return x == v.x && y == v.y;
}

bool DKVector2::operator != (const DKVector2& v) const
{
	return x != v.x || y != v.y;
}

DKVector2 DKVector2::operator + (const DKVector2& v) const
{
	return DKVector2(x + v.x, y + v.y);
}

DKVector2 DKVector2::operator - (const DKVector2& v) const
{
	return DKVector2(x - v.x, y - v.y);
}

DKVector2 DKVector2::operator - (void) const
{
	return DKVector2(-x, -y);
}

DKVector2 DKVector2::operator * (float f) const
{
	return DKVector2(x * f, y * f);
}

DKVector2 DKVector2::operator * (const DKVector2& v) const
{
	return DKVector2(x * v.x, y * v.y);
}

DKVector2 DKVector2::operator * (const DKMatrix2& m) const
{
	return DKVector2(x, y).Transform(m);
}

DKVector2 DKVector2::operator * (const DKMatrix3& m) const
{
	return DKVector2(x, y).Transform(m);
}

DKVector2 DKVector2::operator / (float f) const
{
	float inv = 1.0f / f;
	return DKVector2(x * inv, y * inv);
}

DKVector2 DKVector2::operator / (const DKVector2& v) const
{
	return DKVector2(x / v.x, y / v.y);
}

DKVector2& DKVector2::operator += (const DKVector2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

DKVector2& DKVector2::operator -= (const DKVector2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

DKVector2& DKVector2::operator *= (float f)
{
	x *= f;
	y *= f;
	return *this;
}

DKVector2& DKVector2::operator *= (const DKVector2& v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}

DKVector2& DKVector2::operator *= (const DKMatrix2& m)
{
	return Transform(m);
}

DKVector2& DKVector2::operator *= (const DKMatrix3& m)
{
	return Transform(m);
}

DKVector2& DKVector2::operator /= (float f)
{
	float inv = 1.0f / f;
	x *= inv;
	y *= inv;
	return *this;
}

DKVector2& DKVector2::operator /= (const DKVector2& v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}

float DKVector2::Length(void) const
{
	return sqrt( x*x + y*y );
}

float DKVector2::LengthSq(void) const
{
	return x*x + y*y;
}

DKVector2& DKVector2::Transform(const DKMatrix2& m)
{
	DKVector2 vec(x, y);
	this->x = (vec.x * m.m[0][0]) + (vec.y * m.m[1][0]);
	this->y = (vec.x * m.m[0][1]) + (vec.y * m.m[1][1]);
	return *this;
}

DKVector2& DKVector2::Transform(const DKMatrix3& m)
{
	DKVector2 vec(x, y);
	this->x = (vec.x * m.m[0][0]) + (vec.y * m.m[1][0]) + m.m[2][0];
	this->y = (vec.x * m.m[0][1]) + (vec.y * m.m[1][1]) + m.m[2][1];
	float w = 1.0f / ((vec.x * m.m[0][2]) + (vec.y * m.m[1][2]) + m.m[2][2]);

	this->x *= w;
	this->y *= w;
	return *this;
}

DKVector2& DKVector2::Normalize(void)
{
	float lengthSq = x*x + y*y;
	if (lengthSq > 0.0)
	{
		float lenInv = 1.0f / sqrt(lengthSq);
		this->x *= lenInv;
		this->y *= lenInv;
	}
	return *this;
}

