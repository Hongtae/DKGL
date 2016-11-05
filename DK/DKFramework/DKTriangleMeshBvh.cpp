//
//  File: DKTriangleMeshBvh.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKTriangleMeshBvh.h"

using namespace DKGL;

DKTriangleMeshBvh::DKTriangleMeshBvh(void) : mesh(NULL)
{
}

DKTriangleMeshBvh::~DKTriangleMeshBvh(void)
{
}

DKAabb DKTriangleMeshBvh::Aabb(void) const
{
	return bvh.Aabb();
}

void DKTriangleMeshBvh::Build(DKTriangleMesh* m)
{
	struct TriangleAabb : public DKBvh::VolumeInterface
	{
		int NumberOfObjects(void) const override
		{
			return mesh->NumberOfTriangles();
		}
		DKAabb AabbForObjectAtIndex(int index) override
		{
			DKTriangle tri;
			if (mesh->GetTriangleAtIndex(index, tri))
				return tri.Aabb();
			return DKAabb();
		}
		void Lock(void) override
		{
			mesh->Lock();
		}
		void Unlock(void) override
		{
			mesh->Unlock();
		}
		DKTriangleMesh* mesh;
	};
	DKObject<TriangleAabb> vol = DKOBJECT_NEW TriangleAabb();
	this->mesh = m;
	vol->mesh = this->mesh;
	bvh.Build(vol.SafeCast<DKBvh::VolumeInterface>());
}

void DKTriangleMeshBvh::Rebuild(void)
{
	bvh.Rebuild();
}

bool DKTriangleMeshBvh::RayTest(const DKLine& ray, DKVector3* hitPoint) const
{
	if (this->mesh)
	{
		bool hit = false;
		bool queryClosestHitPoint = hitPoint != NULL;
		float closestHitFraction = FLT_MAX;
		DKTriangle tri;
		DKVector3 tmp;

		auto triangleRayTest = [&](int index, const DKLine& ray)->bool
		{
			if (this->mesh->GetTriangleAtIndex(index, tri))
			{
				if (tri.RayTest(ray, &tmp))
				{
					hit = true;
					if (queryClosestHitPoint)
					{
						float f = (tmp - ray.begin).LengthSq();
						if (closestHitFraction > f)
							closestHitFraction = f;
					}
					return queryClosestHitPoint;
				}
			}
			return true;
		};

		const_cast<DKTriangleMeshBvh*>(this)->mesh->Lock();
		this->bvh.RayTest(ray, DKFunction(triangleRayTest));
		const_cast<DKTriangleMeshBvh*>(this)->mesh->Unlock();

		if (hit)	// set by triangleRayTest()
		{
			if (hitPoint)
				*hitPoint = ray.begin + ray.Direction() * sqrt(closestHitFraction);
			return true;
		}
	}
	return false;
}
