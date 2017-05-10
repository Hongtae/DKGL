//
//  File: DKPoint2PointConstraint.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletPhysics.h"
#include "DKPoint2PointConstraint.h"

using namespace DKFramework;
using namespace DKFramework::Private;

DKPoint2PointConstraint::DKPoint2PointConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKVector3& pivotInA, const DKVector3& pivotInB)
: DKConstraint(LinkType::Point2Point, rbA, rbB, NewConstraintImpl<btPoint2PointConstraint>::Create(rbA, rbB, pivotInA, pivotInB))
{
	DKASSERT_DEBUG(dynamic_cast<btPoint2PointConstraint*>(this->impl));
}

DKPoint2PointConstraint::DKPoint2PointConstraint(DKRigidBody* rbA, const DKVector3& pivotInA)
: DKPoint2PointConstraint(rbA, NULL, pivotInA, DKVector3::zero)
{
}

DKPoint2PointConstraint::DKPoint2PointConstraint(void)
: DKPoint2PointConstraint(NULL, NULL, DKVector3::zero, DKVector3::zero)
{
}

DKPoint2PointConstraint::~DKPoint2PointConstraint(void)
{
	DKASSERT_DEBUG(dynamic_cast<btPoint2PointConstraint*>(this->impl));
}

DKVector3 DKPoint2PointConstraint::PivotInA(void) const
{
	return BulletVector3(static_cast<btPoint2PointConstraint*>(this->impl)->getPivotInA());
}

DKVector3 DKPoint2PointConstraint::PivotInB(void) const
{
	return BulletVector3(static_cast<btPoint2PointConstraint*>(this->impl)->getPivotInB());
}

void DKPoint2PointConstraint::SetPivotInA(const DKVector3& v)
{
	static_cast<btPoint2PointConstraint*>(this->impl)->setPivotA(BulletVector3(v));
}

void DKPoint2PointConstraint::SetPivotInB(const DKVector3& v)
{
	static_cast<btPoint2PointConstraint*>(this->impl)->setPivotB(BulletVector3(v));
}

bool DKPoint2PointConstraint::IsValidParam(ParamType type, ParamAxis axis) const
{
	if (axis == ParamAxis::Default)
	{
		switch (type)
		{
		case ParamType::ERP:
		case ParamType::STOP_ERP:
		case ParamType::CFM:
		case ParamType::STOP_CFM:
			return true;
		}
	}
	return false;
}

bool DKPoint2PointConstraint::HasParam(ParamType type, ParamAxis axis) const
{
	if (axis == ParamAxis::Default)
	{
		int flags = static_cast<btPoint2PointConstraint*>(this->impl)->getFlags();

		switch (type)
		{
		case ParamType::ERP:
		case ParamType::STOP_ERP:
			return flags & BT_P2P_FLAGS_ERP;
		case ParamType::CFM:
		case ParamType::STOP_CFM:
			return flags & BT_P2P_FLAGS_CFM;
		}
	}
	return false;
}

void DKPoint2PointConstraint::ResetContext(void)
{
	DKASSERT_DEBUG(dynamic_cast<btPoint2PointConstraint*>(this->impl));
	btPoint2PointConstraint* c = static_cast<btPoint2PointConstraint*>(this->impl);

	btVector3 pivotA = c->getPivotInA();
	btVector3 pivotB = c->getPivotInB();

	DKConstraint::ResetContext();

	c->setPivotA(pivotA);
	c->setPivotB(pivotB);
}

void DKPoint2PointConstraint::ResetContextImpl(void)
{
	DKASSERT_DEBUG(dynamic_cast<btPoint2PointConstraint*>(this->impl));
	btPoint2PointConstraint* c = static_cast<btPoint2PointConstraint*>(this->impl);
	c->~btPoint2PointConstraint();
	NewConstraintImpl<btPoint2PointConstraint>::Reset(c, bodyA, bodyB, DKVector3::zero, DKVector3::zero);
}

DKObject<DKModel> DKPoint2PointConstraint::Clone(UUIDObjectMap& uuids) const
{
	DKObject<DKRigidBody> rbA = bodyA;
	DKObject<DKRigidBody> rbB = bodyB;
	return DKObject<DKPoint2PointConstraint>::New(rbA, rbB, PivotInA(), PivotInB())->Copy(uuids, this);
}

DKPoint2PointConstraint* DKPoint2PointConstraint::Copy(UUIDObjectMap& uuids, const DKPoint2PointConstraint* obj)
{
	if (DKConstraint::Copy(uuids, obj))
	{
		btPoint2PointConstraint* dst = static_cast<btPoint2PointConstraint*>(this->impl);
		btPoint2PointConstraint* src = static_cast<btPoint2PointConstraint*>(obj->impl);

		dst->setPivotA(src->getPivotInA());
		dst->setPivotB(src->getPivotInB());
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKPoint2PointConstraint::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKSerializer* Init(DKPoint2PointConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKPoint2PointConstraint");
			this->Bind(L"super", target->DKConstraint::Serializer(), NULL);

			this->Bind(L"pivotA",
				DKFunction([this](DKVariant& v) {v.SetVector3(this->pivotA); }),
				DKFunction([this](DKVariant& v) {this->pivotA = v.Vector3(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeVector3; }),
				NULL);

			this->Bind(L"pivotB",
				DKFunction([this](DKVariant& v) {v.SetVector3(this->pivotB); }),
				DKFunction([this](DKVariant& v) {this->pivotB = v.Vector3(); }),
				DKFunction([this](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeVector3; }),
				NULL);

			this->SetCallback(DKFunction([this](DKSerializer::State s)
			{
				if (s == StateSerializeBegin)
				{
					this->pivotA = target->PivotInA();
					this->pivotB = target->PivotInB();
				}
				else if (s == StateDeserializeBegin)
				{
					this->pivotA = DKVector3(0, 0, 0);
					this->pivotB = DKVector3(0, 0, 0);
				}
				else if (s == StateDeserializeSucceed)
				{
					target->SetPivotInA(this->pivotA);
					target->SetPivotInB(this->pivotB);
				}
			}));

			return this;
		}
		DKVector3 pivotA;
		DKVector3 pivotB;
		DKObject<DKPoint2PointConstraint> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
