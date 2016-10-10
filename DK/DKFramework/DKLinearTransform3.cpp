//
//  File: DKLinearTransform3.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKQuaternion.h"
#include "DKLinearTransform3.h"

#ifdef _WIN32
#define copysign _copysign
#endif

using namespace DKGL;
using namespace DKGL;

DKLinearTransform3::DKLinearTransform3(void)
	: matrix3(DKMatrix3::identity)
{
}

DKLinearTransform3::DKLinearTransform3(const DKQuaternion& q)
	: matrix3(q.Matrix3())
{
}

DKLinearTransform3::DKLinearTransform3(float scaleX, float scaleY, float scaleZ)
	: matrix3(
	scaleX, 0.0f, 0.0f,
	0.0f, scaleY, 0.0f,
	0.0f, 0.0f, scaleZ)
{
}

DKLinearTransform3::DKLinearTransform3(const DKVector3& left, const DKVector3& up, const DKVector3& forward)
	: matrix3(left, up, forward)
{
}

DKLinearTransform3::DKLinearTransform3(const DKMatrix3& m)
	: matrix3(m)
{
}

DKLinearTransform3& DKLinearTransform3::Identity(void)
{
	matrix3.Identity();
	return *this;
}

bool DKLinearTransform3::IsIdentity(void) const
{
	return matrix3.IsIdentity();
}

bool DKLinearTransform3::IsDiagonal(void) const
{
	return matrix3.IsDiagonal();
}

DKLinearTransform3& DKLinearTransform3::Scale(float x, float y, float z)
{
	/*
	| X 0 0 |
	| 0 Y 0 |
	| 0 0 Z |
	*/
	matrix3.m[0][0] *= x;
	matrix3.m[0][1] *= y;
	matrix3.m[0][2] *= z;
	matrix3.m[1][0] *= x;
	matrix3.m[1][1] *= y;
	matrix3.m[1][2] *= z;
	matrix3.m[2][0] *= x;
	matrix3.m[2][1] *= y;
	matrix3.m[2][2] *= z;
	return *this;
}

DKLinearTransform3& DKLinearTransform3::Scale(const DKVector3& v)
{
	return Scale(v.x, v.y, v.z);
}

DKLinearTransform3& DKLinearTransform3::Scale(float s)
{
	return Scale(s,s,s);
}

DKLinearTransform3& DKLinearTransform3::RotateX(float r)
{
	/*
	X - Axis:
	|1  0    0   |
	|0  cos  sin |
	|0 -sin  cos |
	*/
	float cosR = cos(r);
	float sinR = sin(r);

	DKMatrix3 mat;
	mat.Identity();
	mat.m[1][1] = cosR;
	mat.m[1][2] = sinR;
	mat.m[2][1] = -sinR;
	mat.m[2][2] = cosR;
	matrix3.Multiply(mat);
	return *this;
}

DKLinearTransform3& DKLinearTransform3::RotateY(float r)
{
	/*
	Y - Axis:
	|cos  0 -sin |
	|0    1  0   |
	|sin  0  cos |
	*/
	float cosR = cos(r);
	float sinR = sin(r);

	DKMatrix3 mat;
	mat.Identity();
	mat.m[0][0] = cosR;
	mat.m[0][2] = -sinR;
	mat.m[2][0] = sinR;
	mat.m[2][2] = cosR;
	matrix3.Multiply(mat);
	return *this;
}

DKLinearTransform3& DKLinearTransform3::RotateZ(float r)
{
	/*
	Z - Axis:
	|cos  sin 0  |
	|-sin cos 0  |
	|0    0   1  |
	*/
	float cosR = cos(r);
	float sinR = sin(r);

	DKMatrix3 mat;
	mat.Identity();
	mat.m[0][0] = cosR;
	mat.m[0][1] = sinR;
	mat.m[1][0] = -sinR;
	mat.m[1][1] = cosR;
	matrix3.Multiply(mat);
	return *this;
}

DKLinearTransform3& DKLinearTransform3::Rotate(const DKVector3& axis, float r)
{
	if (r == 0)
		return *this;
	/*
	DKVector3 au = axis;		// au is unit vector.
	au.Normalize();

	float fCosR = cos(r);
	float fSinR = sin(r);

	DKLinearTransform3 mat;
	mat.Identity();
	mat.m[0][0] = fCosR + au.x * au.x * (1-fCosR);
	mat.m[0][1] = au.x * au.y * (1-fCosR) - au.z * fSinR;
	mat.m[0][2] = au.x * au.z * (1-fCosR) + au.y * fSinR;
	mat.m[0][3] = 0;
	mat.m[1][0] = au.y * au.x * (1-fCosR) + au.z * fSinR;
	mat.m[1][1] = fCosR + au.y * au.y * (1-fCosR);
	mat.m[1][2] = au.y * au.z * (1-fCosR) - au.x * fSinR;
	mat.m[1][3] = 0;
	mat.m[2][0] = au.z * au.x * (1-fCosR) - au.y * fSinR;
	mat.m[2][1] = au.z * au.y * (1-fCosR) + au.x * fSinR;
	mat.m[2][2] = fCosR + au.z * au.z * (1-fCosR);
	mat.m[2][3] = 0;
	mat.m[3][0] = 0;
	mat.m[3][1] = 0;
	mat.m[3][3] = 1;

	return Multiply(mat);
	*/
	matrix3.Multiply(DKQuaternion(axis, r).Matrix3());
	return *this;
}

DKLinearTransform3& DKLinearTransform3::Rotate(const DKQuaternion& q)
{
	matrix3.Multiply(q.Matrix3());
	return *this;
}

DKLinearTransform3 DKLinearTransform3::operator * (const DKLinearTransform3& t) const
{
	return DKLinearTransform3(matrix3 * t.matrix3);
}

DKLinearTransform3& DKLinearTransform3::operator *= (const DKLinearTransform3& t)
{
	matrix3 *= t.matrix3;
	return *this;
}

bool DKLinearTransform3::operator == (const DKLinearTransform3& t) const
{
	return matrix3 == t.matrix3;
}

bool DKLinearTransform3::operator != (const DKLinearTransform3& t) const
{
	return matrix3 != t.matrix3;
}

DKQuaternion DKLinearTransform3::Rotation(void) const
{
	float x = sqrt( Max( float(0), 1 + matrix3.m[0][0] - matrix3.m[1][1] - matrix3.m[2][2] ) ) / 2;
	float y = sqrt( Max( float(0), 1 - matrix3.m[0][0] + matrix3.m[1][1] - matrix3.m[2][2] ) ) / 2;
	float z = sqrt( Max( float(0), 1 - matrix3.m[0][0] - matrix3.m[1][1] + matrix3.m[2][2] ) ) / 2;
	float w = sqrt( Max( float(0), 1 + matrix3.m[0][0] + matrix3.m[1][1] + matrix3.m[2][2] ) ) / 2;
	x = copysign( x, matrix3.m[1][2] - matrix3.m[2][1] );
	y = copysign( y, matrix3.m[2][0] - matrix3.m[0][2] );
	z = copysign( z, matrix3.m[0][1] - matrix3.m[1][0] );

	return DKQuaternion(x, y, z, w);
}

bool DKLinearTransform3::Decompose(DKVector3& scale, DKQuaternion& rotate) const
{
	DKVector3 vScale = DKVector3(
		DKVector3(matrix3.m[0][0], matrix3.m[0][1], matrix3.m[0][2]).Length(),
		DKVector3(matrix3.m[1][0], matrix3.m[1][1], matrix3.m[1][2]).Length(),
		DKVector3(matrix3.m[2][0], matrix3.m[2][1], matrix3.m[2][2]).Length());

	if (vScale.x == 0 || vScale.y == 0 || vScale.z == 0)
		return false;

	scale = vScale;

	DKMatrix3 normalized;
	normalized.Zero();

	normalized.m[0][0] = matrix3.m[0][0]/scale.x; 
	normalized.m[0][1] = matrix3.m[0][1]/scale.x; 
	normalized.m[0][2] = matrix3.m[0][2]/scale.x; 
	normalized.m[1][0] = matrix3.m[1][0]/scale.y; 
	normalized.m[1][1] = matrix3.m[1][1]/scale.y; 
	normalized.m[1][2] = matrix3.m[1][2]/scale.y; 
	normalized.m[2][0] = matrix3.m[2][0]/scale.z; 
	normalized.m[2][1] = matrix3.m[2][1]/scale.z; 
	normalized.m[2][2] = matrix3.m[2][2]/scale.z; 

	rotate = DKLinearTransform3(normalized).Rotation();

	return true;
}

DKLinearTransform3& DKLinearTransform3::Inverse(void)
{
	matrix3.Inverse();
	return *this;
}

DKLinearTransform3& DKLinearTransform3::Multiply(const DKMatrix3& t)
{
	matrix3.Multiply(t);
	return *this;
}

DKLinearTransform3& DKLinearTransform3::Multiply(const DKLinearTransform3& t)
{
	matrix3.Multiply(t.matrix3);
	return *this;
}
