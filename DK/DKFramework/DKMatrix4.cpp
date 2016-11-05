//
//  File: DKMatrix4.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKMatrix4.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKQuaternion.h"

using namespace DKGL;

const DKMatrix4 DKMatrix4::identity = DKMatrix4().Identity();

DKMatrix4::DKMatrix4(void)
	: _11(1), _12(0), _13(0), _14(0)
	, _21(0), _22(1), _23(0), _24(0)
	, _31(0), _32(0), _33(1), _34(0)
	, _41(0), _42(0), _43(0), _44(1)
{
}

DKMatrix4::DKMatrix4(const DKVector4& row1, const DKVector4& row2, const DKVector4& row3, const DKVector4& row4)
	: _11(row1.x), _12(row1.y), _13(row1.z), _14(row1.w)
	, _21(row2.x), _22(row2.y), _23(row2.z), _24(row2.w)
	, _31(row3.x), _32(row3.y), _33(row3.z), _34(row3.w)
	, _41(row4.x), _42(row4.y), _43(row4.z), _44(row4.w)
{
}

DKMatrix4::DKMatrix4(float e11, float e12, float e13, float e14,
	float e21, float e22, float e23, float e24,
	float e31, float e32, float e33, float e34,
	float e41, float e42, float e43, float e44)
	: _11(e11), _12(e12), _13(e13), _14(e14)
	, _21(e21), _22(e22), _23(e23), _24(e24)
	, _31(e31), _32(e32), _33(e33), _34(e34)
	, _41(e41), _42(e42), _43(e43), _44(e44)
{
}

DKMatrix4& DKMatrix4::Zero(void)
{
	m[0][0] = m[0][1] = m[0][2] = m[0][3] = 0.0f;
	m[1][0] = m[1][1] = m[1][2] = m[1][3] = 0.0f;
	m[2][0] = m[2][1] = m[2][2] = m[2][3] = 0.0f;
	m[3][0] = m[3][1] = m[3][2] = m[3][3] = 0.0f;
	return *this;
}

DKMatrix4& DKMatrix4::Identity(void)
{
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	m[0][1] = m[0][2] = m[0][3] = m[1][0] = 0.0f;
	m[1][2] = m[1][3] = m[2][0] = m[2][1] = 0.0f;
	m[2][3] = m[3][0] = m[3][1] = m[3][2] = 0.0f;
	return *this;
}

bool DKMatrix4::IsIdentity(void) const
{
	return
		m[0][0] == 1.0f && m[0][1] == 0.0f && m[0][2] == 0.0f && m[0][3] == 0.0f &&
		m[1][0] == 0.0f && m[1][1] == 1.0f && m[1][2] == 0.0f && m[1][3] == 0.0f &&
		m[2][0] == 0.0f && m[2][1] == 0.0f && m[2][2] == 1.0f && m[2][3] == 0.0f &&
		m[3][0] == 0.0f && m[3][1] == 0.0f && m[3][2] == 0.0f && m[3][3] == 1.0f;
}

bool DKMatrix4::IsDiagonal(void) const
{
	return
		m[0][1] == 0.0f && m[0][2] == 0.0f && m[0][3] == 0.0f &&
		m[1][0] == 0.0f && m[1][2] == 0.0f && m[1][3] == 0.0f &&
		m[2][0] == 0.0f && m[2][1] == 0.0f && m[2][3] == 0.0f &&
		m[3][0] == 0.0f && m[3][1] == 0.0f && m[3][2] == 0.0f;
}

DKMatrix4 DKMatrix4::operator * (const DKMatrix4& m) const
{
	DKMatrix4 mat;
	mat.m[0][0] = (this->m[0][0] * m.m[0][0]) + (this->m[0][1] * m.m[1][0]) + (this->m[0][2] * m.m[2][0]) + (this->m[0][3] * m.m[3][0]);
	mat.m[0][1] = (this->m[0][0] * m.m[0][1]) + (this->m[0][1] * m.m[1][1]) + (this->m[0][2] * m.m[2][1]) + (this->m[0][3] * m.m[3][1]);
	mat.m[0][2] = (this->m[0][0] * m.m[0][2]) + (this->m[0][1] * m.m[1][2]) + (this->m[0][2] * m.m[2][2]) + (this->m[0][3] * m.m[3][2]);
	mat.m[0][3] = (this->m[0][0] * m.m[0][3]) + (this->m[0][1] * m.m[1][3]) + (this->m[0][2] * m.m[2][3]) + (this->m[0][3] * m.m[3][3]);
	mat.m[1][0] = (this->m[1][0] * m.m[0][0]) + (this->m[1][1] * m.m[1][0]) + (this->m[1][2] * m.m[2][0]) + (this->m[1][3] * m.m[3][0]);
	mat.m[1][1] = (this->m[1][0] * m.m[0][1]) + (this->m[1][1] * m.m[1][1]) + (this->m[1][2] * m.m[2][1]) + (this->m[1][3] * m.m[3][1]);
	mat.m[1][2] = (this->m[1][0] * m.m[0][2]) + (this->m[1][1] * m.m[1][2]) + (this->m[1][2] * m.m[2][2]) + (this->m[1][3] * m.m[3][2]);
	mat.m[1][3] = (this->m[1][0] * m.m[0][3]) + (this->m[1][1] * m.m[1][3]) + (this->m[1][2] * m.m[2][3]) + (this->m[1][3] * m.m[3][3]);
	mat.m[2][0] = (this->m[2][0] * m.m[0][0]) + (this->m[2][1] * m.m[1][0]) + (this->m[2][2] * m.m[2][0]) + (this->m[2][3] * m.m[3][0]);
	mat.m[2][1] = (this->m[2][0] * m.m[0][1]) + (this->m[2][1] * m.m[1][1]) + (this->m[2][2] * m.m[2][1]) + (this->m[2][3] * m.m[3][1]);
	mat.m[2][2] = (this->m[2][0] * m.m[0][2]) + (this->m[2][1] * m.m[1][2]) + (this->m[2][2] * m.m[2][2]) + (this->m[2][3] * m.m[3][2]);
	mat.m[2][3] = (this->m[2][0] * m.m[0][3]) + (this->m[2][1] * m.m[1][3]) + (this->m[2][2] * m.m[2][3]) + (this->m[2][3] * m.m[3][3]);
	mat.m[3][0] = (this->m[3][0] * m.m[0][0]) + (this->m[3][1] * m.m[1][0]) + (this->m[3][2] * m.m[2][0]) + (this->m[3][3] * m.m[3][0]);
	mat.m[3][1] = (this->m[3][0] * m.m[0][1]) + (this->m[3][1] * m.m[1][1]) + (this->m[3][2] * m.m[2][1]) + (this->m[3][3] * m.m[3][1]);
	mat.m[3][2] = (this->m[3][0] * m.m[0][2]) + (this->m[3][1] * m.m[1][2]) + (this->m[3][2] * m.m[2][2]) + (this->m[3][3] * m.m[3][2]);
	mat.m[3][3] = (this->m[3][0] * m.m[0][3]) + (this->m[3][1] * m.m[1][3]) + (this->m[3][2] * m.m[2][3]) + (this->m[3][3] * m.m[3][3]);
	return mat;
}

DKMatrix4 DKMatrix4::operator + (const DKMatrix4& m) const
{
	DKMatrix4 mat;
	mat.val[0x0] = this->val[0x0] + m.val[0x0];
	mat.val[0x1] = this->val[0x1] + m.val[0x1];
	mat.val[0x2] = this->val[0x2] + m.val[0x2];
	mat.val[0x3] = this->val[0x3] + m.val[0x3];
	mat.val[0x4] = this->val[0x4] + m.val[0x4];
	mat.val[0x5] = this->val[0x5] + m.val[0x5];
	mat.val[0x6] = this->val[0x6] + m.val[0x6];
	mat.val[0x7] = this->val[0x7] + m.val[0x7];
	mat.val[0x8] = this->val[0x8] + m.val[0x8];
	mat.val[0x9] = this->val[0x9] + m.val[0x9];
	mat.val[0xa] = this->val[0xa] + m.val[0xa];
	mat.val[0xb] = this->val[0xb] + m.val[0xb];
	mat.val[0xc] = this->val[0xc] + m.val[0xc];
	mat.val[0xd] = this->val[0xd] + m.val[0xd];
	mat.val[0xe] = this->val[0xe] + m.val[0xe];
	mat.val[0xf] = this->val[0xf] + m.val[0xf];
	return mat;
}

DKMatrix4 DKMatrix4::operator - (const DKMatrix4& m) const
{
	DKMatrix4 mat;
	mat.val[0x0] = this->val[0x0] - m.val[0x0];
	mat.val[0x1] = this->val[0x1] - m.val[0x1];
	mat.val[0x2] = this->val[0x2] - m.val[0x2];
	mat.val[0x3] = this->val[0x3] - m.val[0x3];
	mat.val[0x4] = this->val[0x4] - m.val[0x4];
	mat.val[0x5] = this->val[0x5] - m.val[0x5];
	mat.val[0x6] = this->val[0x6] - m.val[0x6];
	mat.val[0x7] = this->val[0x7] - m.val[0x7];
	mat.val[0x8] = this->val[0x8] - m.val[0x8];
	mat.val[0x9] = this->val[0x9] - m.val[0x9];
	mat.val[0xa] = this->val[0xa] - m.val[0xa];
	mat.val[0xb] = this->val[0xb] - m.val[0xb];
	mat.val[0xc] = this->val[0xc] - m.val[0xc];
	mat.val[0xd] = this->val[0xd] - m.val[0xd];
	mat.val[0xe] = this->val[0xe] - m.val[0xe];
	mat.val[0xf] = this->val[0xf] - m.val[0xf];
	return mat;
}

DKMatrix4 DKMatrix4::operator * (float f) const
{
	DKMatrix4 mat;
	mat.val[0x0] = this->val[0x0] * f;
	mat.val[0x1] = this->val[0x1] * f;
	mat.val[0x2] = this->val[0x2] * f;
	mat.val[0x3] = this->val[0x3] * f;
	mat.val[0x4] = this->val[0x4] * f;
	mat.val[0x5] = this->val[0x5] * f;
	mat.val[0x6] = this->val[0x6] * f;
	mat.val[0x7] = this->val[0x7] * f;
	mat.val[0x8] = this->val[0x8] * f;
	mat.val[0x9] = this->val[0x9] * f;
	mat.val[0xa] = this->val[0xa] * f;
	mat.val[0xb] = this->val[0xb] * f;
	mat.val[0xc] = this->val[0xc] * f;
	mat.val[0xd] = this->val[0xd] * f;
	mat.val[0xe] = this->val[0xe] * f;
	mat.val[0xf] = this->val[0xf] * f;
	return mat;
}

DKMatrix4 DKMatrix4::operator / (float f) const
{
	float inv = 1.0f / f;
	DKMatrix4 mat;
	mat.val[0x0] = this->val[0x0] * inv;
	mat.val[0x1] = this->val[0x1] * inv;
	mat.val[0x2] = this->val[0x2] * inv;
	mat.val[0x3] = this->val[0x3] * inv;
	mat.val[0x4] = this->val[0x4] * inv;
	mat.val[0x5] = this->val[0x5] * inv;
	mat.val[0x6] = this->val[0x6] * inv;
	mat.val[0x7] = this->val[0x7] * inv;
	mat.val[0x8] = this->val[0x8] * inv;
	mat.val[0x9] = this->val[0x9] * inv;
	mat.val[0xa] = this->val[0xa] * inv;
	mat.val[0xb] = this->val[0xb] * inv;
	mat.val[0xc] = this->val[0xc] * inv;
	mat.val[0xd] = this->val[0xd] * inv;
	mat.val[0xe] = this->val[0xe] * inv;
	mat.val[0xf] = this->val[0xf] * inv;
	return mat;
}

DKMatrix4& DKMatrix4::operator *= (const DKMatrix4& m)
{
	DKMatrix4 mat(*this);
	this->m[0][0] = (mat.m[0][0] * m.m[0][0]) + (mat.m[0][1] * m.m[1][0]) + (mat.m[0][2] * m.m[2][0]) + (mat.m[0][3] * m.m[3][0]);
	this->m[0][1] = (mat.m[0][0] * m.m[0][1]) + (mat.m[0][1] * m.m[1][1]) + (mat.m[0][2] * m.m[2][1]) + (mat.m[0][3] * m.m[3][1]);
	this->m[0][2] = (mat.m[0][0] * m.m[0][2]) + (mat.m[0][1] * m.m[1][2]) + (mat.m[0][2] * m.m[2][2]) + (mat.m[0][3] * m.m[3][2]);
	this->m[0][3] = (mat.m[0][0] * m.m[0][3]) + (mat.m[0][1] * m.m[1][3]) + (mat.m[0][2] * m.m[2][3]) + (mat.m[0][3] * m.m[3][3]);
	this->m[1][0] = (mat.m[1][0] * m.m[0][0]) + (mat.m[1][1] * m.m[1][0]) + (mat.m[1][2] * m.m[2][0]) + (mat.m[1][3] * m.m[3][0]);
	this->m[1][1] = (mat.m[1][0] * m.m[0][1]) + (mat.m[1][1] * m.m[1][1]) + (mat.m[1][2] * m.m[2][1]) + (mat.m[1][3] * m.m[3][1]);
	this->m[1][2] = (mat.m[1][0] * m.m[0][2]) + (mat.m[1][1] * m.m[1][2]) + (mat.m[1][2] * m.m[2][2]) + (mat.m[1][3] * m.m[3][2]);
	this->m[1][3] = (mat.m[1][0] * m.m[0][3]) + (mat.m[1][1] * m.m[1][3]) + (mat.m[1][2] * m.m[2][3]) + (mat.m[1][3] * m.m[3][3]);
	this->m[2][0] = (mat.m[2][0] * m.m[0][0]) + (mat.m[2][1] * m.m[1][0]) + (mat.m[2][2] * m.m[2][0]) + (mat.m[2][3] * m.m[3][0]);
	this->m[2][1] = (mat.m[2][0] * m.m[0][1]) + (mat.m[2][1] * m.m[1][1]) + (mat.m[2][2] * m.m[2][1]) + (mat.m[2][3] * m.m[3][1]);
	this->m[2][2] = (mat.m[2][0] * m.m[0][2]) + (mat.m[2][1] * m.m[1][2]) + (mat.m[2][2] * m.m[2][2]) + (mat.m[2][3] * m.m[3][2]);
	this->m[2][3] = (mat.m[2][0] * m.m[0][3]) + (mat.m[2][1] * m.m[1][3]) + (mat.m[2][2] * m.m[2][3]) + (mat.m[2][3] * m.m[3][3]);
	this->m[3][0] = (mat.m[3][0] * m.m[0][0]) + (mat.m[3][1] * m.m[1][0]) + (mat.m[3][2] * m.m[2][0]) + (mat.m[3][3] * m.m[3][0]);
	this->m[3][1] = (mat.m[3][0] * m.m[0][1]) + (mat.m[3][1] * m.m[1][1]) + (mat.m[3][2] * m.m[2][1]) + (mat.m[3][3] * m.m[3][1]);
	this->m[3][2] = (mat.m[3][0] * m.m[0][2]) + (mat.m[3][1] * m.m[1][2]) + (mat.m[3][2] * m.m[2][2]) + (mat.m[3][3] * m.m[3][2]);
	this->m[3][3] = (mat.m[3][0] * m.m[0][3]) + (mat.m[3][1] * m.m[1][3]) + (mat.m[3][2] * m.m[2][3]) + (mat.m[3][3] * m.m[3][3]);
	return *this;
}

DKMatrix4& DKMatrix4::operator += (const DKMatrix4& m)
{
	this->val[0x0] += m.val[0x0];
	this->val[0x1] += m.val[0x1];
	this->val[0x2] += m.val[0x2];
	this->val[0x3] += m.val[0x3];
	this->val[0x4] += m.val[0x4];
	this->val[0x5] += m.val[0x5];
	this->val[0x6] += m.val[0x6];
	this->val[0x7] += m.val[0x7];
	this->val[0x8] += m.val[0x8];
	this->val[0x9] += m.val[0x9];
	this->val[0xa] += m.val[0xa];
	this->val[0xb] += m.val[0xb];
	this->val[0xc] += m.val[0xc];
	this->val[0xd] += m.val[0xd];
	this->val[0xe] += m.val[0xe];
	this->val[0xf] += m.val[0xf];
	return *this;
}

DKMatrix4& DKMatrix4::operator -= (const DKMatrix4& m)
{
	this->val[0x0] -= m.val[0x0];
	this->val[0x1] -= m.val[0x1];
	this->val[0x2] -= m.val[0x2];
	this->val[0x3] -= m.val[0x3];
	this->val[0x4] -= m.val[0x4];
	this->val[0x5] -= m.val[0x5];
	this->val[0x6] -= m.val[0x6];
	this->val[0x7] -= m.val[0x7];
	this->val[0x8] -= m.val[0x8];
	this->val[0x9] -= m.val[0x9];
	this->val[0xa] -= m.val[0xa];
	this->val[0xb] -= m.val[0xb];
	this->val[0xc] -= m.val[0xc];
	this->val[0xd] -= m.val[0xd];
	this->val[0xe] -= m.val[0xe];
	this->val[0xf] -= m.val[0xf];
	return *this;
}

DKMatrix4& DKMatrix4::operator *= (float f)
{
	this->val[0x0] *= f;
	this->val[0x1] *= f;
	this->val[0x2] *= f;
	this->val[0x3] *= f;
	this->val[0x4] *= f;
	this->val[0x5] *= f;
	this->val[0x6] *= f;
	this->val[0x7] *= f;
	this->val[0x8] *= f;
	this->val[0x9] *= f;
	this->val[0xa] *= f;
	this->val[0xb] *= f;
	this->val[0xc] *= f;
	this->val[0xd] *= f;
	this->val[0xe] *= f;
	this->val[0xf] *= f;
	return *this;
}

DKMatrix4& DKMatrix4::operator /= (float f)
{
	float inv = 1.0f / f;
	this->val[0x0] *= inv;
	this->val[0x1] *= inv;
	this->val[0x2] *= inv;
	this->val[0x3] *= inv;
	this->val[0x4] *= inv;
	this->val[0x5] *= inv;
	this->val[0x6] *= inv;
	this->val[0x7] *= inv;
	this->val[0x8] *= inv;
	this->val[0x9] *= inv;
	this->val[0xa] *= inv;
	this->val[0xb] *= inv;
	this->val[0xc] *= inv;
	this->val[0xd] *= inv;
	this->val[0xe] *= inv;
	this->val[0xf] *= inv;
	return *this;
}

bool DKMatrix4::operator == (const DKMatrix4& m) const
{
	return
		this->val[0x0] == m.val[0x0] &&
		this->val[0x1] == m.val[0x1] &&
		this->val[0x2] == m.val[0x2] &&
		this->val[0x3] == m.val[0x3] &&
		this->val[0x4] == m.val[0x4] &&
		this->val[0x5] == m.val[0x5] &&
		this->val[0x6] == m.val[0x6] &&
		this->val[0x7] == m.val[0x7] &&
		this->val[0x8] == m.val[0x8] &&
		this->val[0x9] == m.val[0x9] &&
		this->val[0xa] == m.val[0xa] &&
		this->val[0xb] == m.val[0xb] &&
		this->val[0xc] == m.val[0xc] &&
		this->val[0xd] == m.val[0xd] &&
		this->val[0xe] == m.val[0xe] &&
		this->val[0xf] == m.val[0xf];
}

bool DKMatrix4::operator != (const DKMatrix4& m) const
{
	return
		this->val[0x0] != m.val[0x0] ||
		this->val[0x1] != m.val[0x1] ||
		this->val[0x2] != m.val[0x2] ||
		this->val[0x3] != m.val[0x3] ||
		this->val[0x4] != m.val[0x4] ||
		this->val[0x5] != m.val[0x5] ||
		this->val[0x6] != m.val[0x6] ||
		this->val[0x7] != m.val[0x7] ||
		this->val[0x8] != m.val[0x8] ||
		this->val[0x9] != m.val[0x9] ||
		this->val[0xa] != m.val[0xa] ||
		this->val[0xb] != m.val[0xb] ||
		this->val[0xc] != m.val[0xc] ||
		this->val[0xd] != m.val[0xd] ||
		this->val[0xe] != m.val[0xe] ||
		this->val[0xf] != m.val[0xf];
}

float DKMatrix4::Determinant(void) const
{
	return
		m[0][3] * m[1][2] * m[2][1] * m[3][0]-m[0][2] * m[1][3] * m[2][1] * m[3][0]-m[0][3] * m[1][1] * m[2][2] * m[3][0]+m[0][1] * m[1][3] * m[2][2] * m[3][0]+
		m[0][2] * m[1][1] * m[2][3] * m[3][0]-m[0][1] * m[1][2] * m[2][3] * m[3][0]-m[0][3] * m[1][2] * m[2][0] * m[3][1]+m[0][2] * m[1][3] * m[2][0] * m[3][1]+
		m[0][3] * m[1][0] * m[2][2] * m[3][1]-m[0][0] * m[1][3] * m[2][2] * m[3][1]-m[0][2] * m[1][0] * m[2][3] * m[3][1]+m[0][0] * m[1][2] * m[2][3] * m[3][1]+
		m[0][3] * m[1][1] * m[2][0] * m[3][2]-m[0][1] * m[1][3] * m[2][0] * m[3][2]-m[0][3] * m[1][0] * m[2][1] * m[3][2]+m[0][0] * m[1][3] * m[2][1] * m[3][2]+
		m[0][1] * m[1][0] * m[2][3] * m[3][2]-m[0][0] * m[1][1] * m[2][3] * m[3][2]-m[0][2] * m[1][1] * m[2][0] * m[3][3]+m[0][1] * m[1][2] * m[2][0] * m[3][3]+
		m[0][2] * m[1][0] * m[2][1] * m[3][3]-m[0][0] * m[1][2] * m[2][1] * m[3][3]-m[0][1] * m[1][0] * m[2][2] * m[3][3]+m[0][0] * m[1][1] * m[2][2] * m[3][3];
}

bool DKMatrix4::GetInverseMatrix(DKMatrix4& matOut, float *pDeterminant) const
{
	float det = Determinant();

	if (det != 0.0f)
	{
		if (pDeterminant)
			*pDeterminant = det;

		matOut.m[0][0] = (m[1][2]*m[2][3]*m[3][1] - m[1][3]*m[2][2]*m[3][1] + m[1][3]*m[2][1]*m[3][2] - m[1][1]*m[2][3]*m[3][2] - m[1][2]*m[2][1]*m[3][3] + m[1][1]*m[2][2]*m[3][3]) / det;
		matOut.m[0][1] = (m[0][3]*m[2][2]*m[3][1] - m[0][2]*m[2][3]*m[3][1] - m[0][3]*m[2][1]*m[3][2] + m[0][1]*m[2][3]*m[3][2] + m[0][2]*m[2][1]*m[3][3] - m[0][1]*m[2][2]*m[3][3]) / det;
		matOut.m[0][2] = (m[0][2]*m[1][3]*m[3][1] - m[0][3]*m[1][2]*m[3][1] + m[0][3]*m[1][1]*m[3][2] - m[0][1]*m[1][3]*m[3][2] - m[0][2]*m[1][1]*m[3][3] + m[0][1]*m[1][2]*m[3][3]) / det;
		matOut.m[0][3] = (m[0][3]*m[1][2]*m[2][1] - m[0][2]*m[1][3]*m[2][1] - m[0][3]*m[1][1]*m[2][2] + m[0][1]*m[1][3]*m[2][2] + m[0][2]*m[1][1]*m[2][3] - m[0][1]*m[1][2]*m[2][3]) / det;
		matOut.m[1][0] = (m[1][3]*m[2][2]*m[3][0] - m[1][2]*m[2][3]*m[3][0] - m[1][3]*m[2][0]*m[3][2] + m[1][0]*m[2][3]*m[3][2] + m[1][2]*m[2][0]*m[3][3] - m[1][0]*m[2][2]*m[3][3]) / det;
		matOut.m[1][1] = (m[0][2]*m[2][3]*m[3][0] - m[0][3]*m[2][2]*m[3][0] + m[0][3]*m[2][0]*m[3][2] - m[0][0]*m[2][3]*m[3][2] - m[0][2]*m[2][0]*m[3][3] + m[0][0]*m[2][2]*m[3][3]) / det;
		matOut.m[1][2] = (m[0][3]*m[1][2]*m[3][0] - m[0][2]*m[1][3]*m[3][0] - m[0][3]*m[1][0]*m[3][2] + m[0][0]*m[1][3]*m[3][2] + m[0][2]*m[1][0]*m[3][3] - m[0][0]*m[1][2]*m[3][3]) / det;
		matOut.m[1][3] = (m[0][2]*m[1][3]*m[2][0] - m[0][3]*m[1][2]*m[2][0] + m[0][3]*m[1][0]*m[2][2] - m[0][0]*m[1][3]*m[2][2] - m[0][2]*m[1][0]*m[2][3] + m[0][0]*m[1][2]*m[2][3]) / det;
		matOut.m[2][0] = (m[1][1]*m[2][3]*m[3][0] - m[1][3]*m[2][1]*m[3][0] + m[1][3]*m[2][0]*m[3][1] - m[1][0]*m[2][3]*m[3][1] - m[1][1]*m[2][0]*m[3][3] + m[1][0]*m[2][1]*m[3][3]) / det;
		matOut.m[2][1] = (m[0][3]*m[2][1]*m[3][0] - m[0][1]*m[2][3]*m[3][0] - m[0][3]*m[2][0]*m[3][1] + m[0][0]*m[2][3]*m[3][1] + m[0][1]*m[2][0]*m[3][3] - m[0][0]*m[2][1]*m[3][3]) / det;
		matOut.m[2][2] = (m[0][1]*m[1][3]*m[3][0] - m[0][3]*m[1][1]*m[3][0] + m[0][3]*m[1][0]*m[3][1] - m[0][0]*m[1][3]*m[3][1] - m[0][1]*m[1][0]*m[3][3] + m[0][0]*m[1][1]*m[3][3]) / det;
		matOut.m[2][3] = (m[0][3]*m[1][1]*m[2][0] - m[0][1]*m[1][3]*m[2][0] - m[0][3]*m[1][0]*m[2][1] + m[0][0]*m[1][3]*m[2][1] + m[0][1]*m[1][0]*m[2][3] - m[0][0]*m[1][1]*m[2][3]) / det;
		matOut.m[3][0] = (m[1][2]*m[2][1]*m[3][0] - m[1][1]*m[2][2]*m[3][0] - m[1][2]*m[2][0]*m[3][1] + m[1][0]*m[2][2]*m[3][1] + m[1][1]*m[2][0]*m[3][2] - m[1][0]*m[2][1]*m[3][2]) / det;
		matOut.m[3][1] = (m[0][1]*m[2][2]*m[3][0] - m[0][2]*m[2][1]*m[3][0] + m[0][2]*m[2][0]*m[3][1] - m[0][0]*m[2][2]*m[3][1] - m[0][1]*m[2][0]*m[3][2] + m[0][0]*m[2][1]*m[3][2]) / det;
		matOut.m[3][2] = (m[0][2]*m[1][1]*m[3][0] - m[0][1]*m[1][2]*m[3][0] - m[0][2]*m[1][0]*m[3][1] + m[0][0]*m[1][2]*m[3][1] + m[0][1]*m[1][0]*m[3][2] - m[0][0]*m[1][1]*m[3][2]) / det;
		matOut.m[3][3] = (m[0][1]*m[1][2]*m[2][0] - m[0][2]*m[1][1]*m[2][0] + m[0][2]*m[1][0]*m[2][1] - m[0][0]*m[1][2]*m[2][1] - m[0][1]*m[1][0]*m[2][2] + m[0][0]*m[1][1]*m[2][2]) / det;

		return true;
	}
	return false;
}

DKMatrix4& DKMatrix4::Inverse(void)
{
	DKMatrix4 mat;
	mat.Identity();
	if (GetInverseMatrix(mat, 0))
		*this = mat;
	return *this;
}

DKMatrix4& DKMatrix4::Transpose(void)
{
	DKMatrix4 mat(*this);

	this->m[0][1] = mat.m[1][0];
	this->m[0][2] = mat.m[2][0];
	this->m[0][3] = mat.m[3][0];

	this->m[1][0] = mat.m[0][1];
	this->m[1][2] = mat.m[2][1];
	this->m[1][3] = mat.m[3][1];

	this->m[2][0] = mat.m[0][2];
	this->m[2][1] = mat.m[1][2];
	this->m[2][3] = mat.m[3][2];

	this->m[3][0] = mat.m[0][3];
	this->m[3][1] = mat.m[1][3];
	this->m[3][2] = mat.m[2][3];

	return *this;
}

DKMatrix4& DKMatrix4::Multiply(const DKMatrix4& m)
{
	DKMatrix4 mat(*this);
	this->m[0][0] = (mat.m[0][0] * m.m[0][0]) + (mat.m[0][1] * m.m[1][0]) + (mat.m[0][2] * m.m[2][0]) + (mat.m[0][3] * m.m[3][0]);
	this->m[0][1] = (mat.m[0][0] * m.m[0][1]) + (mat.m[0][1] * m.m[1][1]) + (mat.m[0][2] * m.m[2][1]) + (mat.m[0][3] * m.m[3][1]);
	this->m[0][2] = (mat.m[0][0] * m.m[0][2]) + (mat.m[0][1] * m.m[1][2]) + (mat.m[0][2] * m.m[2][2]) + (mat.m[0][3] * m.m[3][2]);
	this->m[0][3] = (mat.m[0][0] * m.m[0][3]) + (mat.m[0][1] * m.m[1][3]) + (mat.m[0][2] * m.m[2][3]) + (mat.m[0][3] * m.m[3][3]);
	this->m[1][0] = (mat.m[1][0] * m.m[0][0]) + (mat.m[1][1] * m.m[1][0]) + (mat.m[1][2] * m.m[2][0]) + (mat.m[1][3] * m.m[3][0]);
	this->m[1][1] = (mat.m[1][0] * m.m[0][1]) + (mat.m[1][1] * m.m[1][1]) + (mat.m[1][2] * m.m[2][1]) + (mat.m[1][3] * m.m[3][1]);
	this->m[1][2] = (mat.m[1][0] * m.m[0][2]) + (mat.m[1][1] * m.m[1][2]) + (mat.m[1][2] * m.m[2][2]) + (mat.m[1][3] * m.m[3][2]);
	this->m[1][3] = (mat.m[1][0] * m.m[0][3]) + (mat.m[1][1] * m.m[1][3]) + (mat.m[1][2] * m.m[2][3]) + (mat.m[1][3] * m.m[3][3]);
	this->m[2][0] = (mat.m[2][0] * m.m[0][0]) + (mat.m[2][1] * m.m[1][0]) + (mat.m[2][2] * m.m[2][0]) + (mat.m[2][3] * m.m[3][0]);
	this->m[2][1] = (mat.m[2][0] * m.m[0][1]) + (mat.m[2][1] * m.m[1][1]) + (mat.m[2][2] * m.m[2][1]) + (mat.m[2][3] * m.m[3][1]);
	this->m[2][2] = (mat.m[2][0] * m.m[0][2]) + (mat.m[2][1] * m.m[1][2]) + (mat.m[2][2] * m.m[2][2]) + (mat.m[2][3] * m.m[3][2]);
	this->m[2][3] = (mat.m[2][0] * m.m[0][3]) + (mat.m[2][1] * m.m[1][3]) + (mat.m[2][2] * m.m[2][3]) + (mat.m[2][3] * m.m[3][3]);
	this->m[3][0] = (mat.m[3][0] * m.m[0][0]) + (mat.m[3][1] * m.m[1][0]) + (mat.m[3][2] * m.m[2][0]) + (mat.m[3][3] * m.m[3][0]);
	this->m[3][1] = (mat.m[3][0] * m.m[0][1]) + (mat.m[3][1] * m.m[1][1]) + (mat.m[3][2] * m.m[2][1]) + (mat.m[3][3] * m.m[3][1]);
	this->m[3][2] = (mat.m[3][0] * m.m[0][2]) + (mat.m[3][1] * m.m[1][2]) + (mat.m[3][2] * m.m[2][2]) + (mat.m[3][3] * m.m[3][2]);
	this->m[3][3] = (mat.m[3][0] * m.m[0][3]) + (mat.m[3][1] * m.m[1][3]) + (mat.m[3][2] * m.m[2][3]) + (mat.m[3][3] * m.m[3][3]);
	return *this;
}

DKVector4 DKMatrix4::Row1(void) const
{
	return DKVector4(m[0][0], m[0][1], m[0][2], m[0][3]);
}

DKVector4 DKMatrix4::Row2(void) const
{
	return DKVector4(m[1][0], m[1][1], m[1][2], m[1][3]);
}

DKVector4 DKMatrix4::Row3(void) const
{
	return DKVector4(m[2][0], m[2][1], m[2][2], m[2][3]);
}

DKVector4 DKMatrix4::Row4(void) const
{
	return DKVector4(m[3][0], m[3][1], m[3][2], m[3][3]);
}

DKVector4 DKMatrix4::Column1(void) const
{
	return DKVector4(m[0][0], m[1][0], m[2][0], m[3][0]);
}

DKVector4 DKMatrix4::Column2(void) const
{
	return DKVector4(m[0][1], m[1][1], m[2][1], m[3][1]);
}

DKVector4 DKMatrix4::Column3(void) const
{
	return DKVector4(m[0][2], m[1][2], m[2][2], m[3][2]);
}

DKVector4 DKMatrix4::Column4(void) const
{
	return DKVector4(m[0][3], m[1][3], m[2][3], m[3][3]);
}
