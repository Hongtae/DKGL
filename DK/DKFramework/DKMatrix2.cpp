//
//  File: DKMatrix2.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKMatrix2.h"
#include "DKVector2.h"

using namespace DKGL;
using namespace DKGL;

const DKMatrix2 DKMatrix2::identity = DKMatrix2().Identity();

DKMatrix2::DKMatrix2(void)
	: _11(1), _12(0)
	, _21(0), _22(1)
{
}

DKMatrix2::DKMatrix2(const DKVector2& row1, const DKVector2& row2)
	: _11(row1.x), _12(row1.y)
	, _21(row2.x), _22(row2.y)
{
}

DKMatrix2::DKMatrix2(float e11, float e12, float e21, float e22)
	: _11(e11), _12(e12)
	, _21(e21), _22(e22)
{
}

DKMatrix2& DKMatrix2::Zero(void)
{
	m[0][0] = m[0][1] = m[1][0] = m[1][1] = 0.0f;
	return *this;
}

DKMatrix2& DKMatrix2::Identity(void)
{
	m[0][0] = m[1][1] = 1.0f;
	m[0][1] = m[1][0] = 0.0f;
	return *this;
}

bool DKMatrix2::IsIdentity(void) const
{
	return
		m[0][0] == 1.0f && m[0][1] == 0.0f &&
		m[1][0] == 0.0f && m[1][1] == 1.0f;
}

bool DKMatrix2::IsDiagonal(void) const
{
	return m[0][1] == 0.0f && m[1][0] == 0.0f;
}

float DKMatrix2::Determinant(void) const
{
	return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

bool DKMatrix2::GetInverseMatrix(DKMatrix2& matOut, float *pDeterminant) const
{
	float det = m[0][0] * m[1][1] - m[0][1] * m[1][0];

	if (det != 0.0f)
	{
		if (pDeterminant)
			*pDeterminant = det;

		float detInv = 1.0f / det;

		matOut.m[0][0] = m[1][1] * detInv;
		matOut.m[0][1] = -m[0][1] * detInv;
		matOut.m[1][0] = -m[1][0] * detInv;
		matOut.m[1][1] = m[0][0] * detInv;
		return true;
	}
	return false;
}

DKMatrix2& DKMatrix2::Inverse(void)
{
	DKMatrix2 mat;
	mat.Identity();
	if (GetInverseMatrix(mat, 0))
		*this = mat;
	return *this;
}

DKMatrix2& DKMatrix2::Transpose(void)
{
	float tmp = this->m[0][1];
	this->m[0][1] = this->m[1][0];
	this->m[1][0] = tmp;
	return *this;
}

DKMatrix2& DKMatrix2::Multiply(const DKMatrix2& m)
{
	DKMatrix2 mat(*this);
	this->m[0][0] = (mat.m[0][0] * m.m[0][0]) + (mat.m[0][1] * m.m[1][0]);
	this->m[0][1] = (mat.m[0][0] * m.m[0][1]) + (mat.m[0][1] * m.m[1][1]);
	this->m[1][0] = (mat.m[1][0] * m.m[0][0]) + (mat.m[1][1] * m.m[1][0]);
	this->m[1][1] = (mat.m[1][0] * m.m[0][1]) + (mat.m[1][1] * m.m[1][1]);
	return *this;
}

DKMatrix2 DKMatrix2::operator * (const DKMatrix2& m) const
{
	DKMatrix2 mat;
	mat.m[0][0] = (this->m[0][0] * m.m[0][0]) + (this->m[0][1] * m.m[1][0]);
	mat.m[0][1] = (this->m[0][0] * m.m[0][1]) + (this->m[0][1] * m.m[1][1]);
	mat.m[1][0] = (this->m[1][0] * m.m[0][0]) + (this->m[1][1] * m.m[1][0]);
	mat.m[1][1] = (this->m[1][0] * m.m[0][1]) + (this->m[1][1] * m.m[1][1]);
	return mat;
}

DKMatrix2 DKMatrix2::operator + (const DKMatrix2& m) const
{
	DKMatrix2 mat;
	mat.val[0] = this->val[0] + m.val[0];
	mat.val[1] = this->val[1] + m.val[1];
	mat.val[2] = this->val[2] + m.val[2];
	mat.val[3] = this->val[3] + m.val[3];
	return mat;
}

DKMatrix2 DKMatrix2::operator - (const DKMatrix2& m) const
{
	DKMatrix2 mat;
	mat.val[0] = this->val[0] - m.val[0];
	mat.val[1] = this->val[1] - m.val[1];
	mat.val[2] = this->val[2] - m.val[2];
	mat.val[3] = this->val[3] - m.val[3];
	return mat;
}

DKMatrix2 DKMatrix2::operator * (float f) const
{
	DKMatrix2 mat;
	mat.val[0] = this->val[0] * f;
	mat.val[1] = this->val[1] * f;
	mat.val[2] = this->val[2] * f;
	mat.val[3] = this->val[3] * f;
	return mat;
}

DKMatrix2 DKMatrix2::operator / (float f) const
{
	float inv = 1.0f / f;
	DKMatrix2 mat;
	mat.val[0] = this->val[0] * inv;
	mat.val[1] = this->val[1] * inv;
	mat.val[2] = this->val[2] * inv;
	mat.val[3] = this->val[3] * inv;
	return mat;
}

DKMatrix2& DKMatrix2::operator *= (const DKMatrix2& m)
{
	DKMatrix2 mat(*this);
	this->m[0][0] = (mat.m[0][0] * m.m[0][0]) + (mat.m[0][1] * m.m[1][0]);
	this->m[0][1] = (mat.m[0][0] * m.m[0][1]) + (mat.m[0][1] * m.m[1][1]);
	this->m[1][0] = (mat.m[1][0] * m.m[0][0]) + (mat.m[1][1] * m.m[1][0]);
	this->m[1][1] = (mat.m[1][0] * m.m[0][1]) + (mat.m[1][1] * m.m[1][1]);
	return *this;
}

DKMatrix2& DKMatrix2::operator += (const DKMatrix2& m)
{
	this->val[0] += m.val[0];
	this->val[1] += m.val[1];
	this->val[2] += m.val[2];
	this->val[3] += m.val[3];
	return *this;
}

DKMatrix2& DKMatrix2::operator -= (const DKMatrix2& m)
{
	this->val[0] -= m.val[0];
	this->val[1] -= m.val[1];
	this->val[2] -= m.val[2];
	this->val[3] -= m.val[3];
	return *this;
}

DKMatrix2& DKMatrix2::operator *= (float f)
{
	this->val[0] *= f;
	this->val[1] *= f;
	this->val[2] *= f;
	this->val[3] *= f;
	return *this;
}

DKMatrix2& DKMatrix2::operator /= (float f)
{
	float inv = 1.0f / f;
	this->val[0] *= inv;
	this->val[1] *= inv;
	this->val[2] *= inv;
	this->val[3] *= inv;
	return *this;
}

bool DKMatrix2::operator == (const DKMatrix2& m) const
{
	return
		this->val[0] == m.val[0] && this->val[1] == m.val[1] &&
		this->val[2] == m.val[2] && this->val[3] == m.val[3];
}

bool DKMatrix2::operator != (const DKMatrix2& m) const
{
	return
		this->val[0] != m.val[0] || this->val[1] != m.val[1] ||
		this->val[2] != m.val[2] || this->val[3] != m.val[3];
}

DKVector2 DKMatrix2::Row1(void) const
{
	return DKVector2(m[0][0], m[0][1]);
}

DKVector2 DKMatrix2::Row2(void) const
{
	return DKVector2(m[1][0], m[1][1]);
}

DKVector2 DKMatrix2::Column1(void) const
{
	return DKVector2(m[0][0], m[1][0]);
}

DKVector2 DKMatrix2::Column2(void) const
{
	return DKVector2(m[0][1], m[1][1]);
}
