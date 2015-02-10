//
//  File: DKDynamicsScene.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKScene.h"
#include "DKRigidBody.h"
#include "DKSoftBody.h"
#include "DKConstraint.h"
#include "DKActionController.h"

////////////////////////////////////////////////////////////////////////////////
// DKDynamicsScene
// a scene with dynamics.
// You can extend physical behavior with DKActionController.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKDynamicsScene : public DKScene
	{
	public:
		DKDynamicsScene(void);
		virtual ~DKDynamicsScene(void);

		void SetGravity(const DKVector3& g);
		DKVector3 Gravity(void) const;

		void Update(double tickDelta, DKFoundation::DKTimeTick tick) override;

		// if FixedFrameRate is greater than 0,
		// it works as fixed-frame rate in physics. (determinism)
		// You need to use fixed frame rate when sharing scene with
		// other simulator. (ex: connected via network or subprocesses)
		void SetFixedFrameRate(double fps);
		double FixedFrameRate(void) const;

	protected:
		bool AddSingleObject(DKModel* obj) override;
		void RemoveSingleObject(DKModel* obj) override;
		void RemoveAllObjects(void) override;

		typedef DKFoundation::DKSet<DKRigidBody*> RigidBodySet;
		typedef DKFoundation::DKSet<DKSoftBody*> SoftBodySet;
		typedef DKFoundation::DKSet<DKConstraint*> ConstraintSet;
		typedef DKFoundation::DKSet<DKActionController*> ActionSet;

		RigidBodySet rigidBodies;
		SoftBodySet softBodies;
		ConstraintSet constraints;
		ActionSet actions;

		virtual void UpdateActions(double tickDelta);

		virtual bool NeedCollision(DKCollisionObject* objA, DKCollisionObject* objB);
		virtual bool NeedResponse(DKCollisionObject* objA, DKCollisionObject* objB);

	private:
		double dynamicsFixedFPS; // fixed time stepping unit.
		static void PreTickCallback(void*, float);
		static void PostTickCallback(void*, float);
		class btActionInterface* actionInterface;

		friend class DKActionController;
		friend class DKConstraint;
	};
}
