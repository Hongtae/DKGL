//
//  File: DKSliderConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConstraint.h"

namespace DKFramework
{
	/// @brief slider constraint allows the body to rotate around one axis
	/// and translate along this axis.
	class DKGL_API DKSliderConstraint : public DKConstraint
	{
	public:
		DKSliderConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB, const DKNSTransform& frameInA, const DKNSTransform& frameInB);
		DKSliderConstraint(DKRigidBody* bodyB, const DKNSTransform& frameInB);
		DKSliderConstraint();
		~DKSliderConstraint();

		void SetLinearLimit(float lower, float upper);
		void SetAngularLimit(float lower, float upper);
		void SetLinearLowerLimit(float limit);
		void SetLinearUpperLimit(float limit);
		void SetAngularLowerLimit(float limit);
		void SetAngularUpperLimit(float limit);
		float LinearLowerLimit() const;
		float LinearUpperLimit() const;
		float AngularLowerLimit() const;
		float AngularUpperLimit() const;

		DKNSTransform FrameA() const;
		DKNSTransform FrameB() const;
		void SetFrameA(const DKNSTransform&);
		void SetFrameB(const DKNSTransform&);
		void SetFrames(const DKNSTransform&, const DKNSTransform&);

		bool IsValidParam(ParamType type, ParamAxis axis) const override;
		bool HasParam(ParamType type, ParamAxis axis) const override;

		DKObject<DKSerializer> Serializer() override;

	protected:
		void ResetContext() override;
		void ResetContextImpl() override;

		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKSliderConstraint* Copy(UUIDObjectMap&, const DKSliderConstraint*);
	};
}
