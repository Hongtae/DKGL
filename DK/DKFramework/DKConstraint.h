//
//  File: DKConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKModel.h"
#include "DKRigidBody.h"

////////////////////////////////////////////////////////////////////////////////
// DKConstraint
// join constraint for dynamics of rigid bodies.
// 
// ERP : error reduction parameter
// CFM : constraint force mixing
//
// if CFM = 0 (default), constraint become hard.
// if CFM > 0, constraint become soft. (bigger value makes softer)
// if CFM < 0, become unstable. don't use.
//
// if ERP = 0, no correction for join error occurred.
//             object slipping could happen.
// if ERP = 1, all errors will be corrected while simulation process.
//             but not all objects could be corrected actually,
//             use ERP=1 is not recommended.
//             (0.1 ~ 0.8 is recommended, 0.2 is default)
//
// Note:
//    deserialize using DKSerializer, bodyA, bodyB is restored from UUIDs.
//    If reference bodies are not in same Node-tree (DKModel tree), reference
//    bodies will not be restored. (non seekable with UUIDs if not in group)
//    If constraint failed to recover reference bodies, recover-info still
//    remains in object until Retarget() has called. this makes object able to
//    restore references later (when UUIDs is available.),
//    reference bodies recovered by OnUpdateTreeReferences().
//
//    cloning object by Clone(), reference bodies (bodyA, bodyB) will not be
//    cloned directly. object will try to recover references by UUIDs after
//    clone completed if bodyA, bodyB has been cloned.
//    reference bodies recovered by UpdateCopiedReferenceUUIDs().
//
//    reference bodies (bodyA, bodyB) must not be parent of constraint object.
//    constraint object has reference as DKObject, which has ownership.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKConstraint : public DKModel
	{
	public:
		enum class ParamType
		{
			ERP = 0,
			STOP_ERP,
			CFM,
			STOP_CFM,
		};
		enum class ParamAxis
		{
			Default = 0,
			LinearX,
			LinearY,
			LinearZ,
			AngularX,
			AngularY,
			AngularZ,
		};
		enum class LinkType
		{
			Custom = 0,
			Fixed,
			Point2Point,
			Hinge,
			ConeTwist,
			Generic6Dof,
			Generic6DofSpring,
			Slider,
			Gear,
		};
		const LinkType type;

		virtual ~DKConstraint(void);

		bool disableCollisionsBetweenLinkedBodies;

		virtual bool IsValidParam(ParamType type, ParamAxis axis) const		{return false;}
		virtual bool HasParam(ParamType type, ParamAxis axis) const			{return false;}
		virtual void SetParam(ParamType type, ParamAxis axis, float value);
		virtual float GetParam(ParamType type, ParamAxis axis);

		void SetBreakingImpulseThreshold(float threshold);
		float BreakingImpulseThreshold(void) const;

		bool IsEnabled(void) const;
		void SetEnabled(bool e);

		void Enable(void)				{SetEnabled(true);}
		void Disable(void)				{SetEnabled(false);}

		DKRigidBody* BodyA(void)				{ return bodyA; }
		DKRigidBody* BodyB(void)				{ return bodyB; }
		const DKRigidBody* BodyA(void) const	{ return bodyA; }
		const DKRigidBody* BodyB(void) const	{ return bodyB; }

		virtual bool Retarget(DKRigidBody* a, DKRigidBody* b);

		virtual DKFoundation::DKObject<DKSerializer> Serializer(void) override;

	protected:
		DKConstraint(LinkType type, DKRigidBody* rbA, DKRigidBody* rbB, class btTypedConstraint* p);

		void ResetObject(void);
		// ResetContext, ResetContextImpl should be overriden by subclass.
		// invoked by ResetObject.
		virtual void ResetContext(void);
		virtual void ResetContextImpl(void) = 0;

		DKFoundation::DKObject<DKRigidBody> bodyA;
		DKFoundation::DKObject<DKRigidBody> bodyB;

		// called on object has restored.
		void OnUpdateTreeReferences(NamedObjectMap&, UUIDObjectMap&) override;
	
		// called on object has cloned.
		void UpdateCopiedReferenceUUIDs(UUIDObjectMap&) override;
		DKConstraint* Copy(UUIDObjectMap&, const DKConstraint*);

		// verify argument object (DKModel) can be parent of this.
		// if argument object (DKModel) is refered by this,
		// it can not be ancestor.
		bool CanAcceptObjectAsParent(DKModel*) const override;

		class btTypedConstraint* impl;

	private:
		DKConstraint(const DKConstraint&);
		DKConstraint& operator = (const DKConstraint&);

		struct TargetRestoreInfo
		{
			DKFoundation::DKUUID bodyA;
			DKFoundation::DKUUID bodyB;
		};
		DKFoundation::DKObject<TargetRestoreInfo> restoreInfo; // restore-info
		void RestoreTargets(UUIDObjectMap&);
	};
}
