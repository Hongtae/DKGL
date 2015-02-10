//
//  File: DKCollisionShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKLine.h"
#include "DKResource.h"
#include "DKTransform.h"
#include "DKAABox.h"
#include "DKSphere.h"

////////////////////////////////////////////////////////////////////////////////
// DKCollisionShape
// collision shape class.
// object has no position. calculates with origin as local object.
//
// Note:
//    Using SerializeHelper nested class for serialize, deserialize.
//    because of this class is not modifiable once it has been created.
//    This applied for all subclasses.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKCollisionShape
	{
	public:
		struct SerializeHelper : public DKResource
		{
			DKFoundation::DKObject<DKSerializer> Serializer(void) override;
			DKFoundation::DKObject<DKCollisionShape> shape;
		};

		enum class UpAxis
		{
			Left = 0,
			Top = 1,
			Forward = 2,
		};
		enum class ShapeType
		{
			Custom = 0,
			Empty,
			Compound,
			Box,
			Capsule,
			Cylinder,
			Cone,
			Sphere,
			MultiSphere,
			ConvexHull,
			StaticPlane,
			StaticTriangleMesh,
		};

		const ShapeType type;

		virtual ~DKCollisionShape(void);

		DKAABox AABB(const DKNSTransform& transform) const;
		DKSphere BoundingSphere(void) const;

		void AABB(const DKNSTransform& transform, DKVector3& aabbMin, DKVector3& aabbMax) const;

		void SetMargin(float m);  // collision margin.
		float Margin(void) const;

		void SetLocalScale(const DKVector3& s);
		DKVector3 LocalScale(void) const;

		DKVector3 CalculateLocalInertia(float mass) const;

		bool IsCompund(void) const;
		bool IsConvex(void) const;
		bool IsConcave(void) const;
		bool IsPolyhedral(void) const;

	protected:
		class btCollisionShape* impl;
		DKCollisionShape(ShapeType t, class btCollisionShape* context);
		
	private:
		DKCollisionShape(const DKCollisionShape&);
		DKCollisionShape& operator = (const DKCollisionShape&);
	};
}
