//
//  File: DKFixedConstraint.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKFixedConstraint.h"

namespace DKGL
{
	namespace Private
	{
		struct FixedConstraintExt : public btFixedConstraint
		{
			using btFixedConstraint::m_frameInA;
			using btFixedConstraint::m_frameInB;
		};
	}
}
using namespace DKGL;
using namespace DKGL::Private;


DKFixedConstraint::DKFixedConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKNSTransform& ta, const DKNSTransform& tb)
: DKConstraint(LinkType::Fixed, rbA, rbB, NewConstraintImpl<btFixedConstraint>::Create(rbA, rbB, ta, tb))
{
	DKASSERT_DEBUG(dynamic_cast<btFixedConstraint*>(this->impl));

	this->disableCollisionsBetweenLinkedBodies = true;
}

DKFixedConstraint::DKFixedConstraint(DKRigidBody* rbA, const DKNSTransform& ta)
: DKFixedConstraint(rbA, NULL, ta, DKNSTransform::identity)
{
}

DKFixedConstraint::DKFixedConstraint(void)
: DKFixedConstraint(NULL, NULL, DKNSTransform::identity, DKNSTransform::identity)
{
}

DKFixedConstraint::~DKFixedConstraint(void)
{
	DKASSERT_DEBUG(dynamic_cast<btFixedConstraint*>(this->impl));
}

DKNSTransform DKFixedConstraint::FrameA(void) const
{
	return BulletTransform(static_cast<FixedConstraintExt*>(this->impl)->m_frameInA);
}

DKNSTransform DKFixedConstraint::FrameB(void) const
{
	return BulletTransform(static_cast<FixedConstraintExt*>(this->impl)->m_frameInB);
}

void DKFixedConstraint::SetFrameA(const DKNSTransform& t)
{
	DKASSERT_DEBUG(dynamic_cast<btFixedConstraint*>(this->impl));

	FixedConstraintExt* c = static_cast<FixedConstraintExt*>(this->impl);
	c->m_frameInA = BulletTransform(t);
}

void DKFixedConstraint::SetFrameB(const DKNSTransform& t)
{
	DKASSERT_DEBUG(dynamic_cast<btFixedConstraint*>(this->impl));

	FixedConstraintExt* c = static_cast<FixedConstraintExt*>(this->impl);
	c->m_frameInB = BulletTransform(t);
}

void DKFixedConstraint::SetFrames(const DKNSTransform& a, const DKNSTransform& b)
{
	DKASSERT_DEBUG(dynamic_cast<btFixedConstraint*>(this->impl));

	FixedConstraintExt* c = static_cast<FixedConstraintExt*>(this->impl);
	c->m_frameInA = BulletTransform(a);
	c->m_frameInB = BulletTransform(b);
}

void DKFixedConstraint::ResetContext(void)
{
	DKASSERT_DEBUG(dynamic_cast<btFixedConstraint*>(this->impl));

	FixedConstraintExt* c = static_cast<FixedConstraintExt*>(this->impl);
	btTransform frameA = c->m_frameInA;
	btTransform frameB = c->m_frameInB;

	DKConstraint::ResetContext();

	c->m_frameInA = frameA;
	c->m_frameInB = frameB;
}

void DKFixedConstraint::ResetContextImpl(void)
{
	DKASSERT_DEBUG(dynamic_cast<btFixedConstraint*>(this->impl));
	btFixedConstraint* c = static_cast<btFixedConstraint*>(this->impl);
	c->~btFixedConstraint();
	NewConstraintImpl<btFixedConstraint>::Reset(c, bodyA, bodyB, DKNSTransform::identity, DKNSTransform::identity);
}

DKObject<DKModel> DKFixedConstraint::Clone(UUIDObjectMap& uuids) const
{
	DKObject<DKRigidBody> rbA = bodyA;
	DKObject<DKRigidBody> rbB = bodyB;
	return DKObject<DKFixedConstraint>::New(rbA, rbB, FrameA(), FrameB())->Copy(uuids, this);
}

DKFixedConstraint* DKFixedConstraint::Copy(UUIDObjectMap& uuids, const DKFixedConstraint* obj)
{
	if (DKConstraint::Copy(uuids, obj))
	{
		this->SetFrames(obj->FrameA(), obj->FrameB());
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKFixedConstraint::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKSerializer* Init(DKFixedConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKFixedConstraint");
			this->Bind(L"super", target->DKConstraint::Serializer(), NULL);

			this->Bind(L"frameA",
				DKFunction([this](DKVariant& v) {TransformToVariant(v, this->frameA); }),
				DKFunction([this](DKVariant& v) {VariantToTransform(v, this->frameA); }),
				DKFunction([](const DKVariant& v){return CheckTransformVariant(v); }),
				NULL);

			this->Bind(L"frameB",
				DKFunction([this](DKVariant& v) {TransformToVariant(v, this->frameB); }),
				DKFunction([this](DKVariant& v) {VariantToTransform(v, this->frameB); }),
				DKFunction([](const DKVariant& v){return CheckTransformVariant(v); }),
				NULL);

			this->SetCallback(DKFunction([this](DKSerializer::State s) {
				if (s == StateDeserializeBegin)
				{
					this->frameA = target->FrameA();
					this->frameB = target->FrameB();
				}
				else if (s == StateDeserializeBegin)
				{
					this->frameA.Identity();
					this->frameB.Identity();
				}
				else if (s == StateDeserializeSucceed)
				{
					target->SetFrames(this->frameA, this->frameB);
				}
			}));

			return this;
		}

		DKNSTransform frameA;
		DKNSTransform frameB;
		DKObject<DKFixedConstraint> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
