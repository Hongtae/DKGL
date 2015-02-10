//
//  File: DKConeTwistConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKConstraint.h"

////////////////////////////////////////////////////////////////////////////////
// DKConeTwistConstraint
// cone-twist constraint, useful for simulate ragdoll joints.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKConeTwistConstraint : public DKConstraint
	{
	public:
		DKConeTwistConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB, const DKNSTransform& frameA, const DKNSTransform& frameB);
		DKConeTwistConstraint(DKRigidBody* bodyA, const DKNSTransform& frameA);
		DKConeTwistConstraint(void);
		~DKConeTwistConstraint(void);

		// softness : value in 0.0~1.0 (0.8~1.0 is recommended)
		// biasFactor : value in 0.0~1.0 (0.3 is recommended)
		// relaxationFactor : value in 0.0~1.0
		void SetLimit(float swingSpan1, float swingSpan2, float twistSpan, float softness = 1.0f, float biasFactor = 0.3f, float relaxationFactor = 1.0f);

		float SwingSpan1(void) const;
		void SetSwingSpan1(float);

		float SwingSpan2(void) const;
		void SetSwingSpan2(float);

		float TwistSpan(void) const;
		void SetTwistSpan(float);

		float Softness(void) const;
		void SetSoftness(float);

		float BiasFactor(void) const;
		void SetBiasFactor(float);

		float RelaxationFactor(void) const;
		void SetRelaxationFactor(float);

		float TwistAngle(void) const;

		float Damping(void) const;
		void SetDamping(float damping);

		void SetFrames(const DKNSTransform& fA, const DKNSTransform& fB);
		DKNSTransform FrameA(void) const;
		DKNSTransform FrameB(void) const;

		bool IsValidParam(ParamType type, ParamAxis axis) const override;
		bool HasParam(ParamType type, ParamAxis axis) const override;

		DKFoundation::DKObject<DKSerializer> Serializer(void) override;

	protected:
		void ResetContext(void) override;
		void ResetContextImpl(void) override;

		DKFoundation::DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKConeTwistConstraint* Copy(UUIDObjectMap&, const DKConeTwistConstraint*);
	};
}
