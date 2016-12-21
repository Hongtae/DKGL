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

		DKCollisionObject(void);
		virtual ~DKCollisionObject(void);

		void SetWorldTransform(const DKNSTransform& t) override;
		void SetLocalTransform(const DKNSTransform& t) override;

		void Activate(bool force = false);
		void KeepActivating(bool active);
		void SetKinematic(bool k);
		void SetContactResponse(bool response);

		bool IsActive(void) const;
		bool IsStatic(void) const;
		bool IsKinematic(void) const;
		bool HasContactResponse(void) const;

		void SetRestitution(float);
		float Restitution(void) const;
		void SetFriction(float);
		float Friction(void) const;
		void SetRollingFriction(float);
		float RollingFriction(void) const;
		void SetHitFraction(float);
		float HitFraction(void) const;

		virtual void SetCollisionShape(DKCollisionShape* s);
		DKCollisionShape* CollisionShape(void);
		const DKCollisionShape* CollisionShape(void) const;

		DKObject<DKSerializer> Serializer(void) override;

	protected:
		DKCollisionObject* Copy(UUIDObjectMap&, const DKCollisionObject*);
		DKObject<DKCollisionShape> collisionShape;

		DKCollisionObject(ObjectType t, class btCollisionObject* co);
		class btCollisionObject* impl;
	};
}
