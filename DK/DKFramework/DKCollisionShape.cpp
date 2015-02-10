//
//  File: DKCollisionShape.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKCollisionShape.h"
#include "DKAffineTransform3.h"
#include "DKLinearTransform3.h"

#include "DKCompoundShape.h"
#include "DKBoxShape.h"
#include "DKCapsuleShape.h"
#include "DKCylinderShape.h"
#include "DKConeShape.h"
#include "DKSphereShape.h"
#include "DKMultiSphereShape.h"
#include "DKConvexHullShape.h"
#include "DKStaticPlaneShape.h"
#include "DKStaticTriangleMeshShape.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;

DKCollisionShape::DKCollisionShape(ShapeType t, btCollisionShape* c)
	: impl(c)
	, type(t)
{
	impl->setUserPointer(this);
}

DKCollisionShape::~DKCollisionShape(void)
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);
	delete impl;
}

DKAABox DKCollisionShape::AABB(const DKNSTransform& transform) const
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);

	btVector3 vmin(0,0,0);
	btVector3 vmax(0,0,0);

	impl->getAabb(BulletTransform(transform), vmin, vmax);

	return DKAABox(BulletVector3(vmin), BulletVector3(vmax));
}

DKSphere DKCollisionShape::BoundingSphere(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);

	btVector3 center;
	btScalar radius = 0;

	impl->getBoundingSphere(center, radius);

	return DKSphere(BulletVector3(center), radius);
}

void DKCollisionShape::AABB(const DKNSTransform& transform, DKVector3& aabbMin, DKVector3& aabbMax) const
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);

	btVector3 vmin(0,0,0);
	btVector3 vmax(0,0,0);

	impl->getAabb(BulletTransform(transform), vmin, vmax);
	aabbMin = BulletVector3(vmin);
	aabbMax = BulletVector3(vmax);
}

void DKCollisionShape::SetMargin(float m)
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);

	impl->setMargin(m);
}

float DKCollisionShape::Margin(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);

	return impl->getMargin();
}

void DKCollisionShape::SetLocalScale(const DKVector3& s)
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);

	impl->setLocalScaling(BulletVector3(s));
}

DKVector3 DKCollisionShape::LocalScale(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);

	DKVector3 localScale = BulletVector3(impl->getLocalScaling());
	return localScale;
}

DKVector3 DKCollisionShape::CalculateLocalInertia(float mass) const
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserPointer() == this);

	btVector3 localInertia(0,0,0);
	impl->calculateLocalInertia(mass, localInertia);
	return DKVector3(localInertia.x(), localInertia.y(), localInertia.z());
}

bool DKCollisionShape::IsCompund(void) const
{
	return impl->isCompound();
}

bool DKCollisionShape::IsConvex(void) const
{
	return impl->isConvex();
}

bool DKCollisionShape::IsConcave(void) const
{
	return impl->isConcave();
}

bool DKCollisionShape::IsPolyhedral(void) const
{
	return impl->isPolyhedral();
}

DKObject<DKSerializer> DKCollisionShape::SerializeHelper::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKString GetShapeTypeString(ShapeType t) const
		{
			switch (t)
			{
			case ShapeType::Custom:					return L"custom";
			case ShapeType::Empty:					return L"empty";
			case ShapeType::Compound:				return L"compound";
			case ShapeType::Box:					return L"box";
			case ShapeType::Capsule:				return L"capsule";
			case ShapeType::Cylinder:				return L"cylinder";
			case ShapeType::Cone:					return L"cone";
			case ShapeType::Sphere:					return L"sphere";
			case ShapeType::MultiSphere:			return L"multiSphere";
			case ShapeType::ConvexHull:				return L"convexHull";
			case ShapeType::StaticPlane:			return L"staticPlane";
			case ShapeType::StaticTriangleMesh:		return L"staticTriangleMesh";
			}
			return L"";
		}
		bool GetShapeTypeFromString(const DKString& s, ShapeType* p) const
		{
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::Custom)) == 0)				{*p = ShapeType::Custom;				return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::Empty)) == 0)					{*p = ShapeType::Empty;					return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::Compound)) == 0)				{*p = ShapeType::Compound;				return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::Box)) == 0)					{*p = ShapeType::Box;					return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::Capsule)) == 0)				{*p = ShapeType::Capsule;				return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::Cylinder)) == 0)				{*p = ShapeType::Cylinder;				return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::Cone)) == 0)					{*p = ShapeType::Cone;					return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::Sphere)) == 0)				{*p = ShapeType::Sphere;				return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::MultiSphere)) == 0)			{*p = ShapeType::MultiSphere;			return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::ConvexHull)) == 0)			{*p = ShapeType::ConvexHull;			return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::StaticPlane)) == 0)			{*p = ShapeType::StaticPlane;			return true;}
			if (s.CompareNoCase(GetShapeTypeString(ShapeType::StaticTriangleMesh)) == 0)	{*p = ShapeType::StaticTriangleMesh;	return true;}

			return false;
		}
		void GetTransform(DKVariant& v, const DKNSTransform& t) const
		{
			v.SetValueType(DKVariant::TypePairs);
			v.Pairs().Insert(L"orientation", (const DKVariant::VQuaternion&)t.orientation);
			v.Pairs().Insert(L"position", (const DKVariant::VVector3&)t.position);
		}
		bool SetTransform(const DKVariant& v, DKNSTransform* t) const
		{
			if (v.ValueType() == DKVariant::TypePairs)
			{
				auto orientation = v.Pairs().Find(L"orientation");
				auto position = v.Pairs().Find(L"position");
				if (orientation && orientation->value.ValueType() == DKVariant::TypeQuaternion &&
					position && position->value.ValueType() == DKVariant::TypeVector3)
				{
					if (t)
					{
						t->orientation = orientation->value.Quaternion();
						t->position = position->value.Vector3();
					}
					return true;
				}
			}
			return false;
		}
		void GetShapeVariant(DKVariant& var, DKCollisionShape* shape) const
		{
			DKVariant::VPairs& pairs = var.SetValueType(DKVariant::TypePairs).Pairs();

			switch (shape->type)
			{
			case ShapeType::Compound:
				{
					DKCompoundShape* compound = static_cast<DKCompoundShape*>(shape);

					pairs.Insert(L"numChildren", DKVariant::VInteger(compound->NumberOfShapes()));
					DKVariant::VArray& shapeArray = pairs.Value(L"childShapes").SetValueType(DKVariant::TypeArray).Array();
					DKVariant::VArray& transArray = pairs.Value(L"childTransforms").SetValueType(DKVariant::TypeArray).Array();
					size_t numShapes = compound->NumberOfShapes();
					shapeArray.Reserve(numShapes);
					for (size_t i = 0; i < numShapes; ++i)
					{
						size_t n1 = shapeArray.Add(DKVariant::VPairs());
						GetShapeVariant(shapeArray.Value(n1), compound->ShapeAtIndex(i));
						size_t n2 = transArray.Add(DKVariant::VPairs());
						GetTransform(transArray.Value(n2), compound->TransformAtIndex(i));
					}
				}
				break;
			case ShapeType::Box:
				{
					DKBoxShape* box = static_cast<DKBoxShape*>(shape);
					pairs.Insert(L"halfExtents", (const DKVariant::VVector3&)box->HalfExtents());
				}
				break;
			case ShapeType::Capsule:
				{
					DKCapsuleShape* capsule = static_cast<DKCapsuleShape*>(shape);
					pairs.Insert(L"radius", DKVariant::VFloat(capsule->Radius()));
					pairs.Insert(L"height", DKVariant::VFloat(capsule->HalfHeight() * 2));
					pairs.Insert(L"upAxis", DKVariant::VInteger(static_cast<int>(capsule->BaseAxis())));
				}
				break;
			case ShapeType::Cylinder:
				{
					DKCylinderShape* cylinder = static_cast<DKCylinderShape*>(shape);
					pairs.Insert(L"halfExtents", (const DKVariant::VVector3&)cylinder->HalfExtents());
					pairs.Insert(L"upAxis", DKVariant::VInteger(static_cast<int>(cylinder->BaseAxis())));
				}
				break;
			case ShapeType::Cone:
				{
					DKConeShape* cone = static_cast<DKConeShape*>(shape);
					pairs.Insert(L"radius", DKVariant::VFloat(cone->Radius()));
					pairs.Insert(L"height", DKVariant::VFloat(cone->Height()));
					pairs.Insert(L"upAxis", DKVariant::VInteger(static_cast<int>(cone->BaseAxis())));
				}
				break;
			case ShapeType::Sphere:	
				{
					DKSphereShape* sphere = static_cast<DKSphereShape*>(shape);
					pairs.Insert(L"radius", DKVariant::VFloat(sphere->Radius()));
				}
				break;
			case ShapeType::MultiSphere:
				{
					DKMultiSphereShape* ms = static_cast<DKMultiSphereShape*>(shape);
					size_t numSpheres = ms->NumberOfSpheres();
					pairs.Insert(L"numSpheres", DKVariant::VInteger(numSpheres));
					DKVariant::VArray& centers = pairs.Value(L"centers").SetValueType(DKVariant::TypeArray).Array();
					DKVariant::VArray& radii = pairs.Value(L"radii").SetValueType(DKVariant::TypeArray).Array();
					centers.Reserve(numSpheres);
					radii.Reserve(numSpheres);
					for (size_t i = 0; i < numSpheres; ++i)
					{
						centers.Add((const DKVariant::VVector3&)ms->CenterOfSphereAtIndex(i));
						radii.Add((DKVariant::VFloat)ms->RadiusOfSphereAtIndex(i));
					}
				}
				break;
			case ShapeType::ConvexHull:
				{
					DKConvexHullShape* hull = static_cast<DKConvexHullShape*>(shape);
					size_t numPoints = hull->NumberOfPoints();
					pairs.Insert(L"numPoints", DKVariant::VInteger(numPoints));
					DKVariant::VArray& points = pairs.Value(L"points").SetValueType(DKVariant::TypeArray).Array();
					points.Reserve(numPoints);
					for (size_t i = 0; i < numPoints; ++i)
					{
						points.Add((const DKVariant::VVector3&)hull->PointAtIndex(i));
					}
				}
				break;
			case ShapeType::StaticPlane:
				{
					DKStaticPlaneShape* plane = static_cast<DKStaticPlaneShape*>(shape);
					pairs.Insert(L"planeNormal", (const DKVariant::VVector3&)plane->PlaneNormal());
					pairs.Insert(L"planeConstant", (DKVariant::VFloat)plane->PlaneConstant());
				}
				break;
			case ShapeType::StaticTriangleMesh:
				{
					DKStaticTriangleMeshShape* meshShape = static_cast<DKStaticTriangleMeshShape*>(shape);
					size_t numVertices = 0;
					size_t numIndices = 0;
					size_t indexSize = 0;
					const void* vertices = meshShape->VertexBuffer(&numVertices);
					const void* indices = meshShape->IndexBuffer(&numIndices, &indexSize);
					DKAABox aabb = meshShape->MeshAABB();

					DKVariant::VData& vertsData = pairs.Value(L"vertices").Data();
					vertsData.SetContent(vertices, numVertices * sizeof(DKVector3));
					DKVariant::VData& indicesData = pairs.Value(L"indices").Data();
					indicesData.SetContent(indices, indexSize * numIndices);
					pairs.Insert(L"indexSize", (DKVariant::VInteger)indexSize);
					pairs.Insert(L"aabbMin", (const DKVariant::VVector3&)aabb.positionMin);
					pairs.Insert(L"aabbMax", (const DKVariant::VVector3&)aabb.positionMax);
				}
				break;
			}
			pairs.Insert(L"shapeType", (const DKVariant::VString&)GetShapeTypeString(shape->type));
			pairs.Insert(L"margin", DKVariant::VFloat(shape->Margin()));
			pairs.Insert(L"localScale", (const DKVariant::VVector3&)shape->LocalScale());
		}
		bool CreateShapeFromVariant(const DKVariant& v, DKObject<DKCollisionShape>* p) const
		{
			if (v.ValueType() != DKVariant::TypePairs)
				return false;

			const DKVariant::VPairs& pairs = v.Pairs();
			auto stype = pairs.Find(L"shapeType");
			if (stype == NULL || stype->value.ValueType() != DKVariant::TypeString)
				return false;

			ShapeType type;
			if (!GetShapeTypeFromString(stype->value.String(), &type))
				return false;

			switch (type)
			{
			case ShapeType::Compound:
				{
					auto numChildren = pairs.Find(L"numChildren");
					auto childShapes = pairs.Find(L"childShapes");
					auto childTransforms = pairs.Find(L"childTransforms");
					if (numChildren && numChildren->value.ValueType() == DKVariant::TypeInteger &&
						childShapes && childShapes->value.ValueType() == DKVariant::TypeArray &&
						childTransforms && childTransforms->value.ValueType() == DKVariant::TypeArray)
					{
						DKVariant::VInteger num = numChildren->value.Integer();
						if (childShapes->value.Array().Count() >= num && childTransforms->value.Array().Count() >= num)
						{
							DKObject<DKCompoundShape> compound = NULL;
							if (p) compound = DKObject<DKCompoundShape>::New();

							for (DKVariant::VInteger i = 0; i < num; ++i)
							{
								DKObject<DKCollisionShape> child = NULL;
								DKNSTransform trans;

								if (!CreateShapeFromVariant(childShapes->value.Array().Value(i), p ? &child : NULL))
									return false;
								if (!SetTransform(childTransforms->value.Array().Value(i), p ? &trans : NULL))
									return false;

								if (compound && child)
									compound->AddShape(child, trans);
							}
							if (p)
								*p = compound.SafeCast<DKCollisionShape>();
							break;
						}
					}
				}
				return false;
			case ShapeType::Box:
				{
					auto halfExtents = pairs.Find(L"halfExtents");
					if (halfExtents && halfExtents->value.ValueType() == DKVariant::TypeVector3)
					{
						if (p)
							*p = DKOBJECT_NEW DKBoxShape( halfExtents->value.Vector3() );
						break;
					}
				}
				return false;
			case ShapeType::Capsule:
				{
					auto radius = pairs.Find(L"radius");
					auto height = pairs.Find(L"height");
					auto upAxis = pairs.Find(L"upAxis");
					if (radius && radius->value.ValueType() == DKVariant::TypeFloat &&
						height && height->value.ValueType() == DKVariant::TypeFloat &&
						upAxis && upAxis->value.ValueType() == DKVariant::TypeInteger)
					{
						if (p)
							*p = DKOBJECT_NEW DKCapsuleShape(radius->value.Float(), height->value.Float(),
							static_cast<DKCapsuleShape::UpAxis>(upAxis->value.Integer()));
						break;
					}
				}
				return false;
			case ShapeType::Cylinder:
				{
					auto halfExtents = pairs.Find(L"halfExtents");
					auto upAxis = pairs.Find(L"upAxis");
					if (halfExtents && halfExtents->value.ValueType() == DKVariant::TypeVector3 &&
						upAxis && upAxis->value.ValueType() == DKVariant::TypeInteger)
					{
						if (p)
							*p = DKOBJECT_NEW DKCylinderShape( halfExtents->value.Vector3(),
							static_cast<DKCylinderShape::UpAxis>(upAxis->value.Integer()));
						break;
					}
				}
				return false;
			case ShapeType::Cone:
				{
					auto radius = pairs.Find(L"radius");
					auto height = pairs.Find(L"height");
					auto upAxis = pairs.Find(L"upAxis");
					if (radius && radius->value.ValueType() == DKVariant::TypeFloat &&
						height && height->value.ValueType() == DKVariant::TypeFloat &&
						upAxis && upAxis->value.ValueType() == DKVariant::TypeInteger)
					{
						if (p)
							*p = DKOBJECT_NEW DKConeShape(radius->value.Float(), radius->value.Float(), 
							static_cast<DKConeShape::UpAxis>(upAxis->value.Integer()));
						break;
					}
				}
				return false;
			case ShapeType::Sphere:	
				{
					auto radius = pairs.Find(L"radius");
					if (radius && radius->value.ValueType() == DKVariant::TypeFloat)
					{
						if (p)
							*p = DKOBJECT_NEW DKSphereShape(radius->value.Float());
						break;
					}
				}
				return false;
			case ShapeType::MultiSphere:
				{
					auto numSphere = pairs.Find(L"numSpheres");
					auto centers = pairs.Find(L"centers");
					auto radii = pairs.Find(L"radii");
					if (numSphere && numSphere->value.ValueType() == DKVariant::TypeInteger &&
						centers && centers->value.ValueType() == DKVariant::TypeArray &&
						radii && radii->value.ValueType() == DKVariant::TypeArray)
					{
						DKVariant::VInteger num = Max(numSphere->value.Integer(), DKVariant::VInteger(0));
						if (centers->value.Array().Count() >= num && radii->value.Array().Count() >= num)
						{
							DKArray<DKVector3> centersArray;
							DKArray<float> radiiArray;

							centersArray.Reserve(num);
							radiiArray.Reserve(num);

							for (size_t i = 0; i < num; ++i)
							{
								if (centers->value.Array().Value(i).ValueType() != DKVariant::TypeVector3)
									return false;
								if (radii->value.Array().Value(i).ValueType() != DKVariant::TypeFloat)
									return false;

								if (p)
								{
									centersArray.Add( centers->value.Array().Value(i).Vector3() );
									radiiArray.Add( radii->value.Array().Value(i).Float() );
								}
							}

							if (p)
								*p = DKOBJECT_NEW DKMultiSphereShape(centersArray, radiiArray, num);

							break;
						}
					}
				}
				return false;
			case ShapeType::ConvexHull:
				{
					auto numPoints = pairs.Find(L"numPoints");
					auto points = pairs.Find(L"points");
					if (numPoints && numPoints->value.ValueType() == DKVariant::TypeInteger &&
						points && points->value.ValueType() == DKVariant::TypeArray)
					{
						DKVariant::VInteger num = Max(numPoints->value.Integer(), DKVariant::VInteger(0));
						if (points->value.Array().Count() >= num)
						{
							DKArray<DKVector3> pointsArray;
							pointsArray.Reserve(num);

							for (size_t i = 0; i < num; ++i)
							{
								if (points->value.Array().Value(i).ValueType() != DKVariant::TypeVector3)
									return false;

								if (p)
									pointsArray.Add( points->value.Array().Value(i).Vector3() );
							}
							if (p)
								*p = DKOBJECT_NEW DKConvexHullShape(pointsArray, num);

							break;
						}
					}
				}
				return false;
			case ShapeType::StaticPlane:
				{
					auto normal = pairs.Find(L"planeNormal");
					auto constant = pairs.Find(L"planeConstant");
					if (normal && normal->value.ValueType() == DKVariant::TypeVector3 &&
						constant && constant->value.ValueType() == DKVariant::TypeFloat)
					{
						if (p)
							*p = DKOBJECT_NEW DKStaticPlaneShape(normal->value.Vector3(), constant->value.Float());
						break;
					}
				}
				return false;
			case ShapeType::StaticTriangleMesh:
				{
					auto vertices = pairs.Find(L"vertices");
					auto indices = pairs.Find(L"indices");
					auto indexSize = pairs.Find(L"indexSize");
					auto aabbMin = pairs.Find(L"aabbMin");
					auto aabbMax = pairs.Find(L"aabbMax");
					if (vertices && indices &&  indexSize &&
						vertices->value.ValueType() == DKVariant::TypeData &&
						indices->value.ValueType() == DKVariant::TypeData &&
						indexSize->value.ValueType() == DKVariant::TypeInteger)
					{
						DKAABox aabb;
						if (aabbMin && aabbMax &&
							aabbMin->value.ValueType() == DKVariant::TypeVector3 &&
							aabbMax->value.ValueType() == DKVariant::TypeVector3)
						{
							aabb.positionMin = aabbMin->value.Vector3();
							aabb.positionMax = aabbMax->value.Vector3();
						}

						size_t idxSize = indexSize->value.Integer();
						size_t numVerts = vertices->value.Data().Length() / sizeof(DKVector3);
						size_t numIndices = indices->value.Data().Length() / idxSize;
						if (numVerts > 0 && numIndices > 0 && (idxSize == 2 || idxSize == 4))
						{
							if (p)
							{
								if (idxSize == 4)
								*p = DKOBJECT_NEW DKStaticTriangleMeshShape(
									(const DKVector3*)vertices->value.Data().LockShared(), numVerts,
									(const unsigned int*)indices->value.Data().LockShared(), numIndices, aabb, false, 0.0f);
								else
									*p = DKOBJECT_NEW DKStaticTriangleMeshShape(
									(const DKVector3*)vertices->value.Data().LockShared(), numVerts,
									(const unsigned short*)indices->value.Data().LockShared(), numIndices, aabb, false, 0.0f);
								vertices->value.Data().UnlockShared();
								indices->value.Data().UnlockShared();
							}
							break;
						}
					}
				}
				return false;
			}

			if (p && *p)
			{
				auto margin = pairs.Find(L"margin");
				if (margin && margin->value.ValueType() == DKVariant::TypeFloat)
					(*p)->SetMargin(margin->value.Float());
				auto localScale = pairs.Find(L"localScale");
				if (localScale && localScale->value.ValueType() == DKVariant::TypeVector3)
					(*p)->SetLocalScale(localScale->value.Vector3());
			}
			return true;
		}

		DKSerializer* Init(DKCollisionShape::SerializeHelper* p)
		{
			if (p == NULL)
				return NULL;

			this->target = p;
			this->SetResourceClass(L"DKCollisionShape::SerializeHelper");
			this->Bind(L"super", target->DKResource::Serializer(), NULL);

			this->Bind(L"shape", 
				DKFunction([this](DKVariant& v)
				{
					if (target->shape)
					{
						this->GetShapeVariant(v, target->shape);
					}
				}),
				DKFunction([this](DKVariant& v)
				{
					target->shape = NULL;
					bool b = this->CreateShapeFromVariant(v, &(target->shape));
					DKASSERT_DEBUG(b);
				}),
				DKFunction([this](const DKVariant& v)->bool
				{
					return this->CreateShapeFromVariant(v, NULL);
				}),
				DKFunction([this]
				{
					target->shape = NULL;
				})->Invocation());

			return this;
		}
		DKObject<DKCollisionShape::SerializeHelper> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
