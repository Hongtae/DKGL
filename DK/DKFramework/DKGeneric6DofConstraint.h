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
	/// @brief
	/// generic 6 dof constraint. a joint can control 6 degree of freedom limits.
	/// (3 linear axis limits, 3 angular axis limits.)
	/// (see DKConstraint.h for axis 'ParamAxis')
	///
	/// @note
	///    angular limits have following ranges.\n
	///     AngularX: -pi ~ pi					\n
	///     AngularY: -pi/2 ~ pi/2				\n
	///     AngularZ: -pi ~ pi					\n
	///
	class DKGL_API DKGeneric6DofConstraint : public DKConstraint
	{
	public:
		enum AxisState
		{
			StateLocked = 0,		///< axis locked.
			StateFree,				///< axis free.
			StateRanged,			///< axis limited free (free within range)
		};

		/// DKNSTransform is rigid body's local transform.
		DKGeneric6DofConstraint(DKRigidBody* bodyA, DKRigidBody* bodyB, const DKNSTransform& frameA, const DKNSTransform& frameB);
		DKGeneric6DofConstraint(DKRigidBody* bodyB, const DKNSTransform& frameB);
		DKGeneric6DofConstraint();
		~DKGeneric6DofConstraint();

		void SetLimit(ParamAxis axis, float lower, float upper);
		float LowerLimit(ParamAxis axis) const;
		float UpperLimit(ParamAxis axis) const;

		void SetLinearLowerLimit(const DKVector3& v);
		void SetLinearUpperLimit(const DKVector3& v);
		void SetAngularLowerLimit(const DKVector3& v);
		void SetAngularUpperLimit(const DKVector3& v);
		DKVector3 LinearLowerLimit() const;
		DKVector3 LinearUpperLimit() const;
		DKVector3 AngularLowerLimit() const;
		DKVector3 AngularUpperLimit() const;

		DKVector3 Axis(ParamAxis axis) const;
		float Angle(ParamAxis axis) const;
		float RelativePivotPosition(ParamAxis axis) const;
		AxisState State(ParamAxis axis) const;

		DKNSTransform FrameA() const;
		DKNSTransform FrameB() const;
		void SetFrameA(const DKNSTransform& t);
		void SetFrameB(const DKNSTransform& t);
		void SetFrames(const DKNSTransform& ta, const DKNSTransform& tb);

		bool IsValidParam(ParamType type, ParamAxis axis) const override;
		bool HasParam(ParamType type, ParamAxis axis) const override;

		DKObject<DKSerializer> Serializer() override;

	protected:
		DKGeneric6DofConstraint(LinkType type, DKRigidBody* bodyA, DKRigidBody* bodyB, class btGeneric6DofConstraint* ctxt);
		void ResetContext() override;
		void ResetContextImpl() override;

		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
		DKGeneric6DofConstraint* Copy(UUIDObjectMap&, const DKGeneric6DofConstraint*);
	};
}
