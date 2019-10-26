//
//  File: DKDynamicsScene.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKScene.h"
#include "DKRigidBody.h"
#include "DKSoftBody.h"
#include "DKConstraint.h"
#include "DKActionController.h"

namespace DKFramework
{
	/// @brief a scene with dynamics.
	/// @details
	/// You can extend physical behavior with DKActionController.
	/// @note
	/// dynamics simulation is performed in Bullet-Physics.
	class DKGL_API DKDynamicsScene : public DKScene
	{
	public:
		DKDynamicsScene();
		virtual ~DKDynamicsScene();

		void SetGravity(const DKVector3& g);
		DKVector3 Gravity() const;

		void Update(double tickDelta, DKTimeTick tick) override;

		/// if FixedFrameRate is greater than 0,
		/// it works as fixed-frame rate in physics. (determinism)
		/// You need to use fixed frame rate when sharing scene with
		/// other simulator. (ex: connected via network or subprocesses)
		void SetFixedFrameRate(double fps);
		double FixedFrameRate() const;

		void RemoveAllObjects() override;

	protected:
		bool AddSingleObject(DKModel* obj) override;
		void RemoveSingleObject(DKModel* obj) override;

		typedef DKSet<DKRigidBody*> RigidBodySet;
		typedef DKSet<DKSoftBody*> SoftBodySet;
		typedef DKSet<DKConstraint*> ConstraintSet;
		typedef DKSet<DKActionController*> ActionSet;

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
