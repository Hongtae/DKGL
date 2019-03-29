//
//  File: DKCollisionShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKLine.h"
#include "DKResource.h"
#include "DKTransform.h"
#include "DKAabb.h"
#include "DKSphere.h"

namespace DKFramework
{
	/// @brief
	/// collision shape class.
	/// @details
	/// object has no position. calculates with origin as local object.
	///
	/// @note
	///    Using SerializeHelper nested class for serialize, deserialize.
	///    because of this class is not modifiable once it has been created.
	///    This applied for all subclasses.
	///
	/// @note
	///    This is wrapper of bullet-physics
	class DKGL_API DKCollisionShape
	{
	public:
		struct SerializeHelper : public DKResource
		{
			DKObject<DKSerializer> Serializer() override;
			DKObject<DKCollisionShape> shape;
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

		virtual ~DKCollisionShape();

		DKAabb Aabb(const DKNSTransform& transform) const;
		void Aabb(const DKNSTransform& transform, DKVector3& aabbMin, DKVector3& aabbMax) const;
		DKSphere BoundingSphere() const;

		void SetMargin(float m);  ///< collision margin.
		float Margin() const;

		void SetLocalScale(const DKVector3& s);
		DKVector3 LocalScale() const;

		DKVector3 CalculateLocalInertia(float mass) const;

		bool IsCompund() const;
		bool IsConvex() const;
		bool IsConcave() const;
		bool IsPolyhedral() const;

	protected:
		class btCollisionShape* impl;
		DKCollisionShape(ShapeType t, class btCollisionShape* context);
		
	private:
		DKCollisionShape(const DKCollisionShape&) = delete;
		DKCollisionShape& operator = (const DKCollisionShape&) = delete;
	};
}
