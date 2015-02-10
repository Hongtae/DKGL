//
//  File: DKCollisionObject.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKModel.h"
#include "DKCollisionShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKCollisionObject
// collision object for scene (DKScene, DKDynamicsScene)
// this is basic collision object not for dynamics.
// for dynamics use DKRigidBody instead.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKCollisionObject : public DKModel
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

		DKFoundation::DKObject<DKSerializer> Serializer(void) override;

	protected:
		DKCollisionObject* Copy(UUIDObjectMap&, const DKCollisionObject*);
		DKFoundation::DKObject<DKCollisionShape> collisionShape;

		DKCollisionObject(ObjectType t, class btCollisionObject* co);
		class btCollisionObject* impl;
	};
}
