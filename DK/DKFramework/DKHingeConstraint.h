//
//  File: DKHingeConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKConstraint.h"

////////////////////////////////////////////////////////////////////////////////
// DKHingeConstraint
// a hinge constraint between two rigid bodies each with local axis as
// orientation of the hinge axis.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKHingeConstraint : public DKConstraint
	{
	public:
		// Using DKNSTransform, z-axis is orientation of hinge axis.
		DKHingeConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB, const DKNSTransform& frameA, const DKNSTransform& frameB);
		DKHingeConstraint(DKRigidBody* bodyA, const DKNSTransform& frameA);

		DKHingeConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB,
						  const DKVector3& pivotInA, const DKVector3& pivotInB,
						  const DKVector3& axisInA, const DKVector3& axisInB);
		DKHingeConstraint(DKRigidBody* bodyA, const DKVector3& pivotInA, const DKVector3& axisInA);
		DKHingeConstraint(void);
		~DKHingeConstraint(void);

		void SetLimit(float lower, float upper, float softness = 0.9f, float biasFactor = 0.3f, float relaxationFactor = 1.0f);
		float LowerLimit(void) const;
		float UpperLimit(void) const;
		float Softness(void) const;
		float BiasFactor(void) const;
		float RelaxationFactor(void) const;

		void SetAngularOnly(bool angularOnly);
		bool IsAngularOnly(void) const;

		void SetAngularMotor(bool enable, float targetVelocity, float maxMotorImpulse);
		void EnableAngularMotor(bool enable);
		bool IsAngularMotorEnabled(void) const;
		float MaxMotorImpulse(void) const;
		float MotorTargetVelocity(void) const;

		float HingeAngle(void) const;
		float HingeAngle(const DKNSTransform& ta, const DKNSTransform& tb) const;

		DKNSTransform FrameA(void) const;
		DKNSTransform FrameB(void) const;
		void SetFrameA(const DKNSTransform& t);
		void SetFrameB(const DKNSTransform& t);
		void SetFrames(const DKNSTransform& ta, const DKNSTransform& tb);

		bool IsValidParam(ParamType type, ParamAxis axis) const override;
		bool HasParam(ParamType type, ParamAxis axis) const override;

		DKFoundation::DKObject<DKSerializer> Serializer(void) override;

	protected:
		void ResetContext(void) override;
		void ResetContextImpl(void) override;

		DKFoundation::DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKHingeConstraint* Copy(UUIDObjectMap&, const DKHingeConstraint*);
	};
}
