//
//  File: DKHingeConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConstraint.h"

namespace DKFramework
{
	/// @brief a hinge constraint between two rigid bodies each with local axis as
	/// orientation of the hinge axis.
	class DKGL_API DKHingeConstraint : public DKConstraint
	{
	public:
		/// Using DKNSTransform, z-axis is orientation of hinge axis.
		DKHingeConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB, const DKNSTransform& frameA, const DKNSTransform& frameB);
		DKHingeConstraint(DKRigidBody* bodyA, const DKNSTransform& frameA);

		DKHingeConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB,
						  const DKVector3& pivotInA, const DKVector3& pivotInB,
						  const DKVector3& axisInA, const DKVector3& axisInB);
		DKHingeConstraint(DKRigidBody* bodyA, const DKVector3& pivotInA, const DKVector3& axisInA);
		DKHingeConstraint();
		~DKHingeConstraint();

		void SetLimit(float lower, float upper, float softness = 0.9f, float biasFactor = 0.3f, float relaxationFactor = 1.0f);
		float LowerLimit() const;
		float UpperLimit() const;
		float Softness() const;
		float BiasFactor() const;
		float RelaxationFactor() const;

		void SetAngularOnly(bool angularOnly);
		bool IsAngularOnly() const;

		void SetAngularMotor(bool enable, float targetVelocity, float maxMotorImpulse);
		void EnableAngularMotor(bool enable);
		bool IsAngularMotorEnabled() const;
		float MaxMotorImpulse() const;
		float MotorTargetVelocity() const;

		float HingeAngle() const;
		float HingeAngle(const DKNSTransform& ta, const DKNSTransform& tb) const;

		DKNSTransform FrameA() const;
		DKNSTransform FrameB() const;
		void SetFrameA(const DKNSTransform& t);
		void SetFrameB(const DKNSTransform& t);
		void SetFrames(const DKNSTransform& ta, const DKNSTransform& tb);

		bool IsValidParam(ParamType type, ParamAxis axis) const override;
		bool HasParam(ParamType type, ParamAxis axis) const override;

		DKObject<DKSerializer> Serializer() override;

	protected:
		void ResetContext() override;
		void ResetContextImpl() override;

		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKHingeConstraint* Copy(UUIDObjectMap&, const DKHingeConstraint*);
	};
}
