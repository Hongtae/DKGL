//
//  File: DKStaticTriangleMeshShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKStaticTriangleMeshShape.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;

class DKStaticTriangleMeshShape::IndexedTriangleData : public btStridingMeshInterface
{
public:
	int numTriangles;
	void* vertices;
	size_t numVertices;
	void* indices;
	size_t numIndices;
	PHY_ScalarType indexType;

	~IndexedTriangleData(void)
	{
		if (vertices)
			free(vertices);
		if (indices)
			free(indices);
	}

	static DKSpinLock lock;
	template <typename IndexType>
	IndexedTriangleData(const DKVector3* verts, size_t numVerts,
		const IndexType* indices, size_t numIndices,
		const DKAABox& aabb, bool rebuild, float weld)
		: vertices(NULL)
		, numVertices(0)
		, indices(NULL)
		, numIndices(0)
		, indexType(PHY_INTEGER)
		, aabbMin(BulletVector3(aabb.positionMin))
		, aabbMax(BulletVector3(aabb.positionMax))
	{
		this->numTriangles = (int)(numIndices / 3);
		if (numIndices % 3)
			numIndices -= numIndices % 3;

		if (this->numTriangles == 0 || verts == NULL || numVerts == 0 || indices == 0)
			return;

		if (rebuild)
		{
			DKCriticalSection<DKSpinLock> guard(lock); // for protect weldingThreshold

			static float weldingThreshold;
			weldingThreshold = weld * weld;

			struct Vert
			{
				Vert(const DKVector3& v) : pos(v), lengthSq(v.LengthSq()) {}
				const DKVector3& pos;
				float lengthSq;

				// compare if longer distance than weldingThreshold
				bool operator > (const Vert& rhs) const
				{
					if ((this->pos - rhs.pos).LengthSq() > weldingThreshold)
					{
						if (this->pos.x == rhs.pos.x)
						{
							if (this->pos.y == rhs.pos.y)
								return this->pos.z > rhs.pos.z;
							else
								return this->pos.y > rhs.pos.y;
						}
						else
							return this->pos.x > rhs.pos.x;
					}
					return false;
				}
				bool operator < (const Vert& rhs) const
				{
					if ((this->pos - rhs.pos).LengthSq() > weldingThreshold)
					{
						if (this->pos.x == rhs.pos.x)
						{
							if (this->pos.y == rhs.pos.y)
								return this->pos.z < rhs.pos.z;
							else
								return this->pos.y < rhs.pos.y;
						}
						else
							return this->pos.x < rhs.pos.x;
					}
					return false;
				}
			};

			DKMap<Vert, unsigned int> VertIndexMap;
			DKArray<unsigned int> indices2;
			DKArray<DKVector3> vertices2;
			indices2.Reserve(numIndices);
			vertices2.Reserve(numVerts);

			for (size_t i = 0; i < numIndices; ++i)
			{
				unsigned int index;
				const DKVector3& v = verts[indices[i]];
				auto p = VertIndexMap.Find(v);
				if (p)
					index = p->value;
				else
				{
					index = (unsigned int)vertices2.Count();
					vertices2.Add(v);
					VertIndexMap.Insert(v, index);
				}
				indices2.Add(index);
			}

			this->numVertices = vertices2.Count();
			this->vertices = new DKVector3[this->numVertices];
			memcpy(this->vertices, (const DKVector3*)vertices2, this->numVertices * sizeof(DKVector3));
			this->numIndices = indices2.Count();

			if (vertices2.Count() > 0xFFFF)
			{
				this->indexType = PHY_INTEGER;
				this->indices = malloc(this->numIndices * sizeof(unsigned int));
				for (size_t i = 0; i < this->numIndices; ++i)
					reinterpret_cast<unsigned int*>(this->indices)[i] = indices2.Value(i);
			}
			else
			{
				this->indexType = PHY_SHORT;
				this->indices = malloc(this->numIndices * sizeof(unsigned short));
				for (size_t i = 0; i < this->numIndices; ++i)
					reinterpret_cast<unsigned short*>(this->indices)[i] = indices2.Value(i);
			}
			this->calculateAabbBruteForce(this->aabbMin, this->aabbMax);
		}
		else
		{
			if (sizeof(IndexType) == 4)
				this->indexType = PHY_INTEGER;
			else
				this->indexType = PHY_SHORT;

			this->numVertices = numVerts;
			this->vertices = malloc(numVerts * sizeof(DKVector3));
			this->numIndices = numIndices;
			this->indices = malloc(numIndices * sizeof(IndexType));

			memcpy(this->vertices, verts, numVerts * sizeof(DKVector3));
			memcpy(this->indices, indices, numIndices * sizeof(IndexType));

			if (this->aabbMax.x() < this->aabbMin.x() || this->aabbMax.y() < this->aabbMin.y() || this->aabbMax.z() < this->aabbMin.z())
				this->calculateAabbBruteForce(this->aabbMin, this->aabbMax);
		}
	}

	// override from btStridingMeshInterface
	void getLockedVertexIndexBase(unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride, unsigned char **indexbase, int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart) override
	{
		return getLockedReadOnlyVertexIndexBase((const unsigned char**)vertexbase, numverts, type, stride, (const unsigned char**)indexbase, indexstride, numfaces, indicestype, subpart);
	}
	void getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride, const unsigned char **indexbase, int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart) const override
	{
		DKASSERT_DEBUG(subpart == 0);
		(*vertexbase) = (const unsigned char*)this->vertices;
		numverts = (int)this->numVertices;
		type = PHY_FLOAT;
		stride = sizeof(DKVector3);
		numfaces = this->numTriangles;
		(*indexbase) = (unsigned char*)this->indices;
		if (this->indexType == PHY_INTEGER)
			indexstride = 3 * sizeof(unsigned int);
		else
			indexstride = 3 * sizeof(unsigned short);
		indicestype = this->indexType;
	}
	void unLockVertexBase(int subpart) override {}
	void unLockReadOnlyVertexBase(int subpart) const override {}
	int	getNumSubParts() const override { return 1; }
	void preallocateVertices(int numverts) override {}
	void preallocateIndices(int numindices) override {}
	bool hasPremadeAabb() const override { return true; }
	void setPremadeAabb(const btVector3& aabbMin, const btVector3& aabbMax) const override
	{
		this->aabbMin = aabbMin;
		this->aabbMax = aabbMax;
	}
	void getPremadeAabb(btVector3* aabbMin, btVector3* aabbMax) const override
	{
		*aabbMin = this->aabbMin;
		*aabbMax = this->aabbMax;
	}

	mutable btVector3 aabbMin;
	mutable btVector3 aabbMax;
};
DKSpinLock DKStaticTriangleMeshShape::IndexedTriangleData::lock;

DKStaticTriangleMeshShape::DKStaticTriangleMeshShape(
	const DKVector3* verts, size_t numVerts,
	const unsigned int* indices, size_t numIndices,
	const DKAABox& precalculatedAABB, bool rebuildIndex, float weldingThreshold)
	: DKStaticTriangleMeshShape(new IndexedTriangleData(verts, numVerts, indices, numIndices,
	precalculatedAABB, rebuildIndex, weldingThreshold))
{
}

DKStaticTriangleMeshShape::DKStaticTriangleMeshShape(
	const DKVector3* verts, size_t numVerts,
	const unsigned short* indices, size_t numIndices,
	const DKAABox& precalculatedAABB, bool rebuildIndex, float weldingThreshold)
	: DKStaticTriangleMeshShape(new IndexedTriangleData(verts, numVerts, indices, numIndices,
	precalculatedAABB, rebuildIndex, weldingThreshold))
{
}

DKStaticTriangleMeshShape::DKStaticTriangleMeshShape(IndexedTriangleData* data)
: DKConcaveShape(ShapeType::StaticTriangleMesh, new btBvhTriangleMeshShape(data, true, true))
, meshData(data)
{
}

DKStaticTriangleMeshShape::~DKStaticTriangleMeshShape(void)
{
	delete meshData;
}

void DKStaticTriangleMeshShape::RefitBVH(const DKAABox& aabb)
{
	btBvhTriangleMeshShape* shape = static_cast<btBvhTriangleMeshShape*>(this->impl);
	shape->refitTree(BulletVector3(aabb.positionMin), BulletVector3(aabb.positionMax));
}

void DKStaticTriangleMeshShape::PartialRefitBVH(const DKAABox& aabb)
{
	btBvhTriangleMeshShape* shape = static_cast<btBvhTriangleMeshShape*>(this->impl);
	shape->partialRefitTree(BulletVector3(aabb.positionMin), BulletVector3(aabb.positionMax));
}

DKVector3* DKStaticTriangleMeshShape::VertexBuffer(size_t* numVerts)
{
	if (numVerts)
		*numVerts = this->meshData->numVertices;
	return (DKVector3*)this->meshData->vertices;
}

const DKVector3* DKStaticTriangleMeshShape::VertexBuffer(size_t* numVerts) const
{
	if (numVerts)
		*numVerts = this->meshData->numVertices;
	return (DKVector3*)this->meshData->vertices;
}

const void* DKStaticTriangleMeshShape::IndexBuffer(size_t* numIndices, size_t* indexSize) const
{
	if (numIndices)
		*numIndices = this->meshData->numIndices;
	if (indexSize)
	{
		if (this->meshData->indexType == PHY_INTEGER)
			*indexSize = 4;
		else
			*indexSize = 2;
	}
	return this->meshData->indices;
}

DKAABox DKStaticTriangleMeshShape::MeshAABB(void) const
{
	return DKAABox(BulletVector3(this->meshData->aabbMin), BulletVector3(this->meshData->aabbMax));
}

size_t DKStaticTriangleMeshShape::NumberOfTriangles(void) const
{
	return this->meshData->numTriangles;
}

bool DKStaticTriangleMeshShape::GetTriangleVertexIndices(int triangle, unsigned int* index) const
{
	size_t numTriangles = this->meshData->numTriangles;
	if (triangle >= 0 && triangle < numTriangles)
	{
		if (this->meshData->indexType == PHY_INTEGER)
		{
			unsigned int* idx = &reinterpret_cast<unsigned int*>(this->meshData->indices)[triangle * 3];
			index[0] = idx[0];
			index[1] = idx[1];
			index[2] = idx[2];
		}
		else
		{
			unsigned short* idx = &reinterpret_cast<unsigned short*>(this->meshData->indices)[triangle * 3];
			index[0] = idx[0];
			index[1] = idx[1];
			index[2] = idx[2];
		}
		return true;
	}
	return false;
}

bool DKStaticTriangleMeshShape::GetTriangleFace(int index, DKTriangle& triangle) const
{
	size_t numTriangles = this->meshData->numTriangles;
	if (index >= 0 && index < numTriangles)
	{
		if (this->meshData->indexType == PHY_INTEGER)
		{
			unsigned int* idx = &reinterpret_cast<unsigned int*>(this->meshData->indices)[index * 3];
			triangle.position1 = reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]];
			triangle.position2 = reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]];
			triangle.position3 = reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]];
		}
		else
		{
			unsigned short* idx = &reinterpret_cast<unsigned short*>(this->meshData->indices)[index * 3];
			triangle.position1 = reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]];
			triangle.position2 = reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]];
			triangle.position3 = reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]];
		}
		return true;
	}
	return false;
}

bool DKStaticTriangleMeshShape::SetTriangleFace(int index, const DKTriangle& triangle)
{
	size_t numTriangles = this->meshData->numTriangles;
	if (index >= 0 && index < numTriangles)
	{
		if (this->meshData->indexType == PHY_INTEGER)
		{
			unsigned int* idx = &reinterpret_cast<unsigned int*>(this->meshData->indices)[index * 3];
			reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]] = triangle.position1;
			reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]] = triangle.position2;
			reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]] = triangle.position3;
		}
		else
		{
			unsigned short* idx = &reinterpret_cast<unsigned short*>(this->meshData->indices)[index * 3];
			reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]] = triangle.position1;
			reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]] = triangle.position2;
			reinterpret_cast<DKVector3*>(this->meshData->vertices)[idx[0]] = triangle.position3;
		}
		return true;
	}
	return false;

}