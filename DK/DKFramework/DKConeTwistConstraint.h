//
//  File: DKConeTwistConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConstraint.h"

namespace DKFramework
{
	/// @brief
	/// cone-twist constraint, useful for simulate ragdoll joints.
	class DKGL_API DKConeTwistConstraint : public DKConstraint
	{
	public:
		DKConeTwistConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB, const DKNSTransform& frameA, const DKNSTransform& frameB);
		DKConeTwistConstraint(DKRigidBody* bodyA, const DKNSTransform& frameA);
		DKConeTwistConstraint();
		~DKConeTwistConstraint();

		/// softness : value in 0.0~1.0 (0.8~1.0 is recommended)
		/// biasFactor : value in 0.0~1.0 (0.3 is recommended)
		/// relaxationFactor : value in 0.0~1.0
		void SetLimit(float swingSpan1, float swingSpan2, float twistSpan, float softness = 1.0f, float biasFactor = 0.3f, float relaxationFactor = 1.0f);

		float SwingSpan1() const;
		void SetSwingSpan1(float);

		float SwingSpan2() const;
		void SetSwingSpan2(float);

		float TwistSpan() const;
		void SetTwistSpan(float);

		float Softness() const;
		void SetSoftness(float);

		float BiasFactor() const;
		void SetBiasFactor(float);

		float RelaxationFactor() const;
		void SetRelaxationFactor(float);

		float TwistAngle() const;

		float Damping() const;
		void SetDamping(float damping);

		void SetFrames(const DKNSTransform& fA, const DKNSTransform& fB);
		DKNSTransform FrameA() const;
		DKNSTransform FrameB() const;

		bool IsValidParam(ParamType type, ParamAxis axis) const override;
		bool HasParam(ParamType type, ParamAxis axis) const override;

		DKObject<DKSerializer> Serializer() override;

	protected:
		void ResetContext() override;
		void ResetContextImpl() override;

		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKConeTwistConstraint* Copy(UUIDObjectMap&, const DKConeTwistConstraint*);
	};
}
