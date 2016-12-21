﻿//
//  File: DKFixedConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKConstraint.h"

namespace DKFramework
{
	/// @brief a constraint fixes reference bodis.
	class DKGL_API DKFixedConstraint : public DKConstraint
	{
	public:
		DKFixedConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB, const DKNSTransform& frameA, const DKNSTransform& frameB);
		DKFixedConstraint(DKRigidBody* bodyA, const DKNSTransform& frameA);
		DKFixedConstraint(void);
		~DKFixedConstraint(void);

		DKNSTransform FrameA(void) const;
		DKNSTransform FrameB(void) const;
		void SetFrameA(const DKNSTransform& t);
		void SetFrameB(const DKNSTransform& t);
		void SetFrames(const DKNSTransform& ta, const DKNSTransform& tb);

		DKObject<DKSerializer> Serializer(void) override;

	protected:
		void ResetContext(void) override;
		void ResetContextImpl(void) override;

		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKFixedConstraint* Copy(UUIDObjectMap&, const DKFixedConstraint*);
	};
}
