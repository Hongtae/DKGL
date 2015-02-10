//
//  File: DKHingeConstraint.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKHingeConstraint.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		struct HingeConstraintExt : public btHingeConstraint
		{
			using btHingeConstraint::m_limit;
			using btHingeConstraint::m_flags;
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKHingeConstraint::DKHingeConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKNSTransform& ta, const DKNSTransform& tb)
: DKConstraint(LinkType::Hinge, rbA, rbB, NewConstraintImpl<btHingeConstraint>::Create(rbA, rbB, ta, tb))
{
	DKASSERT_DEBUG(dynamic_cast<btHingeConstraint*>(this->impl));
}

DKHingeConstraint::DKHingeConstraint(DKRigidBody* rbA, const DKNSTransform& ta)
: DKConstraint(LinkType::Hinge, rbA, NULL, NewConstraintImpl<btHingeConstraint>::Create(rbA, ta))
{
	DKASSERT_DEBUG(dynamic_cast<btHingeConstraint*>(this->impl));
}

DKHingeConstraint::DKHingeConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKVector3& pivotInA, const DKVector3& pivotInB, const DKVector3& axisInA, const DKVector3& axisInB)
: DKConstraint(LinkType::Hinge, rbA, rbB, NewConstraintImpl<btHingeConstraint>::Create(rbA, rbB, pivotInA, pivotInB, axisInA, axisInB))
{
	DKASSERT_DEBUG(dynamic_cast<btHingeConstraint*>(this->impl));
}

DKHingeConstraint::DKHingeConstraint(DKRigidBody* rbA, const DKVector3& pivotInA, const DKVector3& axisInA)
: DKConstraint(LinkType::Hinge, rbA, NULL, NewConstraintImpl<btHingeConstraint>::Create(rbA, pivotInA, axisInA))
{
	DKASSERT_DEBUG(dynamic_cast<btHingeConstraint*>(this->impl));
}

DKHingeConstraint::DKHingeConstraint(void)
: DKConstraint(LinkType::Hinge, NULL, NULL, NewConstraintImpl<btHingeConstraint>::Create(nullptr, DKNSTransform::identity))
{
	DKASSERT_DEBUG(dynamic_cast<btHingeConstraint*>(this->impl));
}

DKHingeConstraint::~DKHingeConstraint(void)
{
	DKASSERT_DEBUG(dynamic_cast<btHingeConstraint*>(this->impl));
}

void DKHingeConstraint::SetLimit(float lower, float upper, float softness, float biasFactor, float relaxationFactor)
{
	static_cast<btHingeConstraint*>(this->impl)->setLimit(lower, upper, softness, biasFactor, relaxationFactor);
}

float DKHingeConstraint::LowerLimit(void) const
{
	return static_cast<btHingeConstraint*>(this->impl)->getLowerLimit();
}

float DKHingeConstraint::UpperLimit(void) const
{
	return static_cast<btHingeConstraint*>(this->impl)->getUpperLimit();
}

float DKHingeConstraint::Softness(void) const
{
	return static_cast<HingeConstraintExt*>(this->impl)->m_limit.getSoftness();
}

float DKHingeConstraint::BiasFactor(void) const
{
	return static_cast<HingeConstraintExt*>(this->impl)->m_limit.getBiasFactor();
}

float DKHingeConstraint::RelaxationFactor(void) const
{
	return static_cast<HingeConstraintExt*>(this->impl)->m_limit.getRelaxationFactor();
}

void DKHingeConstraint::SetAngularOnly(bool angularOnly)
{
	static_cast<btHingeConstraint*>(this->impl)->setAngularOnly(angularOnly);
}

bool DKHingeConstraint::IsAngularOnly(void) const
{
	return static_cast<btHingeConstraint*>(this->impl)->getAngularOnly();
}

void DKHingeConstraint::SetAngularMotor(bool enable, float targetVelocity, float maxMotorImpulse)
{
	static_cast<btHingeConstraint*>(this->impl)->enableAngularMotor(enable, targetVelocity, maxMotorImpulse);
}

void DKHingeConstraint::EnableAngularMotor(bool enable)
{
	static_cast<btHingeConstraint*>(this->impl)->enableMotor(enable);
}

bool DKHingeConstraint::IsAngularMotorEnabled(void) const
{
	return static_cast<btHingeConstraint*>(this->impl)->getEnableAngularMotor();
}

float DKHingeConstraint::MaxMotorImpulse(void) const
{
	return static_cast<btHingeConstraint*>(this->impl)->getMaxMotorImpulse();
}

float DKHingeConstraint::MotorTargetVelocity(void) const
{
	return static_cast<btHingeConstraint*>(this->impl)->getMotorTargetVelosity();
}

float DKHingeConstraint::HingeAngle(void) const
{
	return static_cast<btHingeConstraint*>(this->impl)->getHingeAngle();
}

float DKHingeConstraint::HingeAngle(const DKNSTransform& ta, const DKNSTransform& tb) const
{
	return static_cast<btHingeConstraint*>(this->impl)->getHingeAngle(
		BulletTransform(ta), BulletTransform(tb));
}

DKNSTransform DKHingeConstraint::FrameA(void) const
{
	return BulletTransform(static_cast<btHingeConstraint*>(this->impl)->getFrameOffsetA());
}

DKNSTransform DKHingeConstraint::FrameB(void) const
{
	return BulletTransform(static_cast<btHingeConstraint*>(this->impl)->getFrameOffsetB());
}

void DKHingeConstraint::SetFrameA(const DKNSTransform& t)
{
	btHingeConstraint* c = static_cast<btHingeConstraint*>(this->impl);
	btTransform ta = BulletTransform(t);
	btTransform tb = c->getFrameOffsetB();
	c->setFrames(ta, tb);
}

void DKHingeConstraint::SetFrameB(const DKNSTransform& t)
{
	btHingeConstraint* c = static_cast<btHingeConstraint*>(this->impl);
	btTransform ta = c->getFrameOffsetA();
	btTransform tb = BulletTransform(t);
	c->setFrames(ta, tb);
}

void DKHingeConstraint::SetFrames(const DKNSTransform& a, const DKNSTransform& b)
{
	btHingeConstraint* c = static_cast<btHingeConstraint*>(this->impl);
	btTransform ta = BulletTransform(a);
	btTransform tb = BulletTransform(b);
	c->setFrames(ta, tb);
}

bool DKHingeConstraint::IsValidParam(ParamType type, ParamAxis axis) const
{
	if (axis == ParamAxis::Default || axis == ParamAxis::AngularZ)
	{
		switch (type)
		{
		case ParamType::STOP_ERP:
		case ParamType::CFM:
		case ParamType::STOP_CFM:
			return true;
		}
	}
	return false;
}

bool DKHingeConstraint::HasParam(ParamType type, ParamAxis axis) const
{
	if (axis == ParamAxis::Default || axis == ParamAxis::AngularZ)
	{
		int flags = static_cast<HingeConstraintExt*>(this->impl)->m_flags;

		switch (type)
		{
		case ParamType::STOP_ERP:
			return (flags & BT_HINGE_FLAGS_ERP_STOP) != 0;
		case ParamType::CFM:
			return (flags & BT_HINGE_FLAGS_CFM_NORM) != 0;
		case ParamType::STOP_CFM:
			return (flags & BT_HINGE_FLAGS_CFM_STOP) != 0;
		}
	}
	return false;
}

void DKHingeConstraint::ResetContext(void)
{
	DKASSERT_DEBUG(dynamic_cast<btHingeConstraint*>(this->impl));
	btHingeConstraint* c = static_cast<btHingeConstraint*>(this->impl);

	btTransform frameA = c->getFrameOffsetA();
	btTransform frameB = c->getFrameOffsetB();
	float lowerLimit = c->getLowerLimit();
	float upperLimit = c->getUpperLimit();
	float softness = static_cast<HingeConstraintExt*>(c)->m_limit.getSoftness();
	float biasFactor = static_cast<HingeConstraintExt*>(c)->m_limit.getBiasFactor();
	float relaxationFactor = static_cast<HingeConstraintExt*>(c)->m_limit.getRelaxationFactor();
	float motorTargetVelocity = c->getMotorTargetVelosity();
	float maxMotorImpulse = c->getMaxMotorImpulse();
	bool angularOnly = c->getAngularOnly();
	bool angularMotorEnabled = c->getEnableAngularMotor();

	DKConstraint::ResetContext();

	c->setFrames(frameA, frameB);
	c->setLimit(lowerLimit, upperLimit, softness, biasFactor, relaxationFactor);
	c->setAngularOnly(angularOnly);
	c->enableAngularMotor(angularMotorEnabled, motorTargetVelocity, maxMotorImpulse);
}

void DKHingeConstraint::ResetContextImpl(void)
{
	DKASSERT_DEBUG(dynamic_cast<btHingeConstraint*>(this->impl));
	btHingeConstraint* c = static_cast<btHingeConstraint*>(this->impl);
	c->~btHingeConstraint();
	NewConstraintImpl<btHingeConstraint>::Reset(c, bodyA, bodyB, DKNSTransform::identity, DKNSTransform::identity);
}

DKObject<DKModel> DKHingeConstraint::Clone(UUIDObjectMap& uuids) const
{
	DKObject<DKRigidBody> rbA = bodyA;
	DKObject<DKRigidBody> rbB = bodyB;
	return DKObject<DKHingeConstraint>::New(rbA, rbB, FrameA(), FrameB())->Copy(uuids, this);
}

DKHingeConstraint* DKHingeConstraint::Copy(UUIDObjectMap& uuids, const DKHingeConstraint* obj)
{
	if (DKConstraint::Copy(uuids, obj))
	{
		HingeConstraintExt* dst = static_cast<HingeConstraintExt*>(this->impl);
		HingeConstraintExt* src = static_cast<HingeConstraintExt*>(obj->impl);

		dst->setFrames(src->getFrameOffsetA(), src->getFrameOffsetB());
		dst->setLimit(src->getLowerLimit(), src->getUpperLimit(), src->m_limit.getSoftness(), src->m_limit.getBiasFactor(), src->m_limit.getRelaxationFactor());
		dst->setAngularOnly(src->getAngularOnly());
		dst->enableAngularMotor(src->getEnableAngularMotor(), src->getMotorTargetVelosity(), src->getMaxMotorImpulse());
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKHingeConstraint::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		void GetTransform(DKVariant& v, const DKNSTransform& t) const
		{
			v.SetValueType(DKVariant::TypePairs);
			v.Pairs().Insert(L"orientation", (const DKVariant::VQuaternion&)t.orientation);
			v.Pairs().Insert(L"position", (const DKVariant::VVector3&)t.position);
		}
		void SetTransform(const DKVariant& v, DKNSTransform& t) const
		{
			t.orientation = v.Pairs().Find(L"orientation")->value.Quaternion();
			t.position = v.Pairs().Find(L"position")->value.Vector3();
		}
		bool CheckTransform(const DKVariant& v) const
		{
			if (v.ValueType() == DKVariant::TypePairs)
			{
				auto orientation = v.Pairs().Find(L"orientation");
				auto position = v.Pairs().Find(L"position");
				if (orientation && orientation->value.ValueType() == DKVariant::TypeQuaternion &&
					position && position->value.ValueType() == DKVariant::TypeVector3)
					return true;
			}
			return false;
		}
		DKSerializer* Init(DKHingeConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKHingeConstraint");
			this->Bind(L"super", target->DKConstraint::Serializer(), NULL);

			DKObject<DKOperation> dummyFunc = (DKOperation*)DKValue<int>(0);

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

			this->Bind(L"lowerLimit",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->lowerLimit); }),
				DKFunction([this](DKVariant& v) {this->lowerLimit = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"upperLimit",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->upperLimit); }),
				DKFunction([this](DKVariant& v) {this->upperLimit = v.Float(); }),
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

			this->Bind(L"motorTargetVelocity",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->motorTargetVelocity); }),
				DKFunction([this](DKVariant& v) {this->motorTargetVelocity = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"maxMotorImpulse",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->maxMotorImpulse); }),
				DKFunction([this](DKVariant& v) {this->maxMotorImpulse = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"angularOnly",
				DKFunction([this](DKVariant& v) {v.SetInteger(this->angularOnly); }),
				DKFunction([this](DKVariant& v) {this->angularOnly = v.Integer() != 0; }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeInteger; }),
				dummyFunc);

			this->Bind(L"angularMotorEnabled",
				DKFunction([this](DKVariant& v) {v.SetInteger(this->angularMotorEnabled); }),
				DKFunction([this](DKVariant& v) {this->angularMotorEnabled = v.Integer() != 0; }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeInteger; }),
				dummyFunc);

			this->SetCallback(DKFunction([this](DKSerializer::State s)
			{
				if (s == StateSerializeBegin)
				{
					this->frameA = target->FrameA();
					this->frameB = target->FrameB();
					this->lowerLimit = target->LowerLimit();
					this->upperLimit = target->UpperLimit();
					this->softness = target->Softness();
					this->biasFactor = target->BiasFactor();
					this->relaxationFactor = target->RelaxationFactor();
					this->angularOnly = target->IsAngularOnly();
					this->angularMotorEnabled = target->IsAngularMotorEnabled();
					this->motorTargetVelocity = target->MotorTargetVelocity();
					this->maxMotorImpulse = target->MaxMotorImpulse();
				}
				else if (s == StateDeserializeBegin)
				{
					this->frameA.Identity();
					this->frameB.Identity();
					this->lowerLimit = 1.0;
					this->upperLimit = -1.0;
					this->softness = 0.9;
					this->biasFactor = 0.3;
					this->relaxationFactor = 1.0;
					this->motorTargetVelocity = 0.0;
					this->maxMotorImpulse = 0.0;
					this->angularOnly = false;
					this->angularMotorEnabled = false;
				}
				else if (s == StateDeserializeSucceed)
				{
					target->SetFrames(this->frameA, this->frameB);
					target->SetLimit(this->lowerLimit, this->upperLimit, this->softness, this->biasFactor, this->relaxationFactor);
					target->SetAngularOnly(this->angularOnly);
					target->SetAngularMotor(this->angularMotorEnabled, this->motorTargetVelocity, this->maxMotorImpulse);
				}
			}));

			return this;
		}

		DKNSTransform frameA;
		DKNSTransform frameB;
		float lowerLimit;
		float upperLimit;
		float softness;
		float biasFactor;
		float relaxationFactor;

		float motorTargetVelocity;
		float maxMotorImpulse;

		bool angularOnly;
		bool angularMotorEnabled;
		DKObject<DKHingeConstraint> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
