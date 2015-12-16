//
//  File: DKGeneric6DofConstraint.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKGeneric6DofConstraint.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		struct Generic6DofConstraintExt : public btGeneric6DofConstraint
		{
			using btGeneric6DofConstraint::m_flags;
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKGeneric6DofConstraint::DKGeneric6DofConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKNSTransform& ta, const DKNSTransform& tb)
: DKConstraint(LinkType::Generic6Dof, rbA, rbB, NewConstraintImpl<btGeneric6DofConstraint>::Create(rbA, rbB, ta, tb, true))
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofConstraint*>(this->impl));
}

DKGeneric6DofConstraint::DKGeneric6DofConstraint(DKRigidBody* rbB, const DKNSTransform& tb)
: DKGeneric6DofConstraint(NULL, rbB, DKNSTransform::identity, tb)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofConstraint*>(this->impl));
}

DKGeneric6DofConstraint::DKGeneric6DofConstraint(void)
: DKGeneric6DofConstraint(NULL, NULL, DKNSTransform::identity, DKNSTransform::identity)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofConstraint*>(this->impl));
}

DKGeneric6DofConstraint::DKGeneric6DofConstraint(LinkType type, DKRigidBody* rbA, DKRigidBody* rbB, class btGeneric6DofConstraint* ctxt)
: DKConstraint(type, rbA, rbB, ctxt)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofConstraint*>(this->impl));
}

DKGeneric6DofConstraint::~DKGeneric6DofConstraint(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofConstraint*>(this->impl));
}

void DKGeneric6DofConstraint::SetLimit(ParamAxis axis, float lower, float upper)
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		c->setLimit(0, lower, upper);	break;
	case ParamAxis::LinearY:		c->setLimit(1, lower, upper);	break;
	case ParamAxis::LinearZ:		c->setLimit(2, lower, upper);	break;
	case ParamAxis::AngularX:		c->setLimit(3, lower, upper);	break;
	case ParamAxis::AngularY:		c->setLimit(4, lower, upper);	break;
	case ParamAxis::AngularZ:		c->setLimit(5, lower, upper);	break;
	}
}

float DKGeneric6DofConstraint::LowerLimit(ParamAxis axis) const
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:	return c->getTranslationalLimitMotor()->m_lowerLimit[0];	break;
	case ParamAxis::LinearY:	return c->getTranslationalLimitMotor()->m_lowerLimit[1];	break;
	case ParamAxis::LinearZ:	return c->getTranslationalLimitMotor()->m_lowerLimit[2];	break;
	case ParamAxis::AngularX:	return c->getRotationalLimitMotor(0)->m_loLimit;			break;
	case ParamAxis::AngularY:	return c->getRotationalLimitMotor(1)->m_loLimit;			break;
	case ParamAxis::AngularZ:	return c->getRotationalLimitMotor(2)->m_loLimit;			break;
	}
	return 0.0;
}

float DKGeneric6DofConstraint::UpperLimit(ParamAxis axis) const
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:	return c->getTranslationalLimitMotor()->m_upperLimit[0];	break;
	case ParamAxis::LinearY:	return c->getTranslationalLimitMotor()->m_upperLimit[1];	break;
	case ParamAxis::LinearZ:	return c->getTranslationalLimitMotor()->m_upperLimit[2];	break;
	case ParamAxis::AngularX:	return c->getRotationalLimitMotor(0)->m_hiLimit;			break;
	case ParamAxis::AngularY:	return c->getRotationalLimitMotor(1)->m_hiLimit;			break;
	case ParamAxis::AngularZ:	return c->getRotationalLimitMotor(2)->m_hiLimit;			break;
	}
	return 0.0;
}

DKVector3 DKGeneric6DofConstraint::Axis(ParamAxis axis) const
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:	return BulletVector3(c->getAxis(0));	break;
	case ParamAxis::LinearY:	return BulletVector3(c->getAxis(1));	break;
	case ParamAxis::LinearZ:	return BulletVector3(c->getAxis(2));	break;
	case ParamAxis::AngularX:	return BulletVector3(c->getAxis(3));	break;
	case ParamAxis::AngularY:	return BulletVector3(c->getAxis(4));	break;
	case ParamAxis::AngularZ:	return BulletVector3(c->getAxis(5));	break;
	}
	return DKVector3(0, 0, 0);
}

float DKGeneric6DofConstraint::Angle(ParamAxis axis) const
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:	return c->getAngle(0);	break;
	case ParamAxis::LinearY:	return c->getAngle(1);	break;
	case ParamAxis::LinearZ:	return c->getAngle(2);	break;
	case ParamAxis::AngularX:	return c->getAngle(3);	break;
	case ParamAxis::AngularY:	return c->getAngle(4);	break;
	case ParamAxis::AngularZ:	return c->getAngle(5);	break;
	}
	return 0.0f;
}

float DKGeneric6DofConstraint::RelativePivotPosition(ParamAxis axis) const
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:	return c->getRelativePivotPosition(0);	break;
	case ParamAxis::LinearY:	return c->getRelativePivotPosition(1);	break;
	case ParamAxis::LinearZ:	return c->getRelativePivotPosition(2);	break;
	case ParamAxis::AngularX:	return c->getRelativePivotPosition(3);	break;
	case ParamAxis::AngularY:	return c->getRelativePivotPosition(4);	break;
	case ParamAxis::AngularZ:	return c->getRelativePivotPosition(5);	break;
	}
	return 0.0f;
}

void DKGeneric6DofConstraint::SetLinearLowerLimit(const DKVector3& v)
{
	static_cast<btGeneric6DofConstraint*>(this->impl)->setLinearLowerLimit(BulletVector3(v));
}

void DKGeneric6DofConstraint::SetLinearUpperLimit(const DKVector3& v)
{
	static_cast<btGeneric6DofConstraint*>(this->impl)->setLinearUpperLimit(BulletVector3(v));
}

void DKGeneric6DofConstraint::SetAngularLowerLimit(const DKVector3& v)
{
	static_cast<btGeneric6DofConstraint*>(this->impl)->setAngularLowerLimit(BulletVector3(v));
}

void DKGeneric6DofConstraint::SetAngularUpperLimit(const DKVector3& v)
{
	static_cast<btGeneric6DofConstraint*>(this->impl)->setAngularUpperLimit(BulletVector3(v));
}

DKVector3 DKGeneric6DofConstraint::LinearLowerLimit(void) const
{
	btVector3 v(0, 0, 0);
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	c->getLinearLowerLimit(v);
	return BulletVector3(v);
}

DKVector3 DKGeneric6DofConstraint::LinearUpperLimit(void) const
{
	btVector3 v(0, 0, 0);
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	c->getLinearUpperLimit(v);
	return BulletVector3(v);
}

DKVector3 DKGeneric6DofConstraint::AngularLowerLimit(void) const
{
	btVector3 v(0, 0, 0);
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	c->getAngularLowerLimit(v);
	return BulletVector3(v);
}

DKVector3 DKGeneric6DofConstraint::AngularUpperLimit(void) const
{
	btVector3 v(0, 0, 0);
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	c->getAngularUpperLimit(v);
	return BulletVector3(v);
}

DKGeneric6DofConstraint::AxisState DKGeneric6DofConstraint::State(ParamAxis axis) const
{
	float lower = this->LowerLimit(axis);
	float upper = this->UpperLimit(axis);

	if (lower == upper)
		return StateLocked;
	if (lower < upper)
		return StateRanged;
	return StateFree;
}

DKNSTransform DKGeneric6DofConstraint::FrameA(void) const
{
	return BulletTransform(static_cast<btGeneric6DofConstraint*>(this->impl)->getFrameOffsetA());
}

DKNSTransform DKGeneric6DofConstraint::FrameB(void) const
{
	return BulletTransform(static_cast<btGeneric6DofConstraint*>(this->impl)->getFrameOffsetB());
}

void DKGeneric6DofConstraint::SetFrameA(const DKNSTransform& t)
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	btTransform ta = BulletTransform(t);
	btTransform tb = c->getFrameOffsetB();
	c->setFrames(ta, tb);
}

void DKGeneric6DofConstraint::SetFrameB(const DKNSTransform& t)
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	btTransform ta = c->getFrameOffsetA();
	btTransform tb = BulletTransform(t);
	c->setFrames(ta, tb);
}

void DKGeneric6DofConstraint::SetFrames(const DKNSTransform& a, const DKNSTransform& b)
{
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	btTransform ta = BulletTransform(a);
	btTransform tb = BulletTransform(b);
	c->setFrames(ta, tb);
}

bool DKGeneric6DofConstraint::IsValidParam(ParamType type, ParamAxis axis) const
{
	switch (axis)
	{
	case ParamAxis::LinearX:
	case ParamAxis::LinearY:
	case ParamAxis::LinearZ:
	case ParamAxis::AngularX:
	case ParamAxis::AngularY:
	case ParamAxis::AngularZ:
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

bool DKGeneric6DofConstraint::HasParam(ParamType type, ParamAxis axis) const
{
	int bshift = 0;
	switch (axis)
	{
	case ParamAxis::LinearX:	bshift = 0;	break;
	case ParamAxis::LinearY:	bshift = BT_6DOF_FLAGS_AXIS_SHIFT;		break;
	case ParamAxis::LinearZ:	bshift = BT_6DOF_FLAGS_AXIS_SHIFT * 2;	break;
	case ParamAxis::AngularX:	bshift = BT_6DOF_FLAGS_AXIS_SHIFT * 3;	break;
	case ParamAxis::AngularY:	bshift = BT_6DOF_FLAGS_AXIS_SHIFT * 4;	break;
	case ParamAxis::AngularZ:	bshift = BT_6DOF_FLAGS_AXIS_SHIFT * 5;	break;
	default:
		return false;
	}

	int flags = static_cast<Generic6DofConstraintExt*>(this->impl)->m_flags;

	switch (type)
	{
	case ParamType::STOP_ERP:
		return flags & (BT_6DOF_FLAGS_ERP_STOP << bshift);
	case ParamType::CFM:
		return flags & (BT_6DOF_FLAGS_CFM_NORM << bshift);
	case ParamType::STOP_CFM:
		return flags & (BT_6DOF_FLAGS_CFM_STOP << bshift);
	}
	return false;
}

void DKGeneric6DofConstraint::ResetContext(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofConstraint*>(this->impl));
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);

	btTransform frameA = c->getFrameOffsetA();
	btTransform frameB = c->getFrameOffsetB();
	btVector3 linearLowerLimit(0, 0, 0);
	btVector3 linearUpperLimit(0, 0, 0);
	btVector3 angularLowerLimit(0, 0, 0);
	btVector3 angularUpperLimit(0, 0, 0);

	c->getLinearLowerLimit(linearLowerLimit);
	c->getLinearUpperLimit(linearUpperLimit);
	c->getAngularLowerLimit(angularLowerLimit);
	c->getAngularUpperLimit(angularUpperLimit);
		
	DKConstraint::ResetContext();

	c->setFrames(frameA, frameB);
	c->setLinearLowerLimit(linearLowerLimit);
	c->setLinearUpperLimit(linearUpperLimit);
	c->setAngularLowerLimit(angularLowerLimit);
	c->setAngularUpperLimit(angularUpperLimit);
}

void DKGeneric6DofConstraint::ResetContextImpl(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofConstraint*>(this->impl));
	btGeneric6DofConstraint* c = static_cast<btGeneric6DofConstraint*>(this->impl);
	c->~btGeneric6DofConstraint();
	NewConstraintImpl<btGeneric6DofConstraint>::Reset(c, bodyA, bodyB, DKNSTransform::identity, DKNSTransform::identity, true);
}

DKObject<DKModel> DKGeneric6DofConstraint::Clone(UUIDObjectMap& uuids) const
{
	DKObject<DKRigidBody> rbA = bodyA;
	DKObject<DKRigidBody> rbB = bodyB;
	return DKObject<DKGeneric6DofConstraint>::New(rbA, rbB, FrameA(), FrameB())->Copy(uuids, this);
}

DKGeneric6DofConstraint* DKGeneric6DofConstraint::Copy(UUIDObjectMap& uuids, const DKGeneric6DofConstraint* obj)
{
	if (DKConstraint::Copy(uuids, obj))
	{
		btGeneric6DofConstraint* dst = static_cast<btGeneric6DofConstraint*>(this->impl);
		btGeneric6DofConstraint* src = static_cast<btGeneric6DofConstraint*>(obj->impl);

		btTransform frameA = src->getFrameOffsetA();
		btTransform frameB = src->getFrameOffsetB();
		btVector3 linearLowerLimit(0, 0, 0);
		btVector3 linearUpperLimit(0, 0, 0);
		btVector3 angularLowerLimit(0, 0, 0);
		btVector3 angularUpperLimit(0, 0, 0);
		src->getLinearLowerLimit(linearLowerLimit);
		src->getLinearUpperLimit(linearUpperLimit);
		src->getAngularLowerLimit(angularLowerLimit);
		src->getAngularUpperLimit(angularUpperLimit);

		dst->setFrames(frameA, frameB);
		dst->setLinearLowerLimit(linearLowerLimit);
		dst->setLinearUpperLimit(linearUpperLimit);
		dst->setAngularLowerLimit(angularLowerLimit);
		dst->setAngularUpperLimit(angularUpperLimit);
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKGeneric6DofConstraint::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKSerializer* Init(DKGeneric6DofConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKGeneric6DofConstraint");
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
				DKFunction([this](DKVariant& v)	{v.SetVector3(this->linearLowerLimit); }),
				DKFunction([this](DKVariant& v)	{this->linearLowerLimit = v.Vector3(); }),
				DKFunction([this](const DKVariant& v)->bool	{return v.ValueType() == DKVariant::TypeVector3; }),
				dummyFunc);

			this->Bind(L"linearUpperLimit",
				DKFunction([this](DKVariant& v) {v.SetVector3(this->linearUpperLimit); }),
				DKFunction([this](DKVariant& v)	{this->linearUpperLimit = v.Vector3(); }),
				DKFunction([this](const DKVariant& v)->bool	{return v.ValueType() == DKVariant::TypeVector3; }),
				dummyFunc);

			this->Bind(L"angularLowerLimit",
				DKFunction([this](DKVariant& v)	{v.SetVector3(this->angularLowerLimit); }),
				DKFunction([this](DKVariant& v)	{this->angularLowerLimit = v.Vector3(); }),
				DKFunction([this](const DKVariant& v)->bool	{return v.ValueType() == DKVariant::TypeVector3; }),
				dummyFunc);

			this->Bind(L"angularUpperLimit",
				DKFunction([this](DKVariant& v) {v.SetVector3(this->angularUpperLimit); }),
				DKFunction([this](DKVariant& v)	{this->angularUpperLimit = v.Vector3(); }),
				DKFunction([this](const DKVariant& v)->bool	{return v.ValueType() == DKVariant::TypeVector3; }),
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
					this->linearLowerLimit = DKVector3(1, 1, 1);
					this->linearUpperLimit = DKVector3(-1, -1, -1);
					this->angularLowerLimit = DKVector3(1, 1, 1);
					this->angularUpperLimit = DKVector3(-1, -1, -1);
				}
				else if (s == StateDeserializeSucceed)
				{
					target->SetFrames(this->frameA, this->frameB);
					target->SetLinearLowerLimit(this->linearLowerLimit);
					target->SetLinearUpperLimit(this->linearUpperLimit);
					target->SetAngularLowerLimit(this->angularLowerLimit);
					target->SetAngularUpperLimit(this->angularUpperLimit);
				}
			}));

			return this;
		}

		DKNSTransform frameA;
		DKNSTransform frameB;
		DKVector3 linearLowerLimit;
		DKVector3 linearUpperLimit;
		DKVector3 angularLowerLimit;
		DKVector3 angularUpperLimit;
		DKObject<DKGeneric6DofConstraint> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
