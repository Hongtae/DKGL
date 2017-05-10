//
//  File: DKGeneric6DofConstraint.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletPhysics.h"
#include "DKGearConstraint.h"

namespace DKFramework
{
	namespace Private
	{
		struct GearConstraintExt : public btGearConstraint
		{
			using btGearConstraint::m_axisInA;
			using btGearConstraint::m_axisInB;
			using btGearConstraint::m_ratio;
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKGearConstraint::DKGearConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKVector3& axisA, const DKVector3& axisB, float ratio)
: DKConstraint(LinkType::Gear, rbA, rbB, NewConstraintImpl<btGearConstraint>::Create(rbA, rbB, axisA,axisB,ratio))
{
	DKASSERT_DEBUG(dynamic_cast<btGearConstraint*>(this->impl));
}

DKGearConstraint::DKGearConstraint(void)
: DKGearConstraint(NULL, NULL, DKVector3::zero, DKVector3::zero, 1.0)
{
	DKASSERT_DEBUG(dynamic_cast<btGearConstraint*>(this->impl));
}

DKGearConstraint::~DKGearConstraint(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGearConstraint*>(this->impl));
}

void DKGearConstraint::SetAxisInA(const DKVector3& axis)
{
	GearConstraintExt* c = static_cast<GearConstraintExt*>(this->impl);
	c->m_axisInA = BulletVector3(axis);
}

void DKGearConstraint::SetAxisInB(const DKVector3& axis)
{
	GearConstraintExt* c = static_cast<GearConstraintExt*>(this->impl);
	c->m_axisInB = BulletVector3(axis);
}

DKVector3 DKGearConstraint::AxisInA(void) const
{
	GearConstraintExt* c = static_cast<GearConstraintExt*>(this->impl);
	return BulletVector3(c->m_axisInA);
}

DKVector3 DKGearConstraint::AxisInB(void) const
{
	GearConstraintExt* c = static_cast<GearConstraintExt*>(this->impl);
	return BulletVector3(c->m_axisInB);
}

float DKGearConstraint::Ratio(void) const
{
	GearConstraintExt* c = static_cast<GearConstraintExt*>(this->impl);
	return c->m_ratio;
}

void DKGearConstraint::SetRatio(float r)
{
	GearConstraintExt* c = static_cast<GearConstraintExt*>(this->impl);
	c->m_ratio = r;
}

void DKGearConstraint::ResetContext(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGearConstraint*>(this->impl));
	btGearConstraint* c = static_cast<btGearConstraint*>(this->impl);

	btVector3 axisA = static_cast<GearConstraintExt*>(c)->m_axisInA;
	btVector3 axisB = static_cast<GearConstraintExt*>(c)->m_axisInB;
	float ratio = static_cast<GearConstraintExt*>(c)->m_ratio;

	DKConstraint::ResetContext();

	static_cast<GearConstraintExt*>(c)->m_ratio = ratio;
	static_cast<GearConstraintExt*>(c)->m_axisInA = axisA;
	static_cast<GearConstraintExt*>(c)->m_axisInB = axisB;
}

void DKGearConstraint::ResetContextImpl(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGearConstraint*>(this->impl));
	btGearConstraint* c = static_cast<btGearConstraint*>(this->impl);
	c->~btGearConstraint();
	NewConstraintImpl<btGearConstraint>::Reset(c, bodyA, bodyB, DKVector3::zero, DKVector3::zero);
}

DKObject<DKModel> DKGearConstraint::Clone(UUIDObjectMap& uuids) const
{
	DKObject<DKRigidBody> rbA = bodyA;
	DKObject<DKRigidBody> rbB = bodyB;
	return DKObject<DKGearConstraint>::New(rbA, rbB, AxisInA(), AxisInB())->Copy(uuids, this);
}

DKGearConstraint* DKGearConstraint::Copy(UUIDObjectMap& uuids, const DKGearConstraint* obj)
{
	if (DKConstraint::Copy(uuids, obj))
	{
		GearConstraintExt* dst = static_cast<GearConstraintExt*>(this->impl);
		GearConstraintExt* src = static_cast<GearConstraintExt*>(obj->impl);

		dst->m_axisInA = src->m_axisInA;
		dst->m_axisInB = src->m_axisInB;
		dst->m_ratio = src->m_ratio;
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKGearConstraint::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKSerializer* Init(DKGearConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKGearConstraint");
			this->Bind(L"super", target->DKConstraint::Serializer(), NULL);

			this->Bind(L"axisA",
				DKFunction([this](DKVariant& v) {v.SetVector3(this->axisA); }),
				DKFunction([this](DKVariant& v) {this->axisA = v.Vector3(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeVector3; }),
				NULL);

			this->Bind(L"axisB",
				DKFunction([this](DKVariant& v) {v.SetVector3(this->axisB); }),
				DKFunction([this](DKVariant& v) {this->axisB = v.Vector3(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeVector3; }),
				NULL);

			this->Bind(L"ratio",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->ratio); }),
				DKFunction([this](DKVariant& v) {this->ratio = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				NULL);

			this->SetCallback(DKFunction([this](DKSerializer::State s)
			{
				if (s == StateSerializeBegin)
				{
					this->axisA = target->AxisInA();
					this->axisB = target->AxisInB();
					this->ratio = target->Ratio();
				}
				else if (s == StateDeserializeBegin)
				{
					this->axisA = DKVector3(0, 1, 0);
					this->axisB = DKVector3(0, 1, 0);
					this->ratio = 1.0;
				}
				else if (s == StateDeserializeSucceed)
				{
					target->SetAxisInA(this->axisA);
					target->SetAxisInB(this->axisB);
					target->SetRatio(this->ratio);
				}
			}));

			return this;
		}
		float ratio;
		DKVector3 axisA;
		DKVector3 axisB;
		DKObject<DKGearConstraint> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}

