//
//  File: DKDynamicsWorld.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletPhysics.h"
#include "DKMath.h"
#include "DKDynamicsWorld.h"

namespace DKFramework
{
	namespace Private
	{
		struct CollisionDispatcher : public btCollisionDispatcher
		{
			typedef DKFunctionSignature<bool (DKCollisionObject*, DKCollisionObject*)> CollisionHandler;
			DKObject<CollisionHandler> collisionFunc;
			DKObject<CollisionHandler> responseFunc;

			CollisionDispatcher(btCollisionConfiguration* config) : btCollisionDispatcher(config) {}

			bool needsCollision(const btCollisionObject* body0,const btCollisionObject* body1)
			{
				if (btCollisionDispatcher::needsCollision(body0, body1))
				{
					if (collisionFunc)
					{
						DKCollisionObject* obj0 = (DKCollisionObject*)body0->getUserPointer();
						DKCollisionObject* obj1 = (DKCollisionObject*)body1->getUserPointer();

						return collisionFunc->Invoke(obj0, obj1);
					}					
					return true;
				}
				return false;
			}	
			bool needsResponse(const btCollisionObject* body0,const btCollisionObject* body1)
			{
				if (btCollisionDispatcher::needsResponse(body0, body1))
				{
					if (responseFunc)
					{
						DKCollisionObject* obj0 = (DKCollisionObject*)body0->getUserPointer();
						DKCollisionObject* obj1 = (DKCollisionObject*)body1->getUserPointer();

						return responseFunc->Invoke(obj0, obj1);
					}
					return true;
				}
				return false;
			}
			//void dispatchAllCollisionPairs(btOverlappingPairCache* pairCache,const btDispatcherInfo& dispatchInfo,btDispatcher* dispatcher)
			//{
			//	btCollisionDispatcher::dispatchAllCollisionPairs(pairCache, dispatchInfo, dispatcher);
			//}
		};

		CollisionWorldContext* CreateDynamicsWorldContext()
		{
			CollisionWorldContext* ctxt = new CollisionWorldContext();
			ctxt->configuration = new btDefaultCollisionConfiguration();
			ctxt->dispatcher = new CollisionDispatcher(ctxt->configuration);
			ctxt->broadphase = new btDbvtBroadphase();
			ctxt->solver = new btSequentialImpulseConstraintSolver();
			ctxt->world = new btDiscreteDynamicsWorld(ctxt->dispatcher, ctxt->broadphase, ctxt->solver, ctxt->configuration);
			ctxt->tick = 0;
			return ctxt;
		}

		struct ActionInterface : public btActionInterface
		{
			typedef DKFunctionSignature<void (double)>	Updater;
			typedef DKFunctionSignature<void>			Drawer;

			DKObject<Updater> updater;

			void updateAction( btCollisionWorld*, btScalar delta)	{updater->Invoke(delta);}
			void debugDraw(btIDebugDraw* debugDrawer)				{}
		};
	}
}

using namespace DKFramework;
using namespace DKFramework::Private;

DKDynamicsWorld::DKDynamicsWorld()
	: DKWorld(CreateDynamicsWorldContext())
	, dynamicsFixedFPS(0.0)
	, actionInterface(NULL)
{
	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context->broadphase);
	DKASSERT_DEBUG(context->dispatcher);
	DKASSERT_DEBUG(context->configuration);
	DKASSERT_DEBUG(context->solver);
	DKASSERT_DEBUG(context->world);
	DKASSERT_DEBUG(dynamic_cast<btDiscreteDynamicsWorld*>(context->world));

	CollisionDispatcher* dispatcher = dynamic_cast<CollisionDispatcher*>(context->dispatcher);
	DKASSERT_DEBUG(dispatcher);

	dispatcher->collisionFunc = DKFunction(this, &DKDynamicsWorld::NeedCollision);
	dispatcher->responseFunc = DKFunction(this, &DKDynamicsWorld::NeedResponse);

	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(context->world);
	world->setInternalTickCallback((btInternalTickCallback)DKDynamicsWorld::PreTickCallback, this, true);
	world->setInternalTickCallback((btInternalTickCallback)DKDynamicsWorld::PostTickCallback, this, false);

	ActionInterface* act = new ActionInterface();
	act->updater = DKFunction(this, &DKDynamicsWorld::UpdateActions);
	this->actionInterface = act;
	world->addAction(this->actionInterface);	
}

DKDynamicsWorld::~DKDynamicsWorld()
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(dynamic_cast<btDiscreteDynamicsWorld*>(context->world));

	this->RemoveAllObjects();

	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(context->world);

	world->removeAction(this->actionInterface);

	this->actions.Clear();

	DKASSERT_DEBUG(world->getNumConstraints() == 0);

	delete this->actionInterface;
}

void DKDynamicsWorld::PreTickCallback(void* world, float delta)
{
	DKDynamicsWorld* scene = static_cast<DKDynamicsWorld*>(static_cast<btDynamicsWorld*>(world)->getWorldUserInfo());

	scene->context->internalTick++;
	scene->UpdateObjectKinematics(delta, scene->context->internalTick);
}

void DKDynamicsWorld::PostTickCallback(void* world, float delta)
{
	DKDynamicsWorld* scene = static_cast<DKDynamicsWorld*>(static_cast<btDynamicsWorld*>(world)->getWorldUserInfo());
}

void DKDynamicsWorld::Update(double tickDelta, DKTimeTick tick)
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(dynamic_cast<btDiscreteDynamicsWorld*>(context->world));

	if (tick && tick == context->tick)
		return;

	DKCriticalSection<DKSpinLock> guard(context->lock);

	context->tick = tick;

	PrepareUpdateNode();

	if (dynamicsFixedFPS > 0.001)	// fixed frame rate for calculate physics (frame per second)
	{
		const double fixedTimeStep = 1.0 / dynamicsFixedFPS;
		int maxSubStep = ceil(tickDelta * dynamicsFixedFPS) + 1;
		DKASSERT_DEBUG( maxSubStep > 0 );
		DKASSERT_DEBUG( tickDelta < maxSubStep * fixedTimeStep );
		static_cast<btDiscreteDynamicsWorld*>(context->world)->stepSimulation(tickDelta, maxSubStep, fixedTimeStep);
	}
	else
	{
		static_cast<btDiscreteDynamicsWorld*>(context->world)->stepSimulation(tickDelta);
	}

	UpdateObjectSceneStates();
	CleanupUpdateNode();
}

void DKDynamicsWorld::SetFixedFrameRate(double fps)
{
	dynamicsFixedFPS = fps;
}

double DKDynamicsWorld::FixedFrameRate() const
{
	return dynamicsFixedFPS;
}

void DKDynamicsWorld::UpdateActions(double tickDelta)
{
	this->actions.EnumerateForward([=](const DKActionController* p)
	{
		const_cast<DKActionController*>(p)->Update(tickDelta, context->internalTick);
	});
}

void DKDynamicsWorld::SetGravity(const DKVector3& g)
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(dynamic_cast<btDiscreteDynamicsWorld*>(context->world));

	static_cast<btDiscreteDynamicsWorld*>(context->world)->setGravity(btVector3(g.x, g.y, g.z));
}

DKVector3 DKDynamicsWorld::Gravity() const
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(dynamic_cast<btDiscreteDynamicsWorld*>(context->world));

	btVector3 g = static_cast<btDiscreteDynamicsWorld*>(context->world)->getGravity();
	return DKVector3(g.x(), g.y(), g.z());
}

bool DKDynamicsWorld::AddSingleObject(DKModel* obj)
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(dynamic_cast<btDiscreteDynamicsWorld*>(context->world));
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(context->world);

	DKASSERT_DEBUG(obj);
	DKASSERT_DEBUG(obj->Scene() == this);

	switch (obj->type)
	{
	case DKModel::TypeCollision:
		DKASSERT_DEBUG(dynamic_cast<DKCollisionObject*>(obj) != NULL);
		if (true) {
			DKCollisionObject* col = static_cast<DKCollisionObject*>(obj);
			if (col->objectType == DKCollisionObject::RigidBody)
			{
				DKASSERT_DEBUG(dynamic_cast<DKRigidBody*>(col));
				DKRigidBody* rigidBody = static_cast<DKRigidBody*>(col);
				DKASSERT_DEBUG(this->rigidBodies.Contains(rigidBody) == false);

				btRigidBody* rb = btRigidBody::upcast(BulletCollisionObject(col));
				DKASSERT_DEBUG(rb);
				DKASSERT_DEBUG(rb->getCollisionShape());

				DKCriticalSection<DKSpinLock> guard(context->lock);
				world->addRigidBody(rb);
				this->rigidBodies.Insert(rigidBody);
				return true;
			}
			else if (col->objectType == DKCollisionObject::SoftBody)
			{
				DKERROR_THROW("NOT IMPLEMENTED");
			}
		}
		break;
	case DKModel::TypeConstraint:
		DKASSERT_DEBUG(dynamic_cast<DKConstraint*>(obj) != NULL);
		if (true)
		{
			DKConstraint* con = static_cast<DKConstraint*>(obj);
			DKASSERT_DEBUG(this->constraints.Contains(con) == false);

			btTypedConstraint* c = BulletTypedConstraint(con);
			DKCriticalSection<DKSpinLock> guard(context->lock);
			world->addConstraint(c, con->disableCollisionsBetweenLinkedBodies);
			this->constraints.Insert(con);
			return true;
		}
		break;
	case DKModel::TypeAction:
		DKASSERT_DEBUG(dynamic_cast<DKActionController*>(obj) != NULL);
		if (true) {
			DKActionController* act = static_cast<DKActionController*>(obj);
			DKASSERT_DEBUG(this->actions.Contains(act) == false);
			this->actions.Insert(act);
			return true;
		}
		break;
	default:
		return DKWorld::AddSingleObject(obj);
	}
	return false;
}

void DKDynamicsWorld::RemoveSingleObject(DKModel* obj)
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(dynamic_cast<btDiscreteDynamicsWorld*>(context->world));
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(context->world);

	DKASSERT_DEBUG(obj);
	DKASSERT_DEBUG(obj->Scene() == this);

	switch (obj->type)
	{
	case DKModel::TypeCollision:
		DKASSERT_DEBUG(dynamic_cast<DKCollisionObject*>(obj) != NULL);
		if (true) {
			DKCollisionObject* col = static_cast<DKCollisionObject*>(obj);
			if (col->objectType == DKCollisionObject::RigidBody)
			{
				DKASSERT_DEBUG(dynamic_cast<DKRigidBody*>(col));
				DKRigidBody* rigidBody = static_cast<DKRigidBody*>(col);
				DKASSERT_DEBUG(this->rigidBodies.Contains(rigidBody));

				btRigidBody* rb = btRigidBody::upcast(BulletCollisionObject(col));
				DKASSERT_DEBUG(rb);
				DKASSERT_DEBUG(rb->getCollisionShape());

				DKCriticalSection<DKSpinLock> guard(context->lock);
				world->removeRigidBody(rb);
				this->rigidBodies.Remove(rigidBody);
			}
			else if (col->objectType == DKCollisionObject::SoftBody)
			{
				DKERROR_THROW("NOT IMPLEMENTED");
			}
		}
		break;
	case DKModel::TypeConstraint:
		DKASSERT_DEBUG(dynamic_cast<DKConstraint*>(obj) != NULL);
		if (true) {
			DKConstraint* con = static_cast<DKConstraint*>(obj);
			DKASSERT_DEBUG(this->constraints.Contains(con));

			btTypedConstraint* c = BulletTypedConstraint(con);
			DKCriticalSection<DKSpinLock> guard(context->lock);
			world->removeConstraint(c);
			this->constraints.Remove(con);
		}
		break;
	case DKModel::TypeAction:
		DKASSERT_DEBUG(dynamic_cast<DKActionController*>(obj) != NULL);
		if (true) {
			DKActionController* act = static_cast<DKActionController*>(obj);
			DKASSERT_DEBUG(this->actions.Contains(act));
			this->actions.Remove(act);
		}
		break;
	default:
		DKWorld::RemoveSingleObject(obj);
	}
}

void DKDynamicsWorld::RemoveAllObjects()
{
	DKASSERT_DEBUG(context && context->world);
	DKASSERT_DEBUG(dynamic_cast<btDiscreteDynamicsWorld*>(context->world));
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(context->world);

	context->lock.Lock();
	for (int i = world->getNumConstraints() - 1; i >= 0; --i)
	{
		btTypedConstraint* constraint = world->getConstraint(i);
		world->removeConstraint(constraint);
	}
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; --i)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}
	context->lock.Unlock();
	this->rigidBodies.Clear();
	this->softBodies.Clear();
	this->constraints.Clear();
	this->actions.Clear();
	DKWorld::RemoveAllObjects();
}

bool DKDynamicsWorld::NeedCollision(DKCollisionObject* objA, DKCollisionObject* objB)
{
	//DKLog("DKDynamicsWorld::NeedCollision: %ls, %ls\n", (const wchar_t*)objA->Name(), (const wchar_t*)objB->Name());
	return true;
}

bool DKDynamicsWorld::NeedResponse(DKCollisionObject* objA, DKCollisionObject* objB)
{
	//DKLog("DKDynamicsWorld::NeedResponse: %ls, %ls\n", (const wchar_t*)objA->Name(), (const wchar_t*)objB->Name());
	return true;
}
