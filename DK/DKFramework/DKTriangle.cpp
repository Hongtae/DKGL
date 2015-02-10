//
//  File: DKTriangle.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKTriangle.h"

using namespace DKFoundation;
using namespace DKFramework;

static_assert( sizeof(DKTriangle) == sizeof(DKVector3) * 3, "size mismatch?");

DKTriangle::DKTriangle(void)
{
}

DKTriangle::DKTriangle(const DKVector3& p1, const DKVector3& p2, const DKVector3& p3)
	: position1(p1)
	, position2(p2)
	, position3(p3)
{
}

DKTriangle::~DKTriangle(void)
{
}

bool DKTriangle::Intersect(const DKLine& line, Front front, DKVector3* p) const
{
	// calculate ray-tri intersection algorithm based on:
	// http://www.cs.lth.se/home/Tomas_Akenine_Moller/raytri/raytri.c
	// http://jgt.akpeters.com/papers/MollerTrumbore97/code.html

	const float epsilon = 0.000001f;

	// IntersectRayTriangleFont: ray test with front face of triangle.
	// if intersected, reference parameters set to u,v,t and return true.
	// u,v is positions inside of triangle.
	// t is distance between ray-start and hit position.
	struct
	{
		bool operator () (const DKVector3& rayStart, const DKVector3& rayDir, const DKVector3& tri0, const DKVector3& tri1, const DKVector3& tri2, DKVector3& uvt) const
		{
			DKVector3 edge1 = tri1 - tri0;
			DKVector3 edge2 = tri2 - tri0;
			DKVector3 p = DKVector3::Cross(rayDir, edge2);
			float det = DKVector3::Dot(edge1, p);

			if (det > -epsilon && det < epsilon)
				return false;

			DKVector3 s = rayStart - tri0;
			float u = DKVector3::Dot(s, p);
			if (u < 0.0f || u > det)
				return false;

			DKVector3 q = DKVector3::Cross(s, edge1);
			float v = DKVector3::Dot(rayDir, q);
			if (v < 0.0f || u+v > det)
				return false;

			uvt = DKVector3(u,v, DKVector3::Dot(edge2, q)) / det;
			return true;

		}
		const float epsilon;
	} IntersectRayTriangleFront = {epsilon};

	// IntersectRayTriangleFrontBack: ray test with both faces of triangle.
	// if intersected, reference parameters set to u,v,t and return true.
	// u,v is positions inside of triangle.
	// t is distance between ray-start and hit position.
	struct
	{
		bool operator () (const DKVector3& rayStart, const DKVector3& rayDir, const DKVector3& tri0, const DKVector3& tri1, const DKVector3& tri2, DKVector3& uvt) const
		{
			DKVector3 edge1 = tri1 - tri0;
			DKVector3 edge2 = tri2 - tri0;
			DKVector3 p = DKVector3::Cross(rayDir, edge2);
			float det = DKVector3::Dot(edge1, p);

			if (det > -epsilon && det < epsilon)
				return false;

			float inv_det = 1.0f / det;

			DKVector3 s = rayStart - tri0;
			float u = DKVector3::Dot(s, p) * inv_det;
			if (u < 0.0f || u > 1.0f)
				return false;

			DKVector3 q = DKVector3::Cross(s, edge1);
			float v = DKVector3::Dot(rayDir, q) * inv_det;
			if (v < 0.0f || u+v > 1.0)
				return false;

			uvt = DKVector3(u,v, DKVector3::Dot(edge2, q) * inv_det);
			return true;
		}
		const float epsilon;
	} IntersectRayTriangleFrontBack = {epsilon};

	DKVector3 lineDir = line.Direction();
	DKVector3 uvt(0,0,0);
	switch (front)
	{
	case DKTriangle::Front::Both:
		if (IntersectRayTriangleFrontBack(line.begin, lineDir, this->position1, this->position2, this->position3, uvt) &&
			uvt.z >= -0.0 && line.Length() >= uvt.z)
		{
			if (p)		*p = line.begin + (lineDir * uvt.z);
			return true;
		}
		break;
	case DKTriangle::Front::CW:
		if (IntersectRayTriangleFront(line.begin, lineDir, this->position3, this->position2, this->position1, uvt) &&
			uvt.z >= -0.0 && line.Length() >= uvt.z)
		{
			if (p)		*p = line.begin + (lineDir * uvt.z);
			return true;
		}
		break;
	case DKTriangle::Front::CCW:
		if (IntersectRayTriangleFront(line.begin, lineDir, this->position1, this->position2, this->position3, uvt) &&
			uvt.z >= -0.0 && line.Length() >= uvt.z)
		{
			if (p)		*p = line.begin + (lineDir * uvt.z);
			return true;
		}
		break;
	}

	return false;
}
