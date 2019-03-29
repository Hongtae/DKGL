//
//  File: DKCollisionObject.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKModel.h"
#include "DKCollisionShape.h"

namespace DKFramework
{
	/// @brief
	/// collision object for scene (DKWorld, DKDynamicsWorld)
	/// @details
	/// this is basic collision object not for dynamics.
	/// for dynamics use DKRigidBody instead.
	///
	/// @note
	///  This is wrapper for bullet-physics
	class DKGL_API DKCollisionObject : public DKModel
	{
	public:
		enum ObjectType
		{
			CollisionObject = 0,
			RigidBody,
			SoftBody,
			GhostObject,
		};
		const ObjectType objectType;

		DKCollisionObject();
		virtual ~DKCollisionObject();

		void SetWorldTransform(const DKNSTransform& t) override;
		void SetLocalTransform(const DKNSTransform& t) override;

		void Activate(bool force = false);
		void KeepActivating(bool active);
		void SetKinematic(bool k);
		void SetContactResponse(bool response);

		bool IsActive() const;
		bool IsStatic() const;
		bool IsKinematic() const;
		bool HasContactResponse() const;

		void SetRestitution(float);
		float Restitution() const;
		void SetFriction(float);
		float Friction() const;
		void SetRollingFriction(float);
		float RollingFriction() const;
		void SetHitFraction(float);
		float HitFraction() const;

		virtual void SetCollisionShape(DKCollisionShape* s);
		DKCollisionShape* CollisionShape();
		const DKCollisionShape* CollisionShape() const;

		DKObject<DKSerializer> Serializer() override;

	protected:
		DKCollisionObject* Copy(UUIDObjectMap&, const DKCollisionObject*);
		DKObject<DKCollisionShape> collisionShape;

		DKCollisionObject(ObjectType t, class btCollisionObject* co);
		class btCollisionObject* impl;
	};
}
