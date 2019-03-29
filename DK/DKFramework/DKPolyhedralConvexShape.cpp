//
//  File: DKPolyhedralConvexShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletPhysics.h"
#include "DKPolyhedralConvexShape.h"

using namespace DKFramework;
using namespace DKFramework::Private;

DKPolyhedralConvexShape::DKPolyhedralConvexShape(ShapeType t, btPolyhedralConvexShape* impl)
	: DKConvexShape(t, impl)
{
	DKASSERT_DEBUG( this->impl->isPolyhedral() );
}

DKPolyhedralConvexShape::~DKPolyhedralConvexShape()
{
	DKASSERT_DEBUG( this->impl->isPolyhedral() );
	DKASSERT_DEBUG( dynamic_cast<btPolyhedralConvexShape*>(this->impl) );
}

size_t DKPolyhedralConvexShape::NumberOfVertices() const
{
	return static_cast<btPolyhedralConvexShape*>(this->impl)->getNumVertices();
}

size_t DKPolyhedralConvexShape::NumberOfEdges() const
{
	return static_cast<btPolyhedralConvexShape*>(this->impl)->getNumEdges();
}

size_t DKPolyhedralConvexShape::NumberOfPlanes() const
{
	return static_cast<btPolyhedralConvexShape*>(this->impl)->getNumPlanes();
}

DKLine DKPolyhedralConvexShape::EdgeAtIndex(unsigned int index) const
{
	DKASSERT_DEBUG(index < NumberOfEdges());
	btVector3 pa, pb;
	static_cast<btPolyhedralConvexShape*>(this->impl)->getEdge(index, pa, pb);
	return DKLine(BulletVector3(pa), BulletVector3(pb));
}

DKVector3 DKPolyhedralConvexShape::VertexAtIndex(unsigned int index) const
{
	DKASSERT_DEBUG(index < NumberOfVertices());
	btVector3 vert;
	static_cast<btPolyhedralConvexShape*>(this->impl)->getVertex(index, vert);
	return BulletVector3(vert);
}

DKPlane DKPolyhedralConvexShape::PlaneAtIndex(unsigned int index) const
{
	DKASSERT_DEBUG(index < NumberOfPlanes());
	btVector3 planeNormal;
	btVector3 planeSupport;
	static_cast<btPolyhedralConvexShape*>(this->impl)->getPlane(planeNormal, planeSupport, index);

	//return DKPlane(BulletVector3(planeNormal), planeSupport.dot(planeNormal));
	return DKPlane(BulletVector3(planeNormal), BulletVector3(planeSupport));
}

bool DKPolyhedralConvexShape::IsPointInside(const DKVector3& p, float tolerance) const
{
	return static_cast<btPolyhedralConvexShape*>(this->impl)->isInside(BulletVector3(p), tolerance);
}
