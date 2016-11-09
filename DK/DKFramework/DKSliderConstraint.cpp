//
//  File: DKSliderConstraint.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKSliderConstraint.h"

namespace DKFramework
{
	namespace Private
	{
		struct SliderConstraintExt : public btSliderConstraint
		{
			using btSliderConstraint::m_flags;
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKSliderConstraint::DKSliderConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKNSTransform& ta, const DKNSTransform& tb)
: DKConstraint(LinkType::Slider, rbA, rbB, NewConstraintImpl<btSliderConstraint>::Create(rbA, rbB, ta, tb, true))
{
	DKASSERT_DEBUG(dynamic_cast<btSliderConstraint*>(this->impl));
}

DKSliderConstraint::DKSliderConstraint(DKRigidBody* rbB, const DKNSTransform& tb)
: DKSliderConstraint(NULL, rbB, DKNSTransform::identity, tb)
{
}

DKSliderConstraint::DKSliderConstraint(void)
: DKSliderConstraint(NULL, NULL, DKNSTransform::identity, DKNSTransform::identity)
{
}

DKSliderConstraint::~DKSliderConstraint(void)
{
	DKASSERT_DEBUG(dynamic_cast<btSliderConstraint*>(this->impl));
}

void DKSliderConstraint::SetLinearLimit(float lower, float upper)
{
	static_cast<btSliderConstraint*>(this->impl)->setLowerLinLimit(lower);
	static_cast<btSliderConstraint*>(this->impl)->setUpperLinLimit(upper);
}

void DKSliderConstraint::SetAngularLimit(float lower, float upper)
{
	static_cast<btSliderConstraint*>(this->impl)->setLowerAngLimit(lower);
	static_cast<btSliderConstraint*>(this->impl)->setUpperAngLimit(upper);
}

void DKSliderConstraint::SetLinearLowerLimit(float limit)
{
	static_cast<btSliderConstraint*>(this->impl)->setLowerLinLimit(limit);
}

void DKSliderConstraint::SetLinearUpperLimit(float limit)
{
	static_cast<btSliderConstraint*>(this->impl)->setUpperLinLimit(limit);
}

void DKSliderConstraint::SetAngularLowerLimit(float limit)
{
	static_cast<btSliderConstraint*>(this->impl)->setLowerAngLimit(limit);
}

void DKSliderConstraint::SetAngularUpperLimit(float limit)
{
	static_cast<btSliderConstraint*>(this->impl)->setUpperAngLimit(limit);
}

float DKSliderConstraint::LinearLowerLimit(void) const
{
	return static_cast<btSliderConstraint*>(this->impl)->getLowerLinLimit();
}

float DKSliderConstraint::LinearUpperLimit(void) const
{
	return static_cast<btSliderConstraint*>(this->impl)->getUpperLinLimit();
}

float DKSliderConstraint::AngularLowerLimit(void) const
{
	return static_cast<btSliderConstraint*>(this->impl)->getLowerAngLimit();
}

float DKSliderConstraint::AngularUpperLimit(void) const
{
	return static_cast<btSliderConstraint*>(this->impl)->getUpperAngLimit();
}

DKNSTransform DKSliderConstraint::FrameA(void) const
{
	return BulletTransform(static_cast<btSliderConstraint*>(this->impl)->getFrameOffsetA());
}

DKNSTransform DKSliderConstraint::FrameB(void) const
{
	return BulletTransform(static_cast<btSliderConstraint*>(this->impl)->getFrameOffsetB());
}

void DKSliderConstraint::SetFrameA(const DKNSTransform& t)
{
	btSliderConstraint* c = static_cast<btSliderConstraint*>(this->impl);
	btTransform ta = BulletTransform(t);
	btTransform tb = c->getFrameOffsetB();
	c->setFrames(ta, tb);
}

void DKSliderConstraint::SetFrameB(const DKNSTransform& t)
{
	btSliderConstraint* c = static_cast<btSliderConstraint*>(this->impl);
	btTransform ta = c->getFrameOffsetA();
	btTransform tb = BulletTransform(t);
	c->setFrames(ta, tb);
}

void DKSliderConstraint::SetFrames(const DKNSTransform& a, const DKNSTransform& b)
{
	btSliderConstraint* c = static_cast<btSliderConstraint*>(this->impl);
	btTransform ta = BulletTransform(a);
	btTransform tb = BulletTransform(b);
	c->setFrames(ta, tb);
}

bool DKSliderConstraint::IsValidParam(ParamType type, ParamAxis axis) const
{
	switch (type)
	{
	case ParamType::CFM:
		switch (axis)
		{
		case ParamAxis::Default:
		case ParamAxis::LinearX:
		case ParamAxis::AngularX:
			return true;
		}
		break;
	case ParamType::STOP_ERP:
	case ParamType::STOP_CFM:
		return true;
	}
	return false;
}

bool DKSliderConstraint::HasParam(ParamType type, ParamAxis axis) const
{
	int flags = static_cast<SliderConstraintExt*>(this->impl)->m_flags;

	switch (type)
	{
	case ParamType::STOP_ERP:
		switch (axis)
		{
		case ParamAxis::Default:
		case ParamAxis::LinearX:
			return flags & BT_SLIDER_FLAGS_ERP_LIMLIN;
		case ParamAxis::LinearY:
		case ParamAxis::LinearZ:
			return flags & BT_SLIDER_FLAGS_ERP_ORTLIN;
		case ParamAxis::AngularX:
			return flags & BT_SLIDER_FLAGS_ERP_LIMANG;
		case ParamAxis::AngularY:
		case ParamAxis::AngularZ:
			return flags & BT_SLIDER_FLAGS_ERP_ORTANG;
		}
		break;
	case ParamType::CFM:
		switch (axis)
		{
		case ParamAxis::Default:
		case ParamAxis::LinearX:
			return flags & BT_SLIDER_FLAGS_CFM_DIRLIN;
		case ParamAxis::AngularX:
			return flags & BT_SLIDER_FLAGS_CFM_DIRANG;
		}
		break;
	case ParamType::STOP_CFM:
		switch (axis)
		{
		case ParamAxis::Default:
		case ParamAxis::LinearX:
			return flags & BT_SLIDER_FLAGS_CFM_LIMLIN;
		case ParamAxis::LinearY:
		case ParamAxis::LinearZ:
			return flags & BT_SLIDER_FLAGS_CFM_ORTLIN;
		case ParamAxis::AngularX:
			return flags & BT_SLIDER_FLAGS_CFM_LIMANG;
		case ParamAxis::AngularY:
		case ParamAxis::AngularZ:
			return flags & BT_SLIDER_FLAGS_CFM_ORTANG;
		}
		break;
	}
	return false;
}

void DKSliderConstraint::ResetContext(void)
{
	DKASSERT_DEBUG(dynamic_cast<btSliderConstraint*>(this->impl));
	btSliderConstraint* c = static_cast<btSliderConstraint*>(this->impl);

	btTransform frameA = c->getFrameOffsetA();
	btTransform frameB = c->getFrameOffsetB();

	float linearLowerLimit = c->getLowerLinLimit();
	float linearUpperLimit = c->getUpperLinLimit();
	float angularLowerLimit = c->getLowerAngLimit();
	float angularUpperLimit = c->getUpperAngLimit();

	DKConstraint::ResetContext();

	c->setFrames(frameA, frameB);
	c->setLowerLinLimit(linearLowerLimit);
	c->setUpperLinLimit(linearUpperLimit);
	c->setLowerAngLimit(angularLowerLimit);
	c->setUpperAngLimit(angularUpperLimit);
}

void DKSliderConstraint::ResetContextImpl(void)
{
	DKASSERT_DEBUG(dynamic_cast<btSliderConstraint*>(this->impl));
	btSliderConstraint* c = static_cast<btSliderConstraint*>(this->impl);
	c->~btSliderConstraint();
	NewConstraintImpl<btSliderConstraint>::Reset(c, bodyA, bodyB, DKNSTransform::identity, DKNSTransform::identity, true);
}

DKObject<DKModel> DKSliderConstraint::Clone(UUIDObjectMap& uuids) const
{
	DKObject<DKRigidBody> rbA = bodyA;
	DKObject<DKRigidBody> rbB = bodyB;
	return DKObject<DKSliderConstraint>::New(rbA, rbB, FrameA(), FrameB())->Copy(uuids, this);
}

DKSliderConstraint* DKSliderConstraint::Copy(UUIDObjectMap& uuids, const DKSliderConstraint* obj)
{
	if (DKConstraint::Copy(uuids, obj))
	{
		btSliderConstraint* dst = static_cast<btSliderConstraint*>(this->impl);
		btSliderConstraint* src = static_cast<btSliderConstraint*>(obj->impl);

		dst->setFrames(src->getFrameOffsetA(), src->getFrameOffsetB());
		dst->setLowerLinLimit(src->getLowerLinLimit());
		dst->setUpperLinLimit(src->getUpperLinLimit());
		dst->setLowerAngLimit(src->getLowerAngLimit());
		dst->setUpperAngLimit(src->getUpperAngLimit());
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKSliderConstraint::Serializer(void)
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
		DKSerializer* Init(DKSliderConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKSliderConstraint");
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

			this->Bind(L"linearLowerLimit",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->linearLowerLimit); }),
				DKFunction([this](DKVariant& v) {this->linearLowerLimit = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"linearUpperLimit",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->linearUpperLimit); }),
				DKFunction([this](DKVariant& v) {this->linearUpperLimit = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"angularLowerLimit",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->angularLowerLimit); }),
				DKFunction([this](DKVariant& v) {this->angularLowerLimit = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->Bind(L"angularUpperLimit",
				DKFunction([this](DKVariant& v) {v.SetFloat(this->angularUpperLimit); }),
				DKFunction([this](DKVariant& v) {this->angularUpperLimit = v.Float(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				dummyFunc);

			this->SetCallback(DKFunction([this](DKSerializer::State s)
			{
				if (s == StateSerializeBegin)
				{
					this->frameA = target->FrameA();
					this->frameB = target->FrameB();
					this->linearLowerLimit = target->LinearLowerLimit();
					this->linearUpperLimit = target->LinearUpperLimit();
					this->angularLowerLimit = target->AngularLowerLimit();
					this->angularUpperLimit = target->AngularUpperLimit();
				}
				else if (s == StateDeserializeBegin)
				{
					this->frameA.Identity();
					this->frameB.Identity();
					this->linearLowerLimit = 1.0;
					this->linearUpperLimit = -1.0;
					this->angularLowerLimit = 0.0;
					this->angularUpperLimit = 0.0;
				}
				else if (s == StateDeserializeSucceed)
				{
					target->SetFrames(this->frameA, this->frameB);
					target->SetLinearLimit(this->linearLowerLimit, this->linearUpperLimit);
					target->SetAngularLimit(this->angularLowerLimit, this->angularUpperLimit);
				}
			}));

			return this;
		}

		DKNSTransform frameA;
		DKNSTransform frameB;
		float linearLowerLimit;
		float linearUpperLimit;
		float angularLowerLimit;
		float angularUpperLimit;

		DKObject<DKSliderConstraint> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
