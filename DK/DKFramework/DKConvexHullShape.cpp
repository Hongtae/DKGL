//
//  File: DKConvexHullShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKConvexHullShape.h"
#include "DKTriangle.h"

namespace DKFramework
{
	// convex decomposition calculated by using HACD
	// https://github.com/kmammou/v-hacd

	namespace Private
	{
		typedef HACD::Vec3<HACD::Real> HACDPoint;
		typedef HACD::Vec3<long> HACDTriangle;

		struct HACDCluster
		{
			btConvexHullShape* shape;
			btVector3 centroid;
		};

#define HACD_CLUSTER_CALCULATE_CENTROID		// calculate centroid (center of mass)

		HACDCluster CreateConvexHullShapeHACD(HACD::HACD& hacd, size_t cluster, const btVector3& localScaling)
		{
			DKASSERT_DEBUG(cluster < hacd.GetNClusters());

			// create convex-hull per cluster.
			size_t numPoints = hacd.GetNPointsCH(cluster);
			size_t numTriangles = hacd.GetNTrianglesCH(cluster);

			HACDPoint* points = DKRawPtrNewArray<HACDPoint>(numPoints);
			HACDTriangle* triangles = DKRawPtrNewArray<HACDTriangle>(numTriangles);
			hacd.GetCH(cluster, points, triangles);

			btVector3 centroid(0, 0, 0);

#ifdef HACD_CLUSTER_CALCULATE_CENTROID
			for (size_t i = 0; i < numPoints; ++i)
			{
				btVector3 vertex( points[i].X(), points[i].Y(), points[i].Z() );
				vertex *= localScaling;
				centroid += vertex;
			}

			centroid *=  1.0f / static_cast<float>( numPoints );
#endif

			btAlignedObjectArray<btVector3> vertices;
			vertices.reserve(numPoints);

			for (size_t i = 0; i < numPoints; ++i)
			{
				btVector3 vertex( points[i].X(), points[i].Y(), points[i].Z() );
				vertex *= localScaling;
				vertex -= centroid;
				vertices.push_back(vertex);
			}
			DKRawPtrDeleteArray(points, numPoints);
			DKRawPtrDeleteArray(triangles, numTriangles);

			HACDCluster result;
			result.shape = DKRawPtrNew<btConvexHullShape>(&(vertices[0].getX()), vertices.size());
			result.shape->setMargin(0.01f);
			result.centroid = centroid;

			return result;
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

static_assert( sizeof(DKVector3) == sizeof(float)*3, "DKVector3 must be float[3]");

DKConvexHullShape::DKConvexHullShape(const DKVector3* verts, size_t numVerts)
	: DKPolyhedralConvexShape(ShapeType::ConvexHull,
	(verts && numVerts > 0) ? DKRawPtrNew<btConvexHullShape>(&(verts[0].val[0]), numVerts, sizeof(DKVector3)) : DKRawPtrNew<btConvexHullShape>())
{
}

DKConvexHullShape::DKConvexHullShape(ShapeType t, class btConvexHullShape* impl)
	: DKPolyhedralConvexShape(t, impl)
{
	DKASSERT_DEBUG(impl);
}

DKConvexHullShape::~DKConvexHullShape(void)
{
}

void DKConvexHullShape::AddPoint(const DKVector3& p)
{
	static_cast<btConvexHullShape*>(this->impl)->addPoint(BulletVector3(p));
}

size_t DKConvexHullShape::NumberOfPoints(void) const
{
	return static_cast<btConvexHullShape*>(this->impl)->getNumPoints();
}

DKVector3 DKConvexHullShape::PointAtIndex(unsigned int index) const
{
	DKASSERT_DEBUG(index < NumberOfPoints());
	return BulletVector3(static_cast<btConvexHullShape*>(this->impl)->getUnscaledPoints()[index]);
}

DKVector3 DKConvexHullShape::ScaledPointAtIndex(unsigned int index) const
{
	DKASSERT_DEBUG(index < NumberOfPoints());
	return BulletVector3(static_cast<btConvexHullShape*>(this->impl)->getScaledPoint(index));
}

DKObject<DKConvexHullShape> DKConvexHullShape::CreateHull(const DKTriangle* tri, size_t num)
{
	if (tri == NULL || num == 0)
		return NULL;

	btTriangleMesh* mesh = DKRawPtrNew<btTriangleMesh>();

	for (size_t i = 0; i < num; ++i)
	{
		btVector3 v[3] = {
			BulletVector3(tri[i].position1),
			BulletVector3(tri[i].position2),
			BulletVector3(tri[i].position3)
		};

		mesh->addTriangle(v[0], v[1], v[2]);
	}

	btConvexShape* convexShape = DKRawPtrNew<btConvexTriangleMeshShape>(mesh);
	btShapeHull* shapeHull = DKRawPtrNew<btShapeHull>(convexShape);

	btScalar margin = convexShape->getMargin();
	shapeHull->buildHull(margin);

	btConvexHullShape* convexHullShape = DKRawPtrNew<btConvexHullShape>();
	for (int i = 0; i < shapeHull->numVertices(); ++i)
	{
		convexHullShape->addPoint(shapeHull->getVertexPointer()[i]);
	}

	DKRawPtrDelete(shapeHull);
	DKRawPtrDelete(convexShape);
	DKRawPtrDelete(mesh);

	return DKOBJECT_NEW DKConvexHullShape(ShapeType::ConvexHull, convexHullShape);
}

DKConvexHullShape::ConvexHullArray DKConvexHullShape::DecomposeTriangleMesh(
	const DKVector3* verts,
	size_t numVerts,
	const long* indices,
	size_t numIndices,
	size_t minClusters,
	size_t maxVertsPerCH,
	double maxConcavity,
	bool addExtraDistPoints,
	bool addNeighboursDistPoints,
	bool addFacesPoints)
{
	ConvexHullArray result;

	size_t numTriangles = numIndices / 3;
	if (verts && numVerts > 0 && indices && numTriangles > 0)
	{
		DKArray<HACDPoint> points;
		DKArray<HACDTriangle> triangles;

		points.Reserve(numVerts);
		triangles.Reserve(numTriangles);

		for (size_t i = 0; i < numVerts; ++i)
		{
			points.Add(HACDPoint(verts[i].x, verts[i].y, verts[i].z));
		}
		for (size_t i = 0; i < numTriangles; ++i)
		{
			triangles.Add(HACDTriangle(indices[i*3], indices[i*3+1], indices[i*3+2]));
		}

		HACD::HACD hacd;

		hacd.SetPoints(points);
		hacd.SetNPoints(points.Count());
		hacd.SetTriangles(triangles);
		hacd.SetNTriangles(triangles.Count());
		hacd.SetCompacityWeight(0.1);
		hacd.SetVolumeWeight(0.0);

		hacd.SetNClusters(minClusters);
		hacd.SetNVerticesPerCH(maxVertsPerCH);
		hacd.SetConcavity(maxConcavity);
		hacd.SetAddExtraDistPoints(addExtraDistPoints);
		hacd.SetAddNeighboursDistPoints(addNeighboursDistPoints);
		hacd.SetAddFacesPoints(addFacesPoints);

		hacd.Compute();

		size_t numClusters = hacd.GetNClusters();
		result.Reserve(numClusters);

		for (size_t i = 0; i < numClusters; ++i)
		{
			HACDCluster cluster = CreateConvexHullShapeHACD(hacd, i, btVector3(1,1,1));

			DKASSERT_DEBUG(cluster.shape);

			ConvexHull res = {
				DKOBJECT_NEW DKConvexHullShape(ShapeType::ConvexHull, cluster.shape),
				BulletVector3(cluster.centroid)
			};
			result.Add(res);
		}
	}
	return result;
}
