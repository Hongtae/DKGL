//
//  File: DKTriangleMeshProxyShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKTriangleMeshProxyShape.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;


struct DKTriangleMeshProxyShape::MeshInterface : public btStridingMeshInterface
{
	mutable DKAabb aabb;
	DKTriangleMeshProxyShape* shape;		// should be NULL, while invoking it's constructor.

	MeshInterface(void) : shape(NULL) {}

	// override from btStridingMeshInterface
	void getLockedVertexIndexBase(unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride, unsigned char **indexbase, int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart) override
	{
		*vertexbase = 0;
		numverts = 0;
		type = PHY_FLOAT;
		stride = 0;
		*indexbase = 0;
		indexstride = 0;
		numfaces = 0;
		indicestype = PHY_INTEGER;
	}
	void getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride, const unsigned char **indexbase, int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart) const override
	{
		if (shape)
		{
			MeshInfo info;
			memset(&info, 0, sizeof(info));

			shape->LockMeshPart(subpart, info);

			DKASSERT_DEBUG(info.indexSize == 2 || info.indexSize == 4);

			*vertexbase = (const unsigned char*)info.vertexBuffer;
			numverts = info.numVertices;
			type = PHY_FLOAT;
			stride = info.vertexStride;
			*indexbase = (const unsigned char*)info.indexBuffer;
			indexstride = info.indexStride;
			numfaces = info.numTriangles;
			indicestype = (info.indexSize == 4) ? PHY_INTEGER : PHY_SHORT;
		}
		else
		{
			*vertexbase = 0;
			numverts = 0;
			type = PHY_FLOAT;
			stride = 0;
			*indexbase = 0;
			indexstride = 0;
			numfaces = 0;
			indicestype = PHY_INTEGER;
		}
	}
	void unLockVertexBase(int subpart) override
	{
	}
	void unLockReadOnlyVertexBase(int subpart) const override
	{
		if (shape)
			shape->UnlockMeshPart(subpart);
	}
	int	getNumSubParts() const override
	{
		if (shape)
			return shape->NumberOfMeshParts();
		return 0;
	}
	bool hasPremadeAabb() const override
	{
		return aabb.IsValid();
	}
	void setPremadeAabb(const btVector3& aabbMin, const btVector3& aabbMax) const override
	{
		aabb.positionMin = BulletVector3(aabbMin);
		aabb.positionMax = BulletVector3(aabbMax);
	}
	void getPremadeAabb(btVector3* aabbMin, btVector3* aabbMax) const override
	{
		*aabbMin = BulletVector3(aabb.positionMin);
		*aabbMax = BulletVector3(aabb.positionMax);
	}
	void preallocateVertices(int numverts) override {}
	void preallocateIndices(int numindices) override {}
};

DKTriangleMeshProxyShape::DKTriangleMeshProxyShape(void)
: DKTriangleMeshProxyShape(new MeshInterface())
{
}

DKTriangleMeshProxyShape::DKTriangleMeshProxyShape(MeshInterface* mi)
: DKConcaveShape(ShapeType::Custom, new btBvhTriangleMeshShape(mi, true, false))
, meshInterface(mi)
{
	DKASSERT_DEBUG(meshInterface);
	meshInterface->shape = this;
}

DKTriangleMeshProxyShape::~DKTriangleMeshProxyShape(void)
{
	delete meshInterface;
}

void DKTriangleMeshProxyShape::Build(void)
{
	btVector3 aabbMin, aabbMax;
	meshInterface->calculateAabbBruteForce(aabbMin, aabbMax);
	meshInterface->setPremadeAabb(aabbMin, aabbMax);

	btBvhTriangleMeshShape* shape = static_cast<btBvhTriangleMeshShape*>(this->impl);

	const_cast<btVector3&>(shape->getLocalAabbMin()) = aabbMin;
	const_cast<btVector3&>(shape->getLocalAabbMax()) = aabbMax;

	shape->buildOptimizedBvh();
}

void DKTriangleMeshProxyShape::Refit(const DKAabb& aabb)
{
	btBvhTriangleMeshShape* shape = static_cast<btBvhTriangleMeshShape*>(this->impl);

	if (shape->getOwnsBvh())
	{
		if (aabb.IsValid())
			shape->refitTree(BulletVector3(aabb.positionMin), BulletVector3(aabb.positionMax));
		else
			shape->refitTree(BulletVector3(meshInterface->aabb.positionMin), BulletVector3(meshInterface->aabb.positionMax));
	}
	else
		Build();
}

void DKTriangleMeshProxyShape::PartialRefit(const DKAabb& aabb)
{
	btBvhTriangleMeshShape* shape = static_cast<btBvhTriangleMeshShape*>(this->impl);

	if (shape->getOwnsBvh())
	{
		if (aabb.IsValid())
			shape->partialRefitTree(BulletVector3(aabb.positionMin), BulletVector3(aabb.positionMax));
	}
	else
		Build();
}

DKAabb DKTriangleMeshProxyShape::Aabb(void) const
{
	return meshInterface->aabb;
}
