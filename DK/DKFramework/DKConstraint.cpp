//
//  File: DKConstraint.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKConstraint.h"
#include "DKRigidBody.h"
#include "DKScene.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;


DKConstraint::DKConstraint(LinkType t, DKRigidBody* rbA, DKRigidBody* rbB, class btTypedConstraint* p)
: DKModel(TypeConstraint)
, type(t)
, impl(p)
, bodyA(rbA)
, bodyB(rbB)
, restoreInfo(NULL)
{
	DKASSERT_DEBUG(impl != NULL);
	impl->setUserConstraintPtr(this);
	impl->setBreakingImpulseThreshold(SIMD_INFINITY);
}

DKConstraint::~DKConstraint(void)
{
	DKASSERT_DEBUG(impl != NULL);
	DKASSERT_DEBUG(impl->getUserConstraintPtr() == this);
	delete impl;

	restoreInfo = NULL;
}

bool DKConstraint::CanAcceptObjectAsParent(DKModel* p) const
{
	// parent object (p) must not be a child of bodyA or bodyB.
	if (this->bodyA && p->IsDescendantOf(this->bodyA))
	{
		DKLog("Constraint(%ls) cannot accept object(%ls) as parent. It is descendant of constraint bodyA(%ls).",
			(const wchar_t*)this->Name(), (const wchar_t*)p->Name(), (const wchar_t*)bodyA->Name());
		return false;
	}
	if (this->bodyB && p->IsDescendantOf(this->bodyB))
	{
		DKLog("Constraint(%ls) cannot accept object(%ls) as parent. It is descendant of constraint bodyB(%ls).",
			(const wchar_t*)this->Name(), (const wchar_t*)p->Name(), (const wchar_t*)bodyB->Name());
		return false;
	}

	return DKModel::CanAcceptObjectAsParent(p);
}

void DKConstraint::SetParam(ParamType type, ParamAxis axis, float value)
{
	int param = 0;
	int paxis = -1;
	switch (type)
	{
	case ParamType::ERP:		param = BT_CONSTRAINT_ERP;		break;
	case ParamType::STOP_ERP:	param = BT_CONSTRAINT_STOP_ERP;	break;
	case ParamType::CFM:		param = BT_CONSTRAINT_CFM;		break;
	case ParamType::STOP_CFM:	param = BT_CONSTRAINT_STOP_CFM;	break;
	default:
		DKERROR_THROW_DEBUG("Uknown Param Type!");
		return;
		break;
	}
	switch (axis)
	{
	case ParamAxis::Default:	paxis = -1;	break;
	case ParamAxis::LinearX:	paxis = 0;	break;
	case ParamAxis::LinearY:	paxis = 1;	break;
	case ParamAxis::LinearZ:	paxis = 2;	break;
	case ParamAxis::AngularX:	paxis = 3;	break;
	case ParamAxis::AngularY:	paxis = 4;	break;
	case ParamAxis::AngularZ:	paxis = 5;	break;
	default:
		DKERROR_THROW_DEBUG("Uknown Param Axis!");
		return;
		break;
	}

	impl->setParam(param, value, paxis);
}

float DKConstraint::GetParam(ParamType type, ParamAxis axis)
{
	int param = 0;
	int paxis = -1;
	switch (type)
	{
	case ParamType::ERP:		param = BT_CONSTRAINT_ERP;		break;
	case ParamType::STOP_ERP:	param = BT_CONSTRAINT_STOP_ERP;	break;
	case ParamType::CFM:		param = BT_CONSTRAINT_CFM;		break;
	case ParamType::STOP_CFM:	param = BT_CONSTRAINT_STOP_CFM;	break;
	default:
		DKERROR_THROW_DEBUG("Uknown Param Type!");
		return 0.0f;
		break;
	}
	switch (axis)
	{
	case ParamAxis::Default:	paxis = -1;	break;
	case ParamAxis::LinearX:	paxis = 0;	break;
	case ParamAxis::LinearY:	paxis = 1;	break;
	case ParamAxis::LinearZ:	paxis = 2;	break;
	case ParamAxis::AngularX:	paxis = 3;	break;
	case ParamAxis::AngularY:	paxis = 4;	break;
	case ParamAxis::AngularZ:	paxis = 5;	break;
	default:
		DKERROR_THROW_DEBUG("Uknown Param Axis!");
		return 0.0f;
		break;
	}
	return impl->getParam(param, paxis);
}

void DKConstraint::SetBreakingImpulseThreshold(float threshold)
{
	impl->setBreakingImpulseThreshold(threshold);
}

float DKConstraint::BreakingImpulseThreshold(void) const
{
	return impl->getBreakingImpulseThreshold();
}

bool DKConstraint::IsEnabled(void) const
{
	return impl->isEnabled();
}

void DKConstraint::SetEnabled(bool e)
{
	impl->setEnabled(e);
}

bool DKConstraint::Retarget(DKRigidBody* a, DKRigidBody* b)
{
	restoreInfo = NULL;

	if (a != bodyA || b != bodyB)
	{
		// a, b must not parent object for this.
		// each references has ownership, object never be deleted.
		if (a && this->IsDescendantOf(a))
		{
			DKLog("DKConstraint(%ls) cannot accept RigidBody(%ls) as target body. It is ancestor of this",
				(const wchar_t*)this->Name(), (const wchar_t*)a->Name());
			return false;
		}
		if (b && this->IsDescendantOf(b))
		{
			DKLog("DKConstraint(%ls) cannot accept RigidBody(%ls) as target body. It is ancestor of this",
				(const wchar_t*)this->Name(), (const wchar_t*)b->Name());
			return false;
		}

		bodyA = a;
		bodyB = b;

		this->ResetObject();
	}
	return false;
}

void DKConstraint::ResetObject(void)
{
	struct ReloadConstraint : public DKOperation
	{
		DKConstraint* con;
		void Perform(void) const
		{
			con->ResetContext();
		}
	};
	ReloadConstraint rc;
	rc.con = this;
	// reset
	ReloadSceneContext(&rc);
}

void DKConstraint::ResetContext(void)
{
	bool enabled = impl->isEnabled();
	float breaking = impl->getBreakingImpulseThreshold();

	ResetContextImpl();

	impl->setEnabled(enabled);
	impl->setBreakingImpulseThreshold(breaking);
}

void DKConstraint::OnUpdateTreeReferences(NamedObjectMap&, UUIDObjectMap& uuids)
{
	if (restoreInfo)
		RestoreTargets(uuids);
}

void DKConstraint::UpdateCopiedReferenceUUIDs(UUIDObjectMap& uuids)
{
	RestoreTargets(uuids);
}

void DKConstraint::RestoreTargets(UUIDObjectMap& uuids)
{
	auto find = [&uuids](const DKUuid& uuid) -> DKRigidBody*
	{
		auto p = uuids.Find(uuid);
		if (p)
		{
			if (p->value->type == DKModel::TypeCollision)
			{
				DKASSERT_DEBUG(dynamic_cast<DKCollisionObject*>(p->value));
				if (static_cast<DKCollisionObject*>(p->value)->objectType == DKCollisionObject::RigidBody)
				{
					DKASSERT_DEBUG(dynamic_cast<DKRigidBody*>(p->value));
					return static_cast<DKRigidBody*>(p->value);
				}
			}
		}
		return NULL;
	};

	DKRigidBody* rbA = NULL;
	DKRigidBody* rbB = NULL;

	if (restoreInfo)		// restore
	{
		if (restoreInfo->bodyA.IsValid())
		{
			DKRigidBody* body = find(restoreInfo->bodyA);
			if (body)
				rbA = body;
		}
		if (restoreInfo->bodyB.IsValid())
		{
			DKRigidBody* body = find(restoreInfo->bodyB);
			if (body)
				rbB = body;
		}
	}
	else		// restore bodies with UUIDs
	{
		DKRigidBody* rbA = bodyA;
		DKRigidBody* rbB = bodyB;
		if (rbA)
		{
			DKRigidBody* body = find(rbA->UUID());
			if (body)
				rbA = body;
		}
		if (rbB)
		{
			DKRigidBody* body = find(rbB->UUID());
			if (body)
				rbB = body;
		}
	}
	if (rbA != bodyA || rbB != bodyB)
	{
		this->Retarget(rbA, rbB);

		if (rbA)
			DKLog("DKConstraint(%ls) bodyA(%ls) restored.(%ls)\n", (const wchar_t*)this->Name(), (const wchar_t*)rbA->Name(), (const wchar_t*)rbA->UUID().String());
		if (rbB)
			DKLog("DKConstraint(%ls) bodyB(%ls) restored.(%ls)\n", (const wchar_t*)this->Name(), (const wchar_t*)rbB->Name(), (const wchar_t*)rbB->UUID().String());
	}
}

DKConstraint* DKConstraint::Copy(UUIDObjectMap& uuids, const DKConstraint* obj)
{
	if (DKModel::Copy(uuids, obj))
	{
		this->restoreInfo = NULL;
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKConstraint::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKString LinkTypeToString(LinkType type) const
		{
			switch (type)
			{
			case LinkType::Custom:				return L"custom";
			case LinkType::Fixed:				return L"fixed";
			case LinkType::Point2Point:			return L"point2point";
			case LinkType::Hinge:				return L"hinge";
			case LinkType::ConeTwist:			return L"coneTwist";
			case LinkType::Generic6Dof:			return L"g6dof";
			case LinkType::Generic6DofSpring:	return L"g6spring";
			case LinkType::Slider:				return L"slider";
			case LinkType::Gear:				return L"gear";
			}
			return L"unknown";
		}
		bool StringToLinkType(const DKString& s, LinkType* p) const
		{
			if (s.CompareNoCase(LinkTypeToString(LinkType::Custom)) == 0)				{ *p = LinkType::Custom;				return true; }
			if (s.CompareNoCase(LinkTypeToString(LinkType::Fixed)) == 0)				{ *p = LinkType::Fixed;				return true; }
			if (s.CompareNoCase(LinkTypeToString(LinkType::Point2Point)) == 0)			{ *p = LinkType::Point2Point;		return true; }
			if (s.CompareNoCase(LinkTypeToString(LinkType::Hinge)) == 0)				{ *p = LinkType::Hinge;				return true; }
			if (s.CompareNoCase(LinkTypeToString(LinkType::ConeTwist)) == 0)			{ *p = LinkType::ConeTwist;			return true; }
			if (s.CompareNoCase(LinkTypeToString(LinkType::Generic6Dof)) == 0)			{ *p = LinkType::Generic6Dof;		return true; }
			if (s.CompareNoCase(LinkTypeToString(LinkType::Generic6DofSpring)) == 0)	{ *p = LinkType::Generic6DofSpring;	return true; }
			if (s.CompareNoCase(LinkTypeToString(LinkType::Slider)) == 0)				{ *p = LinkType::Slider;				return true; }
			if (s.CompareNoCase(LinkTypeToString(LinkType::Gear)) == 0)					{ *p = LinkType::Gear;				return true; }

			return false;
		}
		bool CheckLinkType(const DKVariant& v) const
		{
			LinkType t;
			return v.ValueType() == DKVariant::TypeString && StringToLinkType(v.String(), &t) && t == this->target->type;
		}

		DKSerializer* Init(DKConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKConstraint");
			this->Bind(L"super", target->DKModel::Serializer(), NULL);

			// linkType must be matched.
			this->Bind(L"linkType",
				DKFunction([this](DKVariant& v) {v.SetString(LinkTypeToString(this->target->type)); }),
				DKFunction([this](DKVariant& v) {}),
				DKFunction(this, &LocalSerializer::CheckLinkType),
				NULL);

			this->Bind(L"disableCollisionsBetweenLinkedBodies",
				DKFunction([p](DKVariant& v) {v.SetInteger(p->disableCollisionsBetweenLinkedBodies); }),
				DKFunction([p](DKVariant& v) {p->disableCollisionsBetweenLinkedBodies = v.Integer() != 0; }),
				DKFunction([](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeInteger; }),
				DKFunction([p] {p->disableCollisionsBetweenLinkedBodies = true; })->Invocation());

			this->Bind(L"breakingImpulseThreshold",
				DKFunction([p](DKVariant& v) {v.SetFloat(p->BreakingImpulseThreshold()); }),
				DKFunction([p](DKVariant& v) {p->SetBreakingImpulseThreshold(v.Float()); }),
				DKFunction([](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeFloat; }),
				DKFunction([p] {p->SetBreakingImpulseThreshold(SIMD_INFINITY); })->Invocation());

			this->Bind(L"bodyA",
				DKFunction([this](DKVariant& v) {v.SetString(this->bodyA.String()); }),
				DKFunction([this](DKVariant& v) {this->bodyA = v.String(); }),
				DKFunction([](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeString; }),
				DKFunction([this] {this->bodyA.SetZero(); })->Invocation());

			this->Bind(L"bodyB",
				DKFunction([this](DKVariant& v) {v.SetString(this->bodyB.String()); }),
				DKFunction([this](DKVariant& v) {this->bodyB = v.String(); }),
				DKFunction([](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeString; }),
				DKFunction([this] {this->bodyB.SetZero(); })->Invocation());

			this->SetCallback(DKFunction([this](DKSerializer::State s)
			{
				if (s == StateSerializeBegin)
				{
					this->bodyA.SetZero();
					this->bodyB.SetZero();

					if (target->bodyA)
						this->bodyA = target->bodyA->UUID();
					if (target->bodyB)
						this->bodyB = target->bodyB->UUID();					
				}
				else if (s == StateDeserializeBegin)
				{
					bodyA.SetZero();
					bodyB.SetZero();
				}
				else if (s == StateDeserializeSucceed)
				{
					// restoring target will be processed after all references has been restored.
					// save restore-info. (target will be restored later)
					if (bodyA.IsValid() || bodyB.IsValid())
					{
						DKObject<TargetRestoreInfo> restoreInfo = DKObject<TargetRestoreInfo>::New();
						restoreInfo->bodyA = this->bodyA;
						restoreInfo->bodyB = this->bodyB;

						target->restoreInfo = restoreInfo;
					}
					else
						target->restoreInfo = NULL;
				}
			}));
			return this;
		}
		DKObject<DKConstraint> target;
		DKUuid bodyA;
		DKUuid bodyB;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
