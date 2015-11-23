//
//  File: DKMatrix3.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKMatrix3.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKQuaternion.h"

using namespace DKFoundation;
using namespace DKFramework;

const DKMatrix3 DKMatrix3::identity = DKMatrix3().Identity();

DKMatrix3::DKMatrix3(void)
	: _11(1), _12(0), _13(0)
	, _21(0), _22(1), _23(0)
	, _31(0), _32(0), _33(1)
{
}

DKMatrix3::DKMatrix3(const DKVector3& row1, const DKVector3& row2, const DKVector3& row3)
	: _11(row1.x), _12(row1.y), _13(row1.z)
	, _21(row2.x), _22(row2.y), _23(row2.z)
	, _31(row3.x), _32(row3.y), _33(row3.z)
{
}

DKMatrix3::DKMatrix3(float e11, float e12, float e13,
	float e21, float e22, float e23,
	float e31, float e32, float e33)
	: _11(e11), _12(e12), _13(e13)
	, _21(e21), _22(e22), _23(e23)
	, _31(e31), _32(e32), _33(e33)
{
}

DKMatrix3& DKMatrix3::Zero(void)
{
	m[0][0] = m[0][1] = m[0][2] = 0.0f;
	m[1][0] = m[1][1] = m[1][2] = 0.0f;
	m[2][0] = m[2][1] = m[2][2] = 0.0f;
	return *this;
}

DKMatrix3& DKMatrix3::Identity(void)
{
	m[0][0] = m[1][1] = m[2][2] = 1.0f;
	m[0][1] = m[0][2] = 0.0f;
	m[1][0] = m[1][2] = 0.0f;
	m[2][0] = m[2][1] = 0.0f;
	return *this;
}

bool DKMatrix3::IsIdentity(void) const
{
	return
		m[0][0] == 1.0f && m[0][1] == 0.0f && m[0][2] == 0.0f &&
		m[1][0] == 0.0f && m[1][1] == 1.0f && m[1][2] == 0.0f &&
		m[2][0] == 0.0f && m[2][1] == 0.0f && m[2][2] == 1.0f;
}

bool DKMatrix3::IsDiagonal(void) const
{
	return
		m[0][1] == 0.0f && m[0][2] == 0.0f &&
		m[1][0] == 0.0f && m[1][2] == 0.0f &&
		m[2][0] == 0.0f && m[2][1] == 0.0f;
}

DKMatrix3 DKMatrix3::operator * (const DKMatrix3& m) const
{
	DKMatrix3 mat;
	mat.m[0][0] = (this->m[0][0] * m.m[0][0]) + (this->m[0][1] * m.m[1][0]) + (this->m[0][2] * m.m[2][0]);
	mat.m[0][1] = (this->m[0][0] * m.m[0][1]) + (this->m[0][1] * m.m[1][1]) + (this->m[0][2] * m.m[2][1]);
	mat.m[0][2] = (this->m[0][0] * m.m[0][2]) + (this->m[0][1] * m.m[1][2]) + (this->m[0][2] * m.m[2][2]);
	mat.m[1][0] = (this->m[1][0] * m.m[0][0]) + (this->m[1][1] * m.m[1][0]) + (this->m[1][2] * m.m[2][0]);
	mat.m[1][1] = (this->m[1][0] * m.m[0][1]) + (this->m[1][1] * m.m[1][1]) + (this->m[1][2] * m.m[2][1]);
	mat.m[1][2] = (this->m[1][0] * m.m[0][2]) + (this->m[1][1] * m.m[1][2]) + (this->m[1][2] * m.m[2][2]);
	mat.m[2][0] = (this->m[2][0] * m.m[0][0]) + (this->m[2][1] * m.m[1][0]) + (this->m[2][2] * m.m[2][0]);
	mat.m[2][1] = (this->m[2][0] * m.m[0][1]) + (this->m[2][1] * m.m[1][1]) + (this->m[2][2] * m.m[2][1]);
	mat.m[2][2] = (this->m[2][0] * m.m[0][2]) + (this->m[2][1] * m.m[1][2]) + (this->m[2][2] * m.m[2][2]);
	return mat;
}

DKMatrix3 DKMatrix3::operator + (const DKMatrix3& m) const
{
	DKMatrix3 mat;
	mat.val[0] = this->val[0] + m.val[0];
	mat.val[1] = this->val[1] + m.val[1];
	mat.val[2] = this->val[2] + m.val[2];
	mat.val[3] = this->val[3] + m.val[3];
	mat.val[4] = this->val[4] + m.val[4];
	mat.val[5] = this->val[5] + m.val[5];
	mat.val[6] = this->val[6] + m.val[6];
	mat.val[7] = this->val[7] + m.val[7];
	mat.val[8] = this->val[8] + m.val[8];
	return mat;
}

DKMatrix3 DKMatrix3::operator - (const DKMatrix3& m) const
{
	DKMatrix3 mat;
	mat.val[0] = this->val[0] - m.val[0];
	mat.val[1] = this->val[1] - m.val[1];
	mat.val[2] = this->val[2] - m.val[2];
	mat.val[3] = this->val[3] - m.val[3];
	mat.val[4] = this->val[4] - m.val[4];
	mat.val[5] = this->val[5] - m.val[5];
	mat.val[6] = this->val[6] - m.val[6];
	mat.val[7] = this->val[7] - m.val[7];
	mat.val[8] = this->val[8] - m.val[8];
	return mat;
}

DKMatrix3 DKMatrix3::operator * (float f) const
{
	DKMatrix3 mat;
	mat.val[0] = this->val[0] * f;
	mat.val[1] = this->val[1] * f;
	mat.val[2] = this->val[2] * f;
	mat.val[3] = this->val[3] * f;
	mat.val[4] = this->val[4] * f;
	mat.val[5] = this->val[5] * f;
	mat.val[6] = this->val[6] * f;
	mat.val[7] = this->val[7] * f;
	mat.val[8] = this->val[8] * f;
	return mat;
}

DKMatrix3 DKMatrix3::operator / (float f) const
{
	float inv = 1.0f / f;
	DKMatrix3 mat;
	mat.val[0] = this->val[0] * inv;
	mat.val[1] = this->val[1] * inv;
	mat.val[2] = this->val[2] * inv;
	mat.val[3] = this->val[3] * inv;
	mat.val[4] = this->val[4] * inv;
	mat.val[5] = this->val[5] * inv;
	mat.val[6] = this->val[6] * inv;
	mat.val[7] = this->val[7] * inv;
	mat.val[8] = this->val[8] * inv;
	return mat;
}

DKMatrix3& DKMatrix3::operator *= (const DKMatrix3& m)
{
	DKMatrix3 mat(*this);
	this->m[0][0] = (mat.m[0][0] * m.m[0][0]) + (mat.m[0][1] * m.m[1][0]) + (mat.m[0][2] * m.m[2][0]);
	this->m[0][1] = (mat.m[0][0] * m.m[0][1]) + (mat.m[0][1] * m.m[1][1]) + (mat.m[0][2] * m.m[2][1]);
	this->m[0][2] = (mat.m[0][0] * m.m[0][2]) + (mat.m[0][1] * m.m[1][2]) + (mat.m[0][2] * m.m[2][2]);
	this->m[1][0] = (mat.m[1][0] * m.m[0][0]) + (mat.m[1][1] * m.m[1][0]) + (mat.m[1][2] * m.m[2][0]);
	this->m[1][1] = (mat.m[1][0] * m.m[0][1]) + (mat.m[1][1] * m.m[1][1]) + (mat.m[1][2] * m.m[2][1]);
	this->m[1][2] = (mat.m[1][0] * m.m[0][2]) + (mat.m[1][1] * m.m[1][2]) + (mat.m[1][2] * m.m[2][2]);
	this->m[2][0] = (mat.m[2][0] * m.m[0][0]) + (mat.m[2][1] * m.m[1][0]) + (mat.m[2][2] * m.m[2][0]);
	this->m[2][1] = (mat.m[2][0] * m.m[0][1]) + (mat.m[2][1] * m.m[1][1]) + (mat.m[2][2] * m.m[2][1]);
	this->m[2][2] = (mat.m[2][0] * m.m[0][2]) + (mat.m[2][1] * m.m[1][2]) + (mat.m[2][2] * m.m[2][2]);
	return *this;
}

DKMatrix3& DKMatrix3::operator += (const DKMatrix3& m)
{
	this->val[0] += m.val[0];
	this->val[1] += m.val[1];
	this->val[2] += m.val[2];
	this->val[3] += m.val[3];
	this->val[4] += m.val[4];
	this->val[5] += m.val[5];
	this->val[6] += m.val[6];
	this->val[7] += m.val[7];
	this->val[8] += m.val[8];
	return *this;
}

DKMatrix3& DKMatrix3::operator -= (const DKMatrix3& m)
{
	this->val[0] -= m.val[0];
	this->val[1] -= m.val[1];
	this->val[2] -= m.val[2];
	this->val[3] -= m.val[3];
	this->val[4] -= m.val[4];
	this->val[5] -= m.val[5];
	this->val[6] -= m.val[6];
	this->val[7] -= m.val[7];
	this->val[8] -= m.val[8];
	return *this;
}

DKMatrix3& DKMatrix3::operator *= (float f)
{
	this->val[0] *= f;
	this->val[1] *= f;
	this->val[2] *= f;
	this->val[3] *= f;
	this->val[4] *= f;
	this->val[5] *= f;
	this->val[6] *= f;
	this->val[7] *= f;
	this->val[8] *= f;
	return *this;
}

DKMatrix3& DKMatrix3::operator /= (float f)
{
	float inv = 1.0f / f;
	this->val[0] *= inv;
	this->val[1] *= inv;
	this->val[2] *= inv;
	this->val[3] *= inv;
	this->val[4] *= inv;
	this->val[5] *= inv;
	this->val[6] *= inv;
	this->val[7] *= inv;
	this->val[8] *= inv;
	return *this;
}

bool DKMatrix3::operator == (const DKMatrix3& m) const
{
	return
		this->val[0] == m.val[0] &&
		this->val[1] == m.val[1] &&
		this->val[2] == m.val[2] &&
		this->val[3] == m.val[3] &&
		this->val[4] == m.val[4] &&
		this->val[5] == m.val[5] &&
		this->val[6] == m.val[6] &&
		this->val[7] == m.val[7] &&
		this->val[8] == m.val[8];
}

bool DKMatrix3::operator != (const DKMatrix3& m) const
{
	return
		this->val[0] != m.val[0] ||
		this->val[1] != m.val[1] ||
		this->val[2] != m.val[2] ||
		this->val[3] != m.val[3] ||
		this->val[4] != m.val[4] ||
		this->val[5] != m.val[5] ||
		this->val[6] != m.val[6] ||
		this->val[7] != m.val[7] ||
		this->val[8] != m.val[8];
}

float DKMatrix3::Determinant(void) const
{
	return
		m[0][0]*m[1][1]*m[2][2] +
		m[0][1]*m[1][2]*m[2][0] +
		m[0][2]*m[1][0]*m[2][1] -
		m[0][0]*m[1][2]*m[2][1] -
		m[0][1]*m[1][0]*m[2][2] -
		m[0][2]*m[1][1]*m[2][0];
}

bool DKMatrix3::GetInverseMatrix(DKMatrix3& matOut, float *pDeterminant) const
{
	float det = Determinant();

	if (det != 0.0f)
	{
		if (pDeterminant)
			*pDeterminant = det;

		float detInv = 1.0f / det;

		matOut.m[0][0] = (m[1][1]*m[2][2] - m[1][2]*m[2][1]) * detInv;
		matOut.m[0][1] = (m[0][2]*m[2][1] - m[0][1]*m[2][2]) * detInv;
		matOut.m[0][2] = (m[0][1]*m[1][2] - m[0][2]*m[1][1]) * detInv;
		matOut.m[1][0] = (m[1][2]*m[2][0] - m[1][0]*m[2][2]) * detInv;
		matOut.m[1][1] = (m[0][0]*m[2][2] - m[0][2]*m[2][0]) * detInv;
		matOut.m[1][2] = (m[0][2]*m[1][0] - m[0][0]*m[1][2]) * detInv;
		matOut.m[2][0] = (m[1][0]*m[2][1] - m[1][1]*m[2][0]) * detInv;
		matOut.m[2][1] = (m[0][1]*m[2][0] - m[0][0]*m[2][1]) * detInv;
		matOut.m[2][2] = (m[0][0]*m[1][1] - m[0][1]*m[1][0]) * detInv;
		return true;
	}
	return false;
}

DKMatrix3& DKMatrix3::Inverse(void)
{
	DKMatrix3 mat;
	mat.Identity();
	if (GetInverseMatrix(mat, 0))
		*this = mat;
	return *this;
}

DKMatrix3& DKMatrix3::Transpose(void)
{
	DKMatrix3 mat(*this);
	this->m[0][1] = mat.m[1][0];
	this->m[0][2] = mat.m[2][0];
	this->m[1][0] = mat.m[0][1];
	this->m[1][2] = mat.m[2][1];
	this->m[2][0] = mat.m[0][2];
	this->m[2][1] = mat.m[1][2];
	return *this;
}

DKMatrix3& DKMatrix3::Multiply(const DKMatrix3& m)
{
	DKMatrix3 mat(*this);
	this->m[0][0] = (mat.m[0][0] * m.m[0][0]) + (mat.m[0][1] * m.m[1][0]) + (mat.m[0][2] * m.m[2][0]);
	this->m[0][1] = (mat.m[0][0] * m.m[0][1]) + (mat.m[0][1] * m.m[1][1]) + (mat.m[0][2] * m.m[2][1]);
	this->m[0][2] = (mat.m[0][0] * m.m[0][2]) + (mat.m[0][1] * m.m[1][2]) + (mat.m[0][2] * m.m[2][2]);
	this->m[1][0] = (mat.m[1][0] * m.m[0][0]) + (mat.m[1][1] * m.m[1][0]) + (mat.m[1][2] * m.m[2][0]);
	this->m[1][1] = (mat.m[1][0] * m.m[0][1]) + (mat.m[1][1] * m.m[1][1]) + (mat.m[1][2] * m.m[2][1]);
	this->m[1][2] = (mat.m[1][0] * m.m[0][2]) + (mat.m[1][1] * m.m[1][2]) + (mat.m[1][2] * m.m[2][2]);
	this->m[2][0] = (mat.m[2][0] * m.m[0][0]) + (mat.m[2][1] * m.m[1][0]) + (mat.m[2][2] * m.m[2][0]);
	this->m[2][1] = (mat.m[2][0] * m.m[0][1]) + (mat.m[2][1] * m.m[1][1]) + (mat.m[2][2] * m.m[2][1]);
	this->m[2][2] = (mat.m[2][0] * m.m[0][2]) + (mat.m[2][1] * m.m[1][2]) + (mat.m[2][2] * m.m[2][2]);
	return *this;
}

DKVector3 DKMatrix3::Row1(void) const
{
	return DKVector3(m[0][0], m[0][1], m[0][2]);
}

DKVector3 DKMatrix3::Row2(void) const
{
	return DKVector3(m[1][0], m[1][1], m[1][2]);
}

DKVector3 DKMatrix3::Row3(void) const
{
	return DKVector3(m[2][0], m[2][1], m[2][2]);
}

DKVector3 DKMatrix3::Column1(void) const
{
	return DKVector3(m[0][0], m[1][0], m[2][0]);
}

DKVector3 DKMatrix3::Column2(void) const
{
	return DKVector3(m[0][1], m[1][1], m[2][1]);
}

DKVector3 DKMatrix3::Column3(void) const
{
	return DKVector3(m[0][2], m[1][2], m[2][2]);
}
