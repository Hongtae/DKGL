//
//  File: DKConeTwistConstraint.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKConeTwistConstraint.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		struct ConeTwistConstraintExt : public btConeTwistConstraint
		{
			using btConeTwistConstraint::m_swingSpan1;
			using btConeTwistConstraint::m_swingSpan2;
			using btConeTwistConstraint::m_twistSpan;
			using btConeTwistConstraint::m_limitSoftness;
			using btConeTwistConstraint::m_biasFactor;
			using btConeTwistConstraint::m_relaxationFactor;
			using btConeTwistConstraint::m_damping;
			using btConeTwistConstraint::m_flags;
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKConeTwistConstraint::DKConeTwistConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKNSTransform& ta, const DKNSTransform& tb)
: DKConstraint(LinkType::ConeTwist, rbA, rbB, NewConstraintImpl<btConeTwistConstraint>::Create(rbA, rbB, ta, tb))
{
	DKASSERT_DEBUG(dynamic_cast<btConeTwistConstraint*>(BulletTypedConstraint(this)));
}

DKConeTwistConstraint::DKConeTwistConstraint(DKRigidBody* rbA, const DKNSTransform& ta)
: DKConstraint(LinkType::ConeTwist, rbA, NULL, NewConstraintImpl<btConeTwistConstraint>::Create(rbA, ta))
{
	DKASSERT_DEBUG(dynamic_cast<btConeTwistConstraint*>(BulletTypedConstraint(this)));
}

DKConeTwistConstraint::DKConeTwistConstraint(void)
: DKConeTwistConstraint(NULL, NULL, DKNSTransform::identity, DKNSTransform::identity)
{
}

DKConeTwistConstraint::~DKConeTwistConstraint(void)
{
}

void DKConeTwistConstraint::SetLimit(float swingSpan1, float swingSpan2, float twistSpan, float softness, float biasFactor, float relaxationFactor)
{
	static_cast<btConeTwistConstraint*>(this->impl)->setLimit(swingSpan1, swingSpan2, twistSpan, softness, biasFactor, relaxationFactor);
}

float DKConeTwistConstraint::SwingSpan1(void) const
{
	return static_cast<btConeTwistConstraint*>(this->impl)->getSwingSpan1();
}

void DKConeTwistConstraint::SetSwingSpan1(float f)
{
	static_cast<ConeTwistConstraintExt*>(this->impl)->m_swingSpan1 = f;
}

float DKConeTwistConstraint::SwingSpan2(void) const
{
	return static_cast<btConeTwistConstraint*>(this->impl)->getSwingSpan2();
}

void DKConeTwistConstraint::SetSwingSpan2(float f)
{
	static_cast<ConeTwistConstraintExt*>(this->impl)->m_swingSpan2 = f;
}

float DKConeTwistConstraint::TwistSpan(void) const
{
	return static_cast<btConeTwistConstraint*>(this->impl)->getTwistSpan();
}

void DKConeTwistConstraint::SetTwistSpan(float f)
{
	static_cast<ConeTwistConstraintExt*>(this->impl)->m_twistSpan = f;
}

float DKConeTwistConstraint::Softness(void) const
{
	return static_cast<ConeTwistConstraintExt*>(this->impl)->m_limitSoftness;
}

void DKConeTwistConstraint::SetSoftness(float f)
{
	static_cast<ConeTwistConstraintExt*>(this->impl)->m_limitSoftness = f;
}

float DKConeTwistConstraint::BiasFactor(void) const
{
	return static_cast<ConeTwistConstraintExt*>(this->impl)->m_biasFactor;
}

void DKConeTwistConstraint::SetBiasFactor(float f)
{
	static_cast<ConeTwistConstraintExt*>(this->impl)->m_biasFactor = f;
}

float DKConeTwistConstraint::RelaxationFactor(void) const
{
	return static_cast<ConeTwistConstraintExt*>(this->impl)->m_relaxationFactor;
}

void DKConeTwistConstraint::SetRelaxationFactor(float f)
{
	static_cast<ConeTwistConstraintExt*>(this->impl)->m_relaxationFactor = f;
}

float DKConeTwistConstraint::TwistAngle(void) const
{
	return static_cast<ConeTwistConstraintExt*>(this->impl)->getTwistAngle();
}

float DKConeTwistConstraint::Damping(void) const
{
	return static_cast<ConeTwistConstraintExt*>(this->impl)->m_damping;
}

void DKConeTwistConstraint::SetDamping(float damping)
{
	static_cast<ConeTwistConstraintExt*>(this->impl)->setDamping(damping);
}

void DKConeTwistConstraint::SetFrames(const DKNSTransform& fA, const DKNSTransform& fB)
{
	btTransform ta = BulletTransform(fA);
	btTransform tb = BulletTransform(fB);
	static_cast<ConeTwistConstraintExt*>(this->impl)->setFrames(ta, tb);
}

DKNSTransform DKConeTwistConstraint::FrameA(void) const
{
	return BulletTransform(static_cast<ConeTwistConstraintExt*>(this->impl)->getFrameOffsetA());
}

DKNSTransform DKConeTwistConstraint::FrameB(void) const
{
	return BulletTransform(static_cast<ConeTwistConstraintExt*>(this->impl)->getFrameOffsetB());
}

bool DKConeTwistConstraint::IsValidParam(ParamType type, ParamAxis axis) const
{
	switch (axis)
	{
	case ParamAxis::LinearX:
	case ParamAxis::LinearY:
	case ParamAxis::LinearZ:
	case ParamAxis::AngularX:
	case ParamAxis::AngularY:
	case ParamAxis::AngularZ:
		return true;
	}
	return false;
}

bool DKConeTwistConstraint::HasParam(ParamType type, ParamAxis axis) const
{
	int flags = static_cast<ConeTwistConstraintExt*>(this->impl)->m_flags;

	switch (type)
	{
	case ParamType::ERP:
	case ParamType::STOP_ERP:
		switch (axis)
		{
		case ParamAxis::LinearX:
		case ParamAxis::LinearY:
		case ParamAxis::LinearZ:
			return flags & BT_CONETWIST_FLAGS_LIN_ERP;
		case ParamAxis::AngularX:
		case ParamAxis::AngularY:
		case ParamAxis::AngularZ:
			return true;
		}
		break;
	case ParamType::CFM:
	case ParamType::STOP_CFM:
		switch (axis)
		{
		case ParamAxis::LinearX:
		case ParamAxis::LinearY:
		case ParamAxis::LinearZ:
			return flags & BT_CONETWIST_FLAGS_LIN_CFM;
		case ParamAxis::AngularX:
		case ParamAxis::AngularY:
		case ParamAxis::AngularZ:
			return flags & BT_CONETWIST_FLAGS_ANG_CFM;;
		}
		break;
	}
	return false;
}

void DKConeTwistConstraint::ResetContext(void)
{
	DKASSERT_DEBUG(dynamic_cast<btConeTwistConstraint*>(this->impl));
	btConeTwistConstraint* c = static_cast<btConeTwistConstraint*>(this->impl);

	btTransform frameA = c->getFrameOffsetA();
	btTransform frameB = c->getFrameOffsetB();
	float swingSpan1 = static_cast<ConeTwistConstraintExt*>(this->impl)->getSwingSpan1();
	float swingSpan2 = static_cast<ConeTwistConstraintExt*>(this->impl)->getSwingSpan2();
	float twistSpan = static_cast<ConeTwistConstraintExt*>(this->impl)->getTwistSpan();
	float softness = static_cast<ConeTwistConstraintExt*>(this->impl)->m_limitSoftness;
	float biasFactor = static_cast<ConeTwistConstraintExt*>(this->impl)->m_biasFactor;
	float relaxationFactor = static_cast<ConeTwistConstraintExt*>(this->impl)->m_relaxationFactor;
	float damping = static_cast<ConeTwistConstraintExt*>(this->impl)->m_damping;

	DKConstraint::ResetContext();

	c->setFrames(frameA, frameB);
	c->setLimit(swingSpan1, swingSpan2, twistSpan, softness, biasFactor, relaxationFactor);
	c->setDamping(damping);
}

void DKConeTwistConstraint::ResetContextImpl(void)
{
	DKASSERT_DEBUG(dynamic_cast<btConeTwistConstraint*>(this->impl));
	btConeTwistConstraint* c = static_cast<btConeTwistConstraint*>(this->impl);
	c->~btConeTwistConstraint();
	NewConstraintImpl<btConeTwistConstraint>::Reset(c, bodyA, bodyB, DKNSTransform::identity, DKNSTransform::identity);
}

DKObject<DKModel> DKConeTwistConstraint::Clone(UUIDObjectMap& uuids) const
{
	DKObject<DKRigidBody> rbA = bodyA;
	DKObject<DKRigidBody> rbB = bodyB;
	return DKObject<DKConeTwistConstraint>::New(rbA, rbB, FrameA(), FrameB())->Copy(uuids, this);
}

DKConeTwistConstraint* DKConeTwistConstraint::Copy(UUIDObjectMap& uuids, const DKConeTwistConstraint* obj)
{
	if (DKConstraint::Copy(uuids, obj))
	{
		ConeTwistConstraintExt* src = static_cast<ConeTwistConstraintExt*>(obj->impl);
		ConeTwistConstraintExt* dst = static_cast<ConeTwistConstraintExt*>(this->impl);

		dst->setFrames(src->getFrameOffsetA(), src->getFrameOffsetB());
		dst->setLimit(src->m_swingSpan1, src->m_swingSpan2, src->m_twistSpan, src->m_limitSoftness, src->m_biasFactor, src->m_relaxationFactor);
		dst->setDamping(src->m_damping);
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKConeTwistConstraint::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKSerializer* Init(DKConeTwistConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKConeTwistConstraint");
			this->Bind(L"super", target->DKConstraint::Serializer(), NULL);

			DKObject<DKOperation> dummyFunc = (DKOperation*)DKValue<int>(0);

			this->Bind(L"frameOffsetA",
				DKFunction([this](DKVariant& v) {TransformToVariant(v, this->frameA); }),
				DKFunction([this](DKVariant& v) {VariantToTransform(v, this->frameA); }),
				DKFunction([](const DKVariant& v){return CheckTransformVariant(v); }),
				NULL);

			this->Bind(L"frameOffsetB",
				DKFunction([this](DKVariant& v) {TransformToVariant(v, this->frameB); }),
				DKFunction([this](DKVariant& v) {VariantToTransform(v, this->frameB); }),
				DKFunction([](const DKVariant& v){return CheckTransformVariant(v); }),
				NULL);

			this->Bind(L"swingSpan1",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->swingSpan1); }),
				DKFunction([this](DKVariant& v) {this->swingSpan1 = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"swingSpan2",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->swingSpan2); }),
				DKFunction([this](DKVariant& v) {this->swingSpan2 = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"twistSpan",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->twistSpan); }),
				DKFunction([this](DKVariant& v) {this->twistSpan = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"softness",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->softness); }),
				DKFunction([this](DKVariant& v) {this->softness = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"biasFactor",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->biasFactor); }),
				DKFunction([this](DKVariant& v) {this->biasFactor = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"relaxationFactor",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->relaxationFactor); }),
				DKFunction([this](DKVariant& v) {this->relaxationFactor = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"damping",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->damping); }),
				DKFunction([this](DKVariant& v) {this->damping = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->SetCallback(DKFunction([this](DKSerializer::State s)
			{
				if (s == StateSerializeBegin)
				{
					this->frameA = target->FrameA();
					this->frameB = target->FrameB();
					this->swingSpan1 = target->SwingSpan1();
					this->swingSpan2 = target->SwingSpan2();
					this->twistSpan = target->TwistSpan();
					this->softness = target->Softness();
					this->biasFactor = target->BiasFactor();
					this->relaxationFactor = target->RelaxationFactor();
					this->damping = target->Damping();
				}
				else if (s == StateDeserializeBegin)
				{
					this->frameA.Identity();
					this->frameB.Identity();
					this->swingSpan1 = BT_LARGE_FLOAT;
					this->swingSpan2 = BT_LARGE_FLOAT;
					this->twistSpan = BT_LARGE_FLOAT;
					this->softness = 1.0;
					this->biasFactor = 0.3;
					this->relaxationFactor = 1.0;
					this->damping = 0.01;
				}
				else if (s == StateDeserializeSucceed)
				{
					target->SetFrames(this->frameA, this->frameB);
					target->SetLimit(this->swingSpan1, this->swingSpan2, this->twistSpan, this->softness, this->biasFactor, this->relaxationFactor);
					target->SetDamping(this->damping);
				}
			}));

			return this;
		}

		DKNSTransform frameA;
		DKNSTransform frameB;
		float swingSpan1;
		float swingSpan2;
		float twistSpan;
		float softness;
		float biasFactor;
		float relaxationFactor;
		float damping;
		DKObject<DKConeTwistConstraint> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
