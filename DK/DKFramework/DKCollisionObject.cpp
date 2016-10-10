//
//  File: DKCollisionObject.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKCollisionObject.h"

using namespace DKGL;
using namespace DKGL;
using namespace DKGL::Private;

DKCollisionObject::DKCollisionObject(void)
: DKModel(TypeCollision)
, objectType(CollisionObject)
, impl(new btCollisionObject())
{
	impl->setUserPointer(this);
	this->SetCollisionShape(NULL);
}

DKCollisionObject::DKCollisionObject(ObjectType t, btCollisionObject* co)
: DKModel(TypeCollision)
, objectType(t)
, impl(co)
{
	impl->setUserPointer(this);
	this->SetCollisionShape(NULL);
}

DKCollisionObject::~DKCollisionObject(void)
{
	if (impl)
	{
		DKASSERT_DEBUG(impl->getUserPointer() == this);
		delete impl;
	}
}

void DKCollisionObject::SetWorldTransform(const DKNSTransform& t)
{
	DKModel::SetWorldTransform(t);
	this->impl->setWorldTransform(BulletTransform(WorldTransform()));
	this->impl->activate(true);
}

void DKCollisionObject::SetLocalTransform(const DKNSTransform& t)
{
	DKModel::SetLocalTransform(t);
	this->impl->setWorldTransform(BulletTransform(WorldTransform()));
	this->impl->activate(true);
}

void DKCollisionObject::Activate(bool force)
{
	this->impl->activate(force);
}

void DKCollisionObject::KeepActivating(bool active)
{
	if (active)
	{
		this->impl->setActivationState(DISABLE_DEACTIVATION);
	}
	else
	{
		this->impl->forceActivationState(ACTIVE_TAG);
		this->impl->setDeactivationTime(0);
	}
}

void DKCollisionObject::SetContactResponse(bool response)
{
	int flags = this->impl->getCollisionFlags();

	if (response)
		flags = flags & ~btCollisionObject::CF_NO_CONTACT_RESPONSE;
	else
		flags = flags | btCollisionObject::CF_NO_CONTACT_RESPONSE;
	this->impl->setCollisionFlags(flags);
}

bool DKCollisionObject::IsActive(void) const
{
	return this->impl->isActive();
}

bool DKCollisionObject::IsStatic(void) const
{
	return this->impl->isStaticObject();
}

bool DKCollisionObject::IsKinematic(void) const
{
	return this->impl->isKinematicObject();
}

void DKCollisionObject::SetKinematic(bool k)
{
	int flags = impl->getCollisionFlags();
	if (k)
	{
		impl->setCollisionFlags(flags | btCollisionObject::CF_KINEMATIC_OBJECT);
		impl->setActivationState(DISABLE_DEACTIVATION);
	}
	else
	{
		impl->setCollisionFlags(flags & ~btCollisionObject::CF_KINEMATIC_OBJECT);
		impl->forceActivationState(ACTIVE_TAG);
		impl->setDeactivationTime(0);
	}
}

bool DKCollisionObject::HasContactResponse(void) const
{
	return this->impl->hasContactResponse();
}

void DKCollisionObject::SetRestitution(float f)
{
	this->impl->setRestitution(f);
}

float DKCollisionObject::Restitution(void) const
{
	return this->impl->getRestitution();
}

void DKCollisionObject::SetFriction(float f)
{
	this->impl->setFriction(f);
}

float DKCollisionObject::Friction(void) const
{
	return this->impl->getFriction();
}

void DKCollisionObject::SetRollingFriction(float f)
{
	this->impl->setRollingFriction(f);
}

float DKCollisionObject::RollingFriction(void) const
{
	return this->impl->getRollingFriction();
}

void DKCollisionObject::SetHitFraction(float f)
{
	this->impl->setHitFraction(f);
}
float DKCollisionObject::HitFraction(void) const
{
	return this->impl->getHitFraction();
}

void DKCollisionObject::SetCollisionShape(DKCollisionShape* s)
{
	if (s)
	{
		btCollisionShape* shape = BulletCollisionShape(s);
		DKASSERT_DEBUG(shape->getUserPointer() == s);
		this->collisionShape = s;
		this->impl->setCollisionShape(shape);
	}
	else
	{
		// use btEmptyShape if no collisionShape exists.
		static btEmptyShape emptyShape;

		this->collisionShape = NULL;
		this->impl->setCollisionShape(&emptyShape);
	}
}

DKCollisionShape* DKCollisionObject::CollisionShape(void)
{
	return collisionShape;
}

const DKCollisionShape* DKCollisionObject::CollisionShape(void) const
{
	return collisionShape;
}

DKCollisionObject* DKCollisionObject::Copy(UUIDObjectMap& uuids, const DKCollisionObject* co)
{
	if (DKModel::Copy(uuids, co))
		return this;
	return NULL;
}

DKObject<DKSerializer> DKCollisionObject::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKObject<DKCollisionObject> target;
		DKObject<DKCollisionShape> collisionShape;

		// objectType
		DKString ObjectTypeToString(ObjectType type) const
		{
			switch (type)
			{
			case CollisionObject:	return L"collision";
			case RigidBody:			return L"rigid";
			case SoftBody:			return L"soft";
			case GhostObject:		return L"ghost";
			}
			return L"unknown";
		}
		bool StringToObjectType(const DKString& s, ObjectType* p) const
		{
			if (s.CompareNoCase(ObjectTypeToString(CollisionObject)) == 0)	{ *p = CollisionObject;	return true; }
			if (s.CompareNoCase(ObjectTypeToString(RigidBody)) == 0)		{ *p = RigidBody;	return true; }
			if (s.CompareNoCase(ObjectTypeToString(SoftBody)) == 0)			{ *p = SoftBody;	return true; }
			if (s.CompareNoCase(ObjectTypeToString(GhostObject)) == 0)		{ *p = GhostObject;	return true; }
			return false;
		}
		bool CheckBodyType(const DKVariant& v) const
		{
			ObjectType t;
			return v.ValueType() == DKVariant::TypeString && StringToObjectType(v.String(), &t) && t == this->target->objectType;
		}

		// collision shape
		void GetCollisionShape(ExternalType& v)
		{
			DKObject<DKCollisionShape::SerializeHelper> helper = DKObject<DKCollisionShape::SerializeHelper>::New();
			helper->shape = this->collisionShape;
			v = helper.SafeCast<DKResource>();
		}
		void SetCollisionShape(ExternalType& v)
		{
			DKObject<DKCollisionShape::SerializeHelper> helper = v.SafeCast<DKCollisionShape::SerializeHelper>();
			this->collisionShape = helper->shape;
		}
		bool CheckCollisionShape(const ExternalType& v) const
		{
			return v.SafeCast<DKCollisionShape::SerializeHelper>() != NULL;
		}

		// callback
		void Callback(State s)
		{
			if (s == StateSerializeBegin)
			{
				this->collisionShape = target->collisionShape;
			}
			else if (s == StateDeserializeBegin)
			{
				collisionShape = NULL;
			}
			else if (s == StateDeserializeSucceed)
			{
				target->SetCollisionShape(collisionShape);
			}
		}

		DKSerializer* Init(DKCollisionObject* p)
		{
			if (p == NULL)
				return NULL;

			this->target = p;
			this->SetResourceClass(L"DKCollisionObject");
			this->Bind(L"super", target->DKModel::Serializer(), NULL);
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			// ObjectType should be matched.
			this->Bind(L"objectType",
				DKFunction([this](DKVariant& v) {v.SetString(ObjectTypeToString(this->target->objectType)); }),
				DKFunction([this](DKVariant& v) {}),
				DKFunction(this, &LocalSerializer::CheckBodyType),
				NULL);

			this->Bind(L"collisionShape",
				DKFunction(this, &LocalSerializer::GetCollisionShape),
				DKFunction(this, &LocalSerializer::SetCollisionShape),
				DKFunction(this, &LocalSerializer::CheckCollisionShape),
				DKSerializer::ExternalResourceInclude,
				DKFunction([this] { target->collisionShape = NULL; })->Invocation());

			return this;
		}
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
