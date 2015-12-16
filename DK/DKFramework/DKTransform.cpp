//
//  File: DKTransform.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKTransform.h"
#include "DKAffineTransform3.h"
#include "DKLinearTransform3.h"

using namespace DKFoundation;
using namespace DKFramework;

const DKTransformUnit DKTransformUnit::identity = DKTransformUnit().Identity();
const DKUSTransform DKUSTransform::identity = DKUSTransform().Identity();
const DKNSTransform DKNSTransform::identity = DKNSTransform().Identity();

////////////////////////////////////////////////////////////////////////////////
// class DKTransformUnit
////////////////////////////////////////////////////////////////////////////////
DKTransformUnit::DKTransformUnit(void)
	: scale(1.0f, 1.0f, 1.0f)
	, rotation(0.0f, 0.0f, 0.0f, 1.0f)
	, translation(0.0f, 0.0f, 0.0f)
{
}

DKTransformUnit::DKTransformUnit(const DKVector3& s, const DKQuaternion& r, const DKVector3& t)
	: scale(s)
	, rotation(r)
	, translation(t)
{
}

DKMatrix3 DKTransformUnit::Matrix3() const
{
	DKMatrix3 mat3 = rotation.Matrix3();

	// mat3 = scale * rotation
	mat3.m[0][0] *= scale.x;
	mat3.m[0][1] *= scale.x;
	mat3.m[0][2] *= scale.x;

	mat3.m[1][0] *= scale.y;
	mat3.m[1][1] *= scale.y;
	mat3.m[1][2] *= scale.y;

	mat3.m[2][0] *= scale.z;
	mat3.m[2][1] *= scale.z;
	mat3.m[2][2] *= scale.z;

	return mat3;
}

DKMatrix4 DKTransformUnit::Matrix4(void) const
{
	DKMatrix3 mat3 = Matrix3();
	return DKMatrix4(
		mat3.m[0][0], mat3.m[0][1], mat3.m[0][2], 0.0f,
		mat3.m[1][0], mat3.m[1][1], mat3.m[1][2], 0.0f,
		mat3.m[2][0], mat3.m[2][1], mat3.m[2][2], 0.0f,
		translation.x, translation.y, translation.z, 1.0f);
}

DKTransformUnit DKTransformUnit::Interpolate(const DKTransformUnit& target, float t) const
{
	return DKTransformUnit(
		scale + ((target.scale - scale) * t),
		DKQuaternion::Slerp(rotation, target.rotation, t),
		translation + ((target.translation - translation) * t));
}

DKTransformUnit& DKTransformUnit::Identity(void)
{
	scale = DKVector3(1.0f, 1.0f, 1.0f);
	rotation = DKQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
	translation = DKVector3(0.0f, 0.0f, 0.0f);
	return *this;
}

bool DKTransformUnit::operator == (const DKTransformUnit& t) const
{
	return t.scale == this->scale && t.rotation == this->rotation && t.translation == this->translation;
}

bool DKTransformUnit::operator != (const DKTransformUnit& t) const
{
	return t.scale != this->scale || t.rotation != this->rotation || t.translation != this->translation;
}

////////////////////////////////////////////////////////////////////////////////
// class DKUSTransform
////////////////////////////////////////////////////////////////////////////////

DKUSTransform::DKUSTransform(void)
	: scale(1.0f)
	, orientation(0.0f, 0.0f, 0.0f, 1.0f)
	, position(0.0f, 0.0f, 0.0f)
{
}

DKUSTransform::DKUSTransform(float s, const DKQuaternion& r, const DKVector3& t)
	: scale(s)
	, orientation(r)
	, position(t)
{
}

DKMatrix3 DKUSTransform::Matrix3(void) const
{
	return orientation.Matrix3() * scale;
}

DKMatrix4 DKUSTransform::Matrix4(void) const
{
	DKMatrix3 mat3 = orientation.Matrix3() * scale;
	return DKMatrix4(
		mat3.m[0][0], mat3.m[0][1], mat3.m[0][2], 0.0f,
		mat3.m[1][0], mat3.m[1][1], mat3.m[1][2], 0.0f,
		mat3.m[2][0], mat3.m[2][1], mat3.m[2][2], 0.0f,
		position.x, position.y, position.z, 1.0f);
}

DKUSTransform DKUSTransform::Interpolate(const DKUSTransform& target, float t) const
{
	return DKUSTransform(
		scale + ((target.scale - scale) * t),
		DKQuaternion::Slerp(orientation, target.orientation, t),
		position + ((target.position - position) * t));
}

DKUSTransform& DKUSTransform::Identity(void)
{
	scale = 1.0f;
	orientation = DKQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
	position = DKVector3(0.0f, 0.0f, 0.0f);
	return *this;
}

DKUSTransform& DKUSTransform::Inverse(void)
{
	scale = 1.0 / scale;
	orientation.Conjugate();

	position = (-position * scale) * orientation;
	return *this;
}

DKUSTransform DKUSTransform::operator * (const DKUSTransform& t) const
{
	DKMatrix3 mat3 = t.Matrix3();
	return DKUSTransform( scale * t.scale, orientation * t.orientation, position * mat3 + t.position );
}

DKUSTransform& DKUSTransform::operator *= (const DKUSTransform& t)
{
	DKMatrix3 mat3 = t.Matrix3();

	this->position = this->position * mat3 + t.position;
	this->orientation = this->orientation * t.orientation;
	this->scale *= t.scale;
	return *this;
}

bool DKUSTransform::operator == (const DKUSTransform& t) const
{
	return t.scale == this->scale && t.orientation == this->orientation && t.position == this->position;
}

bool DKUSTransform::operator != (const DKUSTransform& t) const
{
	return t.scale != this->scale || t.orientation != this->orientation || t.position != this->position;
}

////////////////////////////////////////////////////////////////////////////////
// class DKNSTransform
////////////////////////////////////////////////////////////////////////////////

DKNSTransform::DKNSTransform(const DKQuaternion& r, const DKVector3& t)
	: orientation(r), position(t)
{
}

DKNSTransform::DKNSTransform(const DKMatrix3& r, const DKVector3& t)
	: orientation(DKLinearTransform3(r).Rotation()), position(t)
{
}

DKNSTransform::DKNSTransform(const DKVector3& t)
	: orientation(DKQuaternion::identity), position(t)
{
}

DKNSTransform& DKNSTransform::Identity(void)
{
	orientation.Identity();
	position.x = 0;
	position.y = 0;
	position.z = 0;
	return *this;
}

DKNSTransform& DKNSTransform::Inverse(void)
{
	orientation.Conjugate();
	position = -position * orientation;
	return *this;
}

DKNSTransform DKNSTransform::Interpolate(const DKNSTransform& target, float t) const
{
	return DKNSTransform(
		DKQuaternion::Slerp(this->orientation, target.orientation, t),
		this->position + ((target.position - this->position) * t));
}

DKMatrix3 DKNSTransform::Matrix3(void) const
{
	return orientation.Matrix3();
}

DKMatrix4 DKNSTransform::Matrix4(void) const
{
	DKMatrix3 mat3 = orientation.Matrix3();
	return DKMatrix4(
		mat3.m[0][0], mat3.m[0][1], mat3.m[0][2], 0.0f,
		mat3.m[1][0], mat3.m[1][1], mat3.m[1][2], 0.0f,
		mat3.m[2][0], mat3.m[2][1], mat3.m[2][2], 0.0f,
		position.x, position.y, position.z, 1.0f);
}

DKNSTransform DKNSTransform::operator * (const DKNSTransform& t) const
{
	return DKNSTransform( this->orientation * t.orientation, this->position * t.orientation + t.position);
}

DKNSTransform& DKNSTransform::operator *= (const DKNSTransform& t)
{
	this->position = this->position * t.orientation + t.position;
	this->orientation *= t.orientation;
	return *this;
}

bool DKNSTransform::operator == (const DKNSTransform& t) const
{
	return t.orientation == this->orientation && t.position == this->position;
}

bool DKNSTransform::operator != (const DKNSTransform& t) const
{
	return t.orientation != this->orientation || t.position != this->position;
}
