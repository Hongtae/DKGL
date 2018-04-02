//
//  File: DKConvexHullShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletPhysics.h"
#include "DKConvexHullShape.h"
#include "DKTriangle.h"

using namespace DKFramework;
using namespace DKFramework::Private;

static_assert( sizeof(DKVector3) == sizeof(float)*3, "DKVector3 must be float[3]");

DKConvexHullShape::DKConvexHullShape(const DKVector3* verts, size_t numVerts)
	: DKPolyhedralConvexShape(ShapeType::ConvexHull,
	(verts && numVerts > 0) ? new btConvexHullShape(&(verts[0].val[0]), numVerts, sizeof(DKVector3)) : new btConvexHullShape())
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

	btTriangleMesh* mesh = new btTriangleMesh();

	for (size_t i = 0; i < num; ++i)
	{
		btVector3 v[3] = {
			BulletVector3(tri[i].position1),
			BulletVector3(tri[i].position2),
			BulletVector3(tri[i].position3)
		};

		mesh->addTriangle(v[0], v[1], v[2]);
	}

	btConvexShape* convexShape = new btConvexTriangleMeshShape(mesh);
	btShapeHull* shapeHull = new btShapeHull(convexShape);

	btScalar margin = convexShape->getMargin();
	shapeHull->buildHull(margin);

	btConvexHullShape* convexHullShape = new btConvexHullShape();
	for (int i = 0; i < shapeHull->numVertices(); ++i)
	{
		convexHullShape->addPoint(shapeHull->getVertexPointer()[i]);
	}

	delete shapeHull;
	delete convexShape;
	delete mesh;

	return DKOBJECT_NEW DKConvexHullShape(ShapeType::ConvexHull, convexHullShape);
}
