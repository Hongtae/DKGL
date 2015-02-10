//
//  File: DKRigidBody.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKCollisionObject.h"
#include "DKCollisionShape.h"

////////////////////////////////////////////////////////////////////////////////
// DKRigidBody
// a rigid body. a node object which can be used by DKScene.
// object should have CollisionShape to interact.
// this class not designed for subclassing.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKRigidBody : public DKCollisionObject
	{
		friend class DKConstraint;
	public:
		struct DKLIB_API ObjectData
		{
			ObjectData(float mass = 0.0f, const DKVector3& localInertia = DKVector3::zero);
			float mass;
			DKVector3 localInertia;
			float linearDamping;
			float angularDamping;
			float friction;
			float rollingFriction;
			float restitution;
			float linearSleepingThreshold;
			float angularSleepingThreshold;
			bool additionalDamping;
			float additionalDampingFactor;
			float additionalLinearDampingThresholdSqr;
			float additionalAngularDampingThresholdSqr;
			float additionalAngularDampingFactor;
		};

		DKRigidBody(const DKFoundation::DKString& name = L"");
		DKRigidBody(DKCollisionShape* shape, float mass = 0.0f);
		DKRigidBody(DKCollisionShape* shape, float mass, const DKVector3& localInertia);
		DKRigidBody(DKCollisionShape* shape, const ObjectData& data);
		~DKRigidBody(void);

		void SetWorldTransform(const DKNSTransform& t) override;
		void SetLocalTransform(const DKNSTransform& t) override;
		DKNSTransform CenterOfMassTransform(void) const;
		DKVector3 CenterOfMassPosition(void) const;

		// mass (0 for static object)
		void SetMass(float mass);
		float Mass(void) const;
		float InverseMass(void) const;

		// local inertia, can be calculated by CollisionShape
		void SetLocalInertia(const DKVector3& inertia);
		DKVector3 LocalInertia(void) const;
		DKVector3 InverseDiagLocalInertia(void) const;
		DKMatrix3 InverseWorldInertiaTensor(void) const;

		// linear velocity
		void SetLinearVelocity(const DKVector3&);
		DKVector3 LinearVelocity(void) const;

		// angular velocity
		void SetAngularVelocity(const DKVector3&);
		DKVector3 AngularVelocity(void) const;

		void SetLinearFactor(const DKVector3&);
		DKVector3 LinearFactor(void) const;

		void SetAngularFactor(const DKVector3&);
		DKVector3 AngularFactor(void) const;

		void SetLinearDamping(float);
		float LinearDamping(void) const;

		void SetAngularDamping(float);
		float AngularDamping(void) const;

		DKVector3 TotalForce(void) const;
		DKVector3 TotalTorque(void) const;

		DKVector3 VelocityInLocalPoint(const DKVector3& pos) const;

		float ComputeImpulseDenominator(const DKVector3& pos, const DKVector3& normal) const;
		float ComputeAngularImpulseDenominator(const DKVector3& axis) const;
		DKVector3 ComputeGyroscopicForce(float maxGyroscopicForce) const;

		void ApplyForce(const DKVector3& force, const DKVector3& relpos);
		void ApplyImpulse(const DKVector3& impulse, const DKVector3& relpos);

		void ApplyTorque(const DKVector3& torque);
		void ApplyCentralForce(const DKVector3& force);
		void ApplyCentralImpulse(const DKVector3& impulse);
		void ApplyTorqueImpulse(const DKVector3& torque);

		void ClearForces();

		void SetLinearSleepingThreshold(float);
		void SetAngularSleepingThreshold(float);
		float LinearSleepingThreshold(void) const;
		float AngularSleepingThreshold(void) const;

		DKFoundation::DKObject<DKSerializer> Serializer(void) override;  

	protected:
		DKFoundation::DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKRigidBody* Copy(UUIDObjectMap&, const DKRigidBody*);

		bool ResetObject(DKCollisionShape* shape, const ObjectData& data);
		bool GetObjectData(ObjectData& data) const;

		void OnAddedToParent(void) override;
		void OnSetAnimation(DKAnimatedTransform*) override;
		void OnUpdateSceneState(const DKNSTransform& parentWorldTransform) override;

	private:
		class btMotionState* motionState;
	};
}
