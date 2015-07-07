//
//  File: DKBvh.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKBvh.h"

using namespace DKFoundation;
using namespace DKFramework;


DKBvh::DKBvh(void) : mesh(NULL)
{
}

DKBvh::~DKBvh(void)
{
}

void DKBvh::Build(DKTriangleMesh* mesh)
{
	this->mesh = mesh;
	BuildInternal();
}

void DKBvh::Rebuild(void)
{
	BuildInternal();
}

void DKBvh::BuildInternal(void)
{
	if (this->mesh)
	{
		this->mesh->Lock();
		nodes.Clear();

		DKArray<LeafNode> leafNodes;

		// Query all leaf-nodes (all triangles)
		int numTriangles = this->mesh->NumberOfTriangles();
		if (numTriangles > 0)
		{
			DKAabb aabb;
			aabb.positionMin = DKVector3(FLT_MAX, FLT_MAX, FLT_MAX);
			aabb.positionMax = DKVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			DKTriangle tri;
			leafNodes.Reserve(numTriangles);
			for (int i = 0; i < numTriangles; ++i)
			{
				if (this->mesh->GetTriangleAtIndex(i, tri))
				{
					LeafNode info = { tri.Aabb(), i };
					leafNodes.Add(info);

					// Update AABB
					for (int k = 0; k < 3; ++k)
					{
						if (aabb.positionMin.val[k] > info.aabb.positionMin.val[k])
							aabb.positionMin.val[k] = info.aabb.positionMin.val[k];
						if (aabb.positionMax.val[k] < info.aabb.positionMax.val[k])
							aabb.positionMax.val[k] = info.aabb.positionMax.val[k];
					}
				}
			}

			this->aabbOffset = aabb.positionMin;
			this->aabbScale = aabb.positionMax - aabb.positionMin;
		}

		if (leafNodes.Count() > 0)
		{
			this->aabbScale.x = Max(this->aabbScale.x, 0.00001);
			this->aabbScale.y = Max(this->aabbScale.y, 0.00001);
			this->aabbScale.z = Max(this->aabbScale.z, 0.00001);

			for (LeafNode& n : leafNodes)
			{
				n.aabb.positionMin = (n.aabb.positionMin - this->aabbOffset) / this->aabbScale * float(0xffff);
				n.aabb.positionMax = (n.aabb.positionMax - this->aabbOffset) / this->aabbScale * float(0xffff);
			}

			nodes.Reserve(leafNodes.Count() * 2);
			BuildTree(leafNodes, leafNodes.Count());
		}

		this->mesh->Unlock();
	}
	else
		nodes.Clear();
}

void DKBvh::BuildTree(LeafNode* leafNodes, int count)
{
	DKASSERT_DEBUG(leafNodes);
	DKASSERT_DEBUG(count > 0);

	if (count == 1)	// leaf-node
	{
		QuantizedAabbNode node;
		for (int i = 0; i < 3; ++i)
		{
			node.aabbMin[i] = leafNodes[0].aabb.positionMin.val[i];
			node.aabbMax[i] = leafNodes[0].aabb.positionMax.val[i];
		}
		node.triangleIndex = leafNodes[0].triangleIndex;
		this->nodes.Add(node);
		return;
	}

	int currentNodeIndex = nodes.Count();

	DKVector3 means(0, 0, 0);
	for (int i = 0; i < count; ++i)
	{
		means += leafNodes[i].aabb.Center();
	}
	means /= float(count);

	// calculate axis (0: x-axis, 1: y-axis, 2: z-axis, DKVector3::val order)
	int splitAxis = [&]()->int
	{
		DKVector3 variance(0, 0, 0);

		for (int i = 0; i < count; ++i)
		{
			DKVector3 center = leafNodes[i].aabb.Center();
			DKVector3 diff = center - means;
			// abs
			variance.x += diff.x > 0.0f ? diff.x : diff.x * -1.0f;
			variance.y += diff.y > 0.0f ? diff.y : diff.y * -1.0f;
			variance.z += diff.z > 0.0f ? diff.z : diff.z * -1.0f;
		}
		return variance.val[0] < variance.val[1] ? (variance.val[1] < variance.val[2] ? 2 : 1) : (variance.val[0] < variance.val[2] ? 2 : 0);
	}();

	// splitting
	float splitValue = means.val[splitAxis];

	int splitIndex = 0;
	// sort leaf-nodes, place larger than splitValue to left.
	for (int i = 0; i < count; ++i)
	{
		DKVector3 center = leafNodes[i].aabb.Center();
		if (center.val[splitAxis] > splitValue)
		{
			LeafNode tmp = leafNodes[i];
			leafNodes[i] = leafNodes[splitIndex];
			leafNodes[splitIndex] = tmp;
			splitIndex++;
		}
	}
	// recursion
}

bool DKBvh::RayTest(const DKLine& ray, DKVector3* hitPoint) const
{
	return false;
}

