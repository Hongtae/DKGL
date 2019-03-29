//
//  File: DKBox.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKBox.h"
#include "DKLine.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKMatrix4.h"
#include "DKAabb.h"

using namespace DKFramework;

DKBox::DKBox()
	: center(0,0,0), u(0,0,0), v(0,0,0), w(0,0,0), hu(-FLT_MAX), hv(-FLT_MAX), hw(-FLT_MAX)
{
}

DKBox::DKBox(const DKVector3& c, const DKVector3& x, const DKVector3& y, const DKVector3& z)
	: center(0,0,0), u(0,0,0), v(0,0,0), w(0,0,0), hu(-FLT_MAX), hv(-FLT_MAX), hw(-FLT_MAX)
{
	center = c;
	DKVector3 u1 = x - c;
	DKVector3 v1 = y - c;
	DKVector3 w1 = z - c;

	u = DKVector3(u1).Normalize();
	v = DKVector3(v1).Normalize();
	w = DKVector3(w1).Normalize();
	hu = u1.Length();
	hv = v1.Length();
	hw = w1.Length();
}

DKBox::DKBox(const DKVector3& c, const DKVector3& uu, const DKVector3& uv, float hx, float hy, float hz)
	: center(0,0,0), u(0,0,0), v(0,0,0), w(0,0,0), hu(-FLT_MAX), hv(-FLT_MAX), hw(-FLT_MAX)
{
	center = c;
	u = DKVector3(uu).Normalize();
	v = DKVector3(uv).Normalize();
	w = DKVector3::Cross(u, v).Normalize();
	hu = hx;
	hv = hy;
	hw = hz;
}

DKBox::DKBox(const DKVector3& c, const DKVector3& uu, const DKVector3& uv, const DKVector3& uw, float hx, float hy, float hz)
	: center(c)
	, u(DKVector3(uu).Normalize())
	, v(DKVector3(uv).Normalize())
	, w(DKVector3(uw).Normalize())
	, hu(hx), hv(hy), hw(hz)
{
}

DKBox::DKBox(const DKMatrix4& m)
	: center(0,0,0), u(0,0,0), v(0,0,0), w(0,0,0), hu(-FLT_MAX), hv(-FLT_MAX), hw(-FLT_MAX)
{
	center = DKVector3(0,0,0).Transform(m);
	DKVector3 x = DKVector3(1,0,0).Transform(m) - center;
	DKVector3 y = DKVector3(0,1,0).Transform(m) - center;
	DKVector3 z = DKVector3(0,0,1).Transform(m) - center;

	u = DKVector3(x).Normalize();
	v = DKVector3(y).Normalize();
	w = DKVector3(z).Normalize();
	hu = x.Length();
	hv = y.Length();
	hw = z.Length();
}

bool DKBox::IsValid() const
{
	return (hu > 0.0 && hv > 0.0 && hw > 0.0);
}

float DKBox::Volume() const
{
	if (hu > 0.0 && hv > 0.0 && hw > 0.0)
		return hu * hv * hw * 8.0f;
	return 0.0f;
}

bool DKBox::IsPointInside(const DKVector3& pos) const
{
	if (this->IsValid())
	{
		DKMatrix4 tm = this->LocalTransform();
		tm.Inverse();

		DKVector3 p = DKVector3(pos).Transform(tm);

		if (p.x >= -hu && p.x <= hu && p.y >= -hv && p.y <= hv && p.z >= -hw && p.z <= hw)
			return true;
	}
	return false;
}

DKMatrix4 DKBox::LocalTransform() const
{
	return DKMatrix4(
		DKVector4(u.x * hu, u.y * hu, u.z * hu, 0),
		DKVector4(v.x * hv, v.y * hv, v.z * hv, 0),
		DKVector4(w.x * hw, w.y * hw, w.z * hw, 0),
		DKVector4(center.x, center.y, center.z, 1));
}

DKMatrix4 DKBox::AffineTransform() const
{
	return DKMatrix4(
		DKVector4(u.x, u.y, u.z, 0),
		DKVector4(v.x, v.y, v.z, 0),
		DKVector4(w.x, w.y, w.z, 0),
		DKVector4(center.x, center.y, center.z, 1));
}

DKQuaternion DKBox::Orientation() const
{
	// rotate by axis-x
	return DKQuaternion(DKVector3(0,1,0) * DKQuaternion(DKVector3(1,0,0), u, 1.0f), v, 1.0f);
}

DKBox& DKBox::RotateOrientation(const DKQuaternion& q)
{
	u.Rotate(q);
	v.Rotate(q);
	w.Rotate(q);
	return *this;
}

bool DKBox::RayTest(const DKLine& ray, DKVector3* p) const
{
	if (this->IsValid())
	{
		// calculate as AABB
		DKMatrix4 boxTM = this->LocalTransform();
		DKMatrix4 invTM = DKMatrix4(boxTM).Inverse();

		DKVector3 begin = ray.begin * invTM;
		DKVector3 end = ray.begin * invTM;

		if (DKAabb(DKVector3(-1,-1,-1), DKVector3(1,1,1)).RayTest(DKLine(begin, end), p))
		{
			if (p)
				p->Transform(boxTM);

			return true;
		}
	}
	return false;
}
