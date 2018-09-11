//
//  File: DKGeneric6DofConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConstraint.h"

namespace DKFramework
{
	/// @brief gear constraint, joint will couple the angular velocity for two rigid bodies
	/// around given local axis and ratio.
	class DKGL_API DKGearConstraint : public DKConstraint
	{
	public:
		DKGearConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB,
						 const DKVector3& axisInA, const DKVector3& axisInB,
						 float ratio = 1.0f);
		DKGearConstraint();
		~DKGearConstraint();

		void SetAxisInA(const DKVector3& axis);
		void SetAxisInB(const DKVector3& axis);
		DKVector3 AxisInA() const;
		DKVector3 AxisInB() const;

		float Ratio() const;
		void SetRatio(float r);
		
		DKObject<DKSerializer> Serializer() override;

	protected:
		void ResetContext() override;
		void ResetContextImpl() override;

		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKGearConstraint* Copy(UUIDObjectMap&, const DKGearConstraint*);
	};
}
