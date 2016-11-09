//
//  File: DKCamera.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKCamera.h"
#include "DKAffineTransform3.h"

using namespace DKFramework;

DKCamera::DKCamera(void)
	: viewMatrix(DKMatrix4::identity)
	, projectionMatrix(DKMatrix4::identity)
{
	SetOrthographic(2.0, 2.0, -1, 1);
}

DKCamera::~DKCamera(void)
{
}

void DKCamera::SetView(const DKVector3& pos, const DKVector3& dir, const DKVector3& up)
{
	DKASSERT_DEBUG(dir.Length() > 0);
	DKASSERT_DEBUG(up.Length() > 0);

	DKVector3 axisZ = -DKVector3(dir).Normalize();
	DKVector3 axisX = DKVector3::Cross(up, axisZ).Normalize();
	DKVector3 axisY = DKVector3::Cross(axisZ, axisX).Normalize();

	float tX = -DKVector3::Dot(axisX, pos);
	float tY = -DKVector3::Dot(axisY, pos);
	float tZ = -DKVector3::Dot(axisZ, pos);

	DKMatrix4 mat(
		axisX.x, axisY.x, axisZ.x, 0.0f,
		axisX.y, axisY.y, axisZ.y, 0.0f,
		axisX.z, axisY.z, axisZ.z, 0.0f,
		tX, tY, tZ, 1.0f);

	SetView(mat);
}

DKVector3 DKCamera::ViewPosition(void) const
{
	DKVector4 v = viewMatrix.Row4();
	DKMatrix3 m(
		viewMatrix.m[0][0], viewMatrix.m[0][1], viewMatrix.m[0][2],
		viewMatrix.m[1][0], viewMatrix.m[1][1], viewMatrix.m[1][2],
		viewMatrix.m[2][0], viewMatrix.m[2][1], viewMatrix.m[2][2]);

	return DKVector3(-v.x, -v.y, -v.z) * m.Inverse();
}

DKVector3 DKCamera::ViewDirection(void) const
{
	DKVector4 v = viewMatrix.Column3();
	return DKVector3(-v.x, -v.y, -v.z).Normalize();
}

DKVector3 DKCamera::ViewUp(void) const
{
	DKVector4 v = viewMatrix.Column2();
	return DKVector3(v.x, v.y, v.z).Normalize();
}

void DKCamera::SetPerspective(float fov, float aspect, float nz, float fz)
{
	DKASSERT_DEBUG( fov > 0 );
	DKASSERT_DEBUG( aspect > 0 );
	DKASSERT_DEBUG( nz > 0 );
	DKASSERT_DEBUG( fz > 0 );
	DKASSERT_DEBUG( fz > nz );

	float f = 1.0f / tan(fov / 2.0f);
	DKMatrix4 mat(
		f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, (fz + nz) / (nz - fz), -1.0f,
		0.0f, 0.0f, (2.0f * fz * nz) / (nz - fz), 0.0f);

	SetProjection(mat);
}

void DKCamera::SetOrthographic(float width, float height, float nz, float fz)
{
	DKASSERT_DEBUG( width > 0 );
	DKASSERT_DEBUG( height > 0 );
	DKASSERT_DEBUG( fz > nz );

	DKMatrix4 mat(
		2.0f / width, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / height, 0.0f, 0.0f,
		0.0f, 0.0f, 2.0f / (nz - fz), 0.0f,
		0.0f, 0.0f, (fz + nz) / (nz - fz), 1.0f);

	SetProjection(mat);
}

bool DKCamera::IsPerspective(void) const
{
	return projectionMatrix.m[3][3] != 1.0f;
}

bool DKCamera::IsOrthographic(void) const
{
	return projectionMatrix.m[3][3] == 1.0f;
}

bool DKCamera::IsPointInside(const DKVector3& point) const
{
	return IsSphereInside(point, 0);
}

bool DKCamera::IsSphereInside(const DKSphere& s) const
{
	if (s.IsValid())
		return IsSphereInside(s.center, s.radius);
	return false;
}

bool DKCamera::IsSphereInside(const DKVector3& center, float radius) const
{
	if (radius < 0)	return false;
	if (frustumNear.Dot(center) < -radius)		return false;
	if (frustumFar.Dot(center) < -radius)		return false;
	if (frustumLeft.Dot(center) < -radius)		return false;
	if (frustumRight.Dot(center) < -radius)		return false;
	if (frustumTop.Dot(center) < -radius)		return false;
	if (frustumBottom.Dot(center) < -radius)	return false;

	return true;
}

void DKCamera::SetView(const DKMatrix4& m)
{
	this->viewMatrix = m;
	this->UpdateFrustum();
}

void DKCamera::SetProjection(const DKMatrix4& m)
{
	this->projectionMatrix = m;
	this->UpdateFrustum();
}

void DKCamera::SetViewProjection(const DKMatrix4& v, const DKMatrix4& p)
{
	this->viewMatrix = v;
	this->projectionMatrix = p;
	this->UpdateFrustum();
}

void DKCamera::UpdateFrustum(void)	// update frustum planes
{
	////////////////////////////////////////////////////////////////////////////////
	// frustum planes
	//
	//         7+-------+4
	//         /|  far /|
	//        / |     / |
	//       /  |    /  |
	//      /  6+---/---+5 
	//     /   /   /   / 
	//   3+-------+0  /
	//    |  /    |  /
	//    | /     | /
	//    |/ near |/
	//   2+-------+1
	//

	DKVector3 vec[8];
	vec[0] = DKVector3( 1,  1, -1);		// near right top
	vec[1] = DKVector3( 1, -1, -1);		// near right bottom
	vec[2] = DKVector3(-1, -1, -1);		// near left bottom
	vec[3] = DKVector3(-1,  1, -1);		// near left top
	vec[4] = DKVector3( 1,  1,  1);		// far right top
	vec[5] = DKVector3( 1, -1,  1);		// far right bottom
	vec[6] = DKVector3(-1, -1,  1);		// far left bottom
	vec[7] = DKVector3(-1,  1,  1);		// far left top

	DKMatrix4 mat = viewMatrix * projectionMatrix;
	mat.Inverse();	// inversed vew projection matrix

	for (int i = 0; i < 8; i++)
	{
		vec[i] *= mat;
	}
	// far		(4,5,6,7)
	// near		(0,1,2,3)
	// top		(0,4,7,3)
	// bottom	(1,5,6,2)
	// right	(0,1,5,4)
	// left		(2,3,7,6)

	frustumFar =	DKPlane(vec[4], vec[7], vec[5]);
	frustumNear =	DKPlane(vec[3], vec[0], vec[2]);
	frustumTop =	DKPlane(vec[3], vec[7], vec[0]);
	frustumBottom =	DKPlane(vec[1], vec[5], vec[2]);
	frustumLeft =	DKPlane(vec[2], vec[6], vec[3]);
	frustumRight =	DKPlane(vec[0], vec[4], vec[1]);
}
