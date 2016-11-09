//
//  File: DKRigidBody.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKRigidBody.h"
#include "DKConstraint.h"

namespace DKFramework
{
	namespace Private
	{
		struct RigidBodyExt : public btRigidBody
		{
			btScalar& linearDamping()							{ return m_linearDamping; }
			btScalar& angularDamping()							{ return m_angularDamping; }
			btScalar& friction()								{ return m_friction; }
			btScalar& rollingFriction()							{ return m_rollingFriction; }
			btScalar& restitution()								{ return m_restitution; }
			btScalar& linearSleepingThreshold()					{ return m_linearSleepingThreshold; }
			btScalar& angularSleepingThreshold()				{ return m_angularSleepingThreshold; }
			bool& additionalDamping()							{ return m_additionalDamping; }
			btScalar& additionalDampingFactor()					{ return m_additionalDampingFactor; }
			btScalar& additionalLinearDampingThresholdSqr()		{ return m_additionalLinearDampingThresholdSqr; }
			btScalar& additionalAngularDampingThresholdSqr()	{ return m_additionalAngularDampingThresholdSqr; }
			btScalar& additionalAngularDampingFactor()			{ return m_additionalAngularDampingFactor; }
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKRigidBody::ObjectData::ObjectData(float m, const DKVector3& inertia)
: mass(m)
, localInertia(inertia)
{
	btRigidBody::btRigidBodyConstructionInfo ci(0, 0, 0);
	linearDamping = ci.m_linearDamping;
	angularDamping = ci.m_angularDamping;
	friction = ci.m_friction;
	rollingFriction = ci.m_rollingFriction;
	restitution = ci.m_restitution;
	linearSleepingThreshold = ci.m_linearSleepingThreshold;
	angularSleepingThreshold = ci.m_angularSleepingThreshold;
	additionalDamping = ci.m_additionalDamping;
	additionalDampingFactor = ci.m_additionalDampingFactor;
	additionalLinearDampingThresholdSqr = ci.m_additionalLinearDampingThresholdSqr;
	additionalAngularDampingThresholdSqr = ci.m_additionalAngularDampingThresholdSqr;
	additionalAngularDampingFactor = ci.m_additionalAngularDampingFactor;
}

bool DKRigidBody::GetObjectData(ObjectData& data) const
{
	RigidBodyExt* body = static_cast<RigidBodyExt*>(btRigidBody::upcast(this->impl));
	if (body)
	{
		data.mass = this->Mass();
		data.localInertia = this->LocalInertia();
		data.linearDamping = body->linearDamping();
		data.angularDamping = body->angularDamping();
		data.friction = body->friction();
		data.rollingFriction = body->rollingFriction();
		data.restitution = body->restitution();
		data.linearSleepingThreshold = body->linearSleepingThreshold();
		data.angularSleepingThreshold = body->angularSleepingThreshold();
		data.additionalDamping = body->additionalDamping();
		data.additionalDampingFactor = body->additionalDampingFactor();
		data.additionalLinearDampingThresholdSqr = body->additionalLinearDampingThresholdSqr();
		data.additionalAngularDampingThresholdSqr = body->additionalAngularDampingThresholdSqr();
		data.additionalAngularDampingFactor = body->additionalAngularDampingFactor();
		return true;
	}
	return false;
}

bool DKRigidBody::ResetObject(DKCollisionShape* shape, const ObjectData& data)
{
	if (this->Scene() == NULL)
	{
		btTransform trans = BulletTransform(worldTransform);
		this->motionState->setWorldTransform(trans);

		btCollisionShape* cs = NULL;
		if (shape)
		{
			cs = BulletCollisionShape(shape);
			DKASSERT_DEBUG(cs->getUserPointer() == shape);
		}

		btRigidBody::btRigidBodyConstructionInfo ci(data.mass, this->motionState, cs, BulletVector3(data.localInertia));
		ci.m_linearDamping = data.linearDamping;
		ci.m_angularDamping = data.angularDamping;
		ci.m_friction = data.friction;
		ci.m_rollingFriction = data.rollingFriction;
		ci.m_restitution = data.restitution;
		ci.m_linearSleepingThreshold = data.linearSleepingThreshold;
		ci.m_angularSleepingThreshold = data.angularSleepingThreshold;
		ci.m_additionalDamping = data.additionalDamping;
		ci.m_additionalDampingFactor = data.additionalDampingFactor;
		ci.m_additionalLinearDampingThresholdSqr = data.additionalLinearDampingThresholdSqr;
		ci.m_additionalAngularDampingThresholdSqr = data.additionalAngularDampingThresholdSqr;
		ci.m_additionalAngularDampingFactor = data.additionalAngularDampingFactor;

		this->impl->~btCollisionObject();
		new (this->impl) btRigidBody(ci);
		this->impl->setUserPointer(this);

		this->SetCollisionShape(shape);

		return true;
	}
	return false;
}

DKRigidBody::DKRigidBody(const DKString& name)
: DKCollisionObject(ObjectType::RigidBody, new btRigidBody(0, 0, 0))
, motionState(new btDefaultMotionState())
{
	SetName(name);
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->setMassProps(0, btVector3(0, 0, 0));
	body->updateInertiaTensor();
	body->setMotionState(this->motionState);
}

DKRigidBody::DKRigidBody(DKCollisionShape* shape, float mass)
: DKCollisionObject(ObjectType::RigidBody, new btRigidBody(0, 0, 0))
, motionState(new btDefaultMotionState())
{
	if (shape)
	{
		btCollisionShape* cs = BulletCollisionShape(shape);
		DKASSERT_DEBUG(cs->getUserPointer() == shape);
		DKVector3 inertia(0, 0, 0);
		if (mass != 0.0)
		{
			inertia = shape->CalculateLocalInertia(mass);
		}
		btRigidBody::btRigidBodyConstructionInfo ci(mass, this->motionState, cs, BulletVector3(inertia));
		this->impl->~btCollisionObject();
		new (this->impl) btRigidBody(ci);
		this->impl->setUserPointer(this);
		this->SetCollisionShape(shape);
	}
	else
	{
		btRigidBody* body = btRigidBody::upcast(this->impl);
		DKASSERT_DEBUG(body);
		body->setMassProps(mass, btVector3(0, 0, 0));
		body->updateInertiaTensor();
		body->setMotionState(this->motionState);
	}
}

DKRigidBody::DKRigidBody(DKCollisionShape* shape, float mass, const DKVector3& inertia)
: DKCollisionObject(ObjectType::RigidBody, new btRigidBody(0, 0, 0))
, motionState(new btDefaultMotionState())
{
	btCollisionShape* cs = NULL;
	if (shape)
	{
		cs = BulletCollisionShape(shape);
		DKASSERT_DEBUG(cs->getUserPointer() == shape);
	}

	btRigidBody::btRigidBodyConstructionInfo ci(mass, this->motionState, cs, BulletVector3(inertia));
	this->impl->~btCollisionObject();
	new (this->impl) btRigidBody(ci);
	this->impl->setUserPointer(this);
	this->SetCollisionShape(shape);
}

DKRigidBody::DKRigidBody(DKCollisionShape* shape, const ObjectData& data)
: DKCollisionObject(ObjectType::RigidBody, new btRigidBody(0, 0, 0))
, motionState(new btDefaultMotionState())
{
	bool b = ResetObject(shape, data);
	DKASSERT_DEBUG(b);
}

DKRigidBody::~DKRigidBody(void)
{
	btRigidBody* rb = btRigidBody::upcast(impl);
	DKASSERT_DEBUG(rb);
	DKASSERT_DEBUG(rb->getUserPointer() == this);
	DKASSERT_DEBUG(rb->getNumConstraintRefs() == 0);

	delete impl;
	impl = NULL;

	delete motionState;
	motionState = NULL;
}

void DKRigidBody::OnAddedToParent(void)
{
	DKModel* p = this->Parent();
	if (p)
	{
		btRigidBody* body = btRigidBody::upcast(this->impl);
		DKASSERT_DEBUG(body);

		DKASSERT_DEBUG(this->Scene() == NULL);

		this->worldTransform = this->localTransform * p->WorldTransform();
		btTransform trans = BulletTransform(this->worldTransform);
		this->motionState->setWorldTransform(trans);
		body->setCenterOfMassTransform(trans);
	}
}

void DKRigidBody::SetWorldTransform(const DKNSTransform& t)
{
	DKModel::SetWorldTransform(t);
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	btTransform trans = BulletTransform(worldTransform);
	motionState->setWorldTransform(trans);
	body->setCenterOfMassTransform(trans);
	body->activate(true);
}

void DKRigidBody::SetLocalTransform(const DKNSTransform& t)
{
	DKModel::SetLocalTransform(t);
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	btTransform trans = BulletTransform(worldTransform);
	motionState->setWorldTransform(trans);
	body->setCenterOfMassTransform(trans);
	body->activate(true);
}

DKNSTransform DKRigidBody::CenterOfMassTransform(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletTransform(body->getCenterOfMassTransform());
}

DKVector3 DKRigidBody::CenterOfMassPosition(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getCenterOfMassPosition());
}

void DKRigidBody::SetMass(float mass)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);

	btVector3 invInertia = body->getInvInertiaDiagLocal();
	btVector3 inertia = btVector3(
		invInertia.x() != 0.0f ? 1.0f / invInertia.x() : 0.0f,
		invInertia.y() != 0.0f ? 1.0f / invInertia.y() : 0.0f,
		invInertia.z() != 0.0f ? 1.0f / invInertia.z() : 0.0f
		);

	body->setMassProps(mass, inertia);
	body->updateInertiaTensor();
}

float DKRigidBody::Mass(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	float m = body->getInvMass();
	if (m != 0.0f)
		return 1.0f / m;
	return m;
}

float DKRigidBody::InverseMass(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return body->getInvMass();
}

void DKRigidBody::SetLocalInertia(const DKVector3& inertia)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	float mass = body->getInvMass();
	if (mass != 0.0f)
		mass = 1.0f / mass;
	body->setMassProps(mass, BulletVector3(inertia));
	body->updateInertiaTensor();
}

DKVector3 DKRigidBody::LocalInertia(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);

	btVector3 invInertia = body->getInvInertiaDiagLocal();
	return DKVector3(
		invInertia.x() != 0.0f ? 1.0f / invInertia.x() : 0.0f,
		invInertia.y() != 0.0f ? 1.0f / invInertia.y() : 0.0f,
		invInertia.z() != 0.0f ? 1.0f / invInertia.z() : 0.0f
		);
}

DKVector3 DKRigidBody::InverseDiagLocalInertia(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getInvInertiaDiagLocal());
}

DKMatrix3 DKRigidBody::InverseWorldInertiaTensor(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletMatrix3(body->getInvInertiaTensorWorld());
}

void DKRigidBody::SetLinearVelocity(const DKVector3& v)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->setLinearVelocity(BulletVector3(v));
}

DKVector3 DKRigidBody::LinearVelocity(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getLinearVelocity());
}

void DKRigidBody::SetAngularVelocity(const DKVector3& v)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->setAngularVelocity(BulletVector3(v));
}

DKVector3 DKRigidBody::AngularVelocity(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getAngularVelocity());
}

void DKRigidBody::SetLinearFactor(const DKVector3& f)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->setLinearFactor(BulletVector3(f));
}

DKVector3 DKRigidBody::LinearFactor(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getLinearFactor());
}

void DKRigidBody::SetAngularFactor(const DKVector3& f)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->setAngularFactor(BulletVector3(f));
}

DKVector3 DKRigidBody::AngularFactor(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getAngularFactor());
}

void DKRigidBody::SetLinearDamping(float d)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	btScalar ang = body->getAngularDamping();
	body->setDamping(d, ang);
}

float DKRigidBody::LinearDamping(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	if (body)
		return body->getLinearDamping();
	return 0;
}

void DKRigidBody::SetAngularDamping(float d)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	btScalar lin = body->getLinearDamping();
	body->setDamping(lin, d);
}

float DKRigidBody::AngularDamping(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return body->getAngularDamping();
}

DKVector3 DKRigidBody::TotalForce(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getTotalForce());
}

DKVector3 DKRigidBody::TotalTorque(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getTotalTorque());
}

DKVector3 DKRigidBody::VelocityInLocalPoint(const DKVector3& pos) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->getVelocityInLocalPoint(BulletVector3(pos)));
}

float DKRigidBody::ComputeImpulseDenominator(const DKVector3& pos, const DKVector3& normal) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return body->computeImpulseDenominator(BulletVector3(pos), BulletVector3(normal));
}

float DKRigidBody::ComputeAngularImpulseDenominator(const DKVector3& axis) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return body->computeAngularImpulseDenominator(BulletVector3(axis));
}

DKVector3 DKRigidBody::ComputeGyroscopicForce(float maxGyroscopicForce) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return BulletVector3(body->computeGyroscopicForce(maxGyroscopicForce));
}

void DKRigidBody::ApplyForce(const DKVector3& force, const DKVector3& relpos)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->applyForce(BulletVector3(force), BulletVector3(relpos));
}

void DKRigidBody::ApplyImpulse(const DKVector3& impulse, const DKVector3& relpos)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->applyImpulse(BulletVector3(impulse), BulletVector3(relpos));
}

void DKRigidBody::ApplyTorque(const DKVector3& torque)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->applyTorque(BulletVector3(torque));
}

void DKRigidBody::ApplyCentralForce(const DKVector3& force)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->applyCentralForce(BulletVector3(force));
}

void DKRigidBody::ApplyCentralImpulse(const DKVector3& impulse)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->applyCentralImpulse(BulletVector3(impulse));
}

void DKRigidBody::ApplyTorqueImpulse(const DKVector3& torque)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->applyTorqueImpulse(BulletVector3(torque));
}

void DKRigidBody::ClearForces()
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	body->clearForces();
}

void DKRigidBody::SetLinearSleepingThreshold(float linear)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	float angular = body->getAngularSleepingThreshold();
	body->setSleepingThresholds(linear, angular);
}

void DKRigidBody::SetAngularSleepingThreshold(float angular)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	float linear = body->getLinearSleepingThreshold();
	body->setSleepingThresholds(linear, angular);
}

float DKRigidBody::LinearSleepingThreshold(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return body->getLinearSleepingThreshold();
}

float DKRigidBody::AngularSleepingThreshold(void) const
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	return body->getAngularSleepingThreshold();
}

void DKRigidBody::OnSetAnimation(DKAnimatedTransform* ani)
{
	this->SetKinematic(ani != NULL);
}

void DKRigidBody::OnUpdateSceneState(const DKNSTransform& parentWorldTransform)
{
	btRigidBody* body = btRigidBody::upcast(this->impl);
	DKASSERT_DEBUG(body);
	if (body->isInWorld())
	{
		DKASSERT_DEBUG(this->Scene() != NULL);

		btTransform trans;
		this->motionState->getWorldTransform(trans);
		DKNSTransform t = BulletTransform(trans);
		this->worldTransform = t;
		if (this->Parent())
			this->localTransform = t * DKNSTransform(parentWorldTransform).Inverse();
		else
			this->localTransform = t;
	}
	else
	{
		DKASSERT_DEBUG(this->Scene() == NULL);

		this->worldTransform = this->localTransform * parentWorldTransform;
		btTransform trans = BulletTransform(this->worldTransform);
		this->motionState->setWorldTransform(trans);
		body->setCenterOfMassTransform(trans);
	}
}

DKObject<DKModel> DKRigidBody::Clone(UUIDObjectMap& uuids) const
{
	return DKObject<DKRigidBody>::New()->Copy(uuids, this);
}

DKRigidBody* DKRigidBody::Copy(UUIDObjectMap& uuids, const DKRigidBody* rb)
{
	if (DKCollisionObject::Copy(uuids, rb))
	{
		ObjectData data;
		DKObject<DKCollisionShape> sp = rb->collisionShape;
		if (rb->GetObjectData(data) && this->ResetObject(sp, data))
			return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKRigidBody::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKObject<DKRigidBody> target;
		ObjectData data;

		void Callback(State s)
		{
			if (s == StateSerializeBegin)
			{
				target->GetObjectData(data);
			}
			else if (s == StateDeserializeBegin)
			{
				data = ObjectData();
			}
			else if (s == StateDeserializeSucceed)
			{
				DKObject<DKCollisionShape> shape = target->CollisionShape();
				target->ResetObject(shape, data);
			}
		}
		// rigid body data
		void GetObjectData(DKVariant& v)
		{
			DKVariant::VPairs& pairs = v.SetValueType(DKVariant::TypePairs).Pairs();
			pairs.Update(L"mass", (DKVariant::VFloat&)data.mass);
			pairs.Update(L"localInertia", (DKVariant::VVector3&)data.localInertia);
			pairs.Update(L"linearDamping", (DKVariant::VFloat&)data.linearDamping);
			pairs.Update(L"angularDamping", (DKVariant::VFloat&)data.angularDamping);
			pairs.Update(L"friction", (DKVariant::VFloat&)data.friction);
			pairs.Update(L"rollingFriction", (DKVariant::VFloat&)data.rollingFriction);
			pairs.Update(L"restitution", (DKVariant::VFloat&)data.restitution);
			pairs.Update(L"linearSleepingThreshold", (DKVariant::VFloat&)data.linearSleepingThreshold);
			pairs.Update(L"angularSleepingThreshold", (DKVariant::VFloat&)data.angularSleepingThreshold);
			pairs.Update(L"additionalDamping", (DKVariant::VInteger)data.additionalDamping);
			pairs.Update(L"additionalDampingFactor", (DKVariant::VFloat&)data.additionalDampingFactor);
			pairs.Update(L"additionalLinearDampingThresholdSqr", (DKVariant::VFloat&)data.additionalLinearDampingThresholdSqr);
			pairs.Update(L"additionalAngularDampingThresholdSqr", (DKVariant::VFloat&)data.additionalAngularDampingThresholdSqr);
			pairs.Update(L"additionalAngularDampingFactor", (DKVariant::VFloat&)data.additionalAngularDampingFactor);
		}
		void SetObjectData(DKVariant& v)
		{
			DKVariant::VPairs& pairs = v.Pairs();
			const DKVariant::VPairs::Pair* p = NULL;
			p = pairs.Find(L"mass");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.mass = p->value.Float();
			p = pairs.Find(L"localInertia");
			if (p && p->value.ValueType() == DKVariant::TypeVector3)
				data.localInertia = p->value.Vector3();
			p = pairs.Find(L"linearDamping");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.linearDamping = p->value.Float();
			p = pairs.Find(L"angularDamping");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.angularDamping = p->value.Float();
			p = pairs.Find(L"friction");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.friction = p->value.Float();
			p = pairs.Find(L"rollingFriction");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.rollingFriction = p->value.Float();
			p = pairs.Find(L"restitution");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.restitution = p->value.Float();
			p = pairs.Find(L"linearSleepingThreshold");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.linearSleepingThreshold = p->value.Float();
			p = pairs.Find(L"angularSleepingThreshold");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.angularSleepingThreshold = p->value.Float();
			p = pairs.Find(L"additionalDamping");
			if (p && p->value.ValueType() == DKVariant::TypeInteger)
				data.additionalDamping = p->value.Integer() != 0;
			p = pairs.Find(L"additionalDampingFactor");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.additionalDampingFactor = p->value.Float();
			p = pairs.Find(L"additionalLinearDampingThresholdSqr");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.additionalLinearDampingThresholdSqr = p->value.Float();
			p = pairs.Find(L"additionalAngularDampingThresholdSqr");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.additionalAngularDampingThresholdSqr = p->value.Float();
			p = pairs.Find(L"additionalAngularDampingFactor");
			if (p && p->value.ValueType() == DKVariant::TypeFloat)
				data.additionalAngularDampingFactor = p->value.Float();
		}
		bool CheckObjectData(const DKVariant& v)
		{
			return v.ValueType() == DKVariant::TypePairs;
		}

		DKSerializer* Init(DKRigidBody* p)
		{
			if (p == NULL)
				return NULL;

			this->target = p;
			this->SetResourceClass(L"DKRigidBody");
			this->Bind(L"super", target->DKCollisionObject::Serializer(), NULL);
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"objectData",
				DKFunction(this, &LocalSerializer::GetObjectData),
				DKFunction(this, &LocalSerializer::SetObjectData),
				DKFunction(this, &LocalSerializer::CheckObjectData),
				NULL);

			return this;
		}
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
