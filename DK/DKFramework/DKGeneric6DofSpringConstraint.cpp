//
//  File: DKGeneric6DofSpringConstraint.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKGeneric6DofSpringConstraint.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		struct Generic6DofSpringConstraintExt : public btGeneric6DofSpringConstraint
		{
			using btGeneric6DofSpringConstraint::m_springEnabled;
			using btGeneric6DofSpringConstraint::m_equilibriumPoint;
			using btGeneric6DofSpringConstraint::m_springStiffness;
			using btGeneric6DofSpringConstraint::m_springDamping;
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKGeneric6DofSpringConstraint::DKGeneric6DofSpringConstraint(DKRigidBody* rbA, DKRigidBody* rbB, const DKNSTransform& ta, const DKNSTransform& tb)
: DKGeneric6DofConstraint(LinkType::Generic6DofSpring, rbA, rbB, NewConstraintImpl<btGeneric6DofSpringConstraint>::Create(rbA, rbB, ta, tb, true))
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofSpringConstraint*>(this->impl));
}

DKGeneric6DofSpringConstraint::DKGeneric6DofSpringConstraint(DKRigidBody* rbB, const DKNSTransform& tb)
: DKGeneric6DofSpringConstraint(NULL, rbB, DKNSTransform::identity, tb)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofSpringConstraint*>(this->impl));
}

DKGeneric6DofSpringConstraint::DKGeneric6DofSpringConstraint(void)
: DKGeneric6DofSpringConstraint(NULL, NULL, DKNSTransform::identity, DKNSTransform::identity)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofSpringConstraint*>(this->impl));
}

DKGeneric6DofSpringConstraint::~DKGeneric6DofSpringConstraint(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofSpringConstraint*>(this->impl));
}

void DKGeneric6DofSpringConstraint::EnableSpring(ParamAxis axis, bool enable)
{
	btGeneric6DofSpringConstraint* c = static_cast<btGeneric6DofSpringConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		c->enableSpring(0, enable);		break;
	case ParamAxis::LinearY:		c->enableSpring(1, enable);		break;
	case ParamAxis::LinearZ:		c->enableSpring(2, enable);		break;
	case ParamAxis::AngularX:		c->enableSpring(3, enable);		break;
	case ParamAxis::AngularY:		c->enableSpring(4, enable);		break;
	case ParamAxis::AngularZ:		c->enableSpring(5, enable);		break;
	}
}

bool DKGeneric6DofSpringConstraint::IsSpringEnabled(ParamAxis axis) const
{
	Generic6DofSpringConstraintExt* c = static_cast<Generic6DofSpringConstraintExt*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		return c->m_springEnabled[0];		break;
	case ParamAxis::LinearY:		return c->m_springEnabled[1];		break;
	case ParamAxis::LinearZ:		return c->m_springEnabled[2];		break;
	case ParamAxis::AngularX:		return c->m_springEnabled[3];		break;
	case ParamAxis::AngularY:		return c->m_springEnabled[4];		break;
	case ParamAxis::AngularZ:		return c->m_springEnabled[5];		break;
	}
	return false;
}

void DKGeneric6DofSpringConstraint::SetStiffness(ParamAxis axis, float stiffness)
{
	btGeneric6DofSpringConstraint* c = static_cast<btGeneric6DofSpringConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		c->setStiffness(0, stiffness);		break;
	case ParamAxis::LinearY:		c->setStiffness(1, stiffness);		break;
	case ParamAxis::LinearZ:		c->setStiffness(2, stiffness);		break;
	case ParamAxis::AngularX:		c->setStiffness(3, stiffness);		break;
	case ParamAxis::AngularY:		c->setStiffness(4, stiffness);		break;
	case ParamAxis::AngularZ:		c->setStiffness(5, stiffness);		break;
	}
}

float DKGeneric6DofSpringConstraint::Stiffness(ParamAxis axis) const
{
	Generic6DofSpringConstraintExt* c = static_cast<Generic6DofSpringConstraintExt*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		return c->m_springStiffness[0];		break;
	case ParamAxis::LinearY:		return c->m_springStiffness[1];		break;
	case ParamAxis::LinearZ:		return c->m_springStiffness[2];		break;
	case ParamAxis::AngularX:		return c->m_springStiffness[3];		break;
	case ParamAxis::AngularY:		return c->m_springStiffness[4];		break;
	case ParamAxis::AngularZ:		return c->m_springStiffness[5];		break;
	}
	return 0.0f;
}

void DKGeneric6DofSpringConstraint::SetDamping(ParamAxis axis, float damping)
{
	btGeneric6DofSpringConstraint* c = static_cast<btGeneric6DofSpringConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		c->setDamping(0, damping);		break;
	case ParamAxis::LinearY:		c->setDamping(1, damping);		break;
	case ParamAxis::LinearZ:		c->setDamping(2, damping);		break;
	case ParamAxis::AngularX:		c->setDamping(3, damping);		break;
	case ParamAxis::AngularY:		c->setDamping(4, damping);		break;
	case ParamAxis::AngularZ:		c->setDamping(5, damping);		break;
	}
}

float DKGeneric6DofSpringConstraint::Damping(ParamAxis axis) const
{
	Generic6DofSpringConstraintExt* c = static_cast<Generic6DofSpringConstraintExt*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		return c->m_springDamping[0];		break;
	case ParamAxis::LinearY:		return c->m_springDamping[1];		break;
	case ParamAxis::LinearZ:		return c->m_springDamping[2];		break;
	case ParamAxis::AngularX:		return c->m_springDamping[3];		break;
	case ParamAxis::AngularY:		return c->m_springDamping[4];		break;
	case ParamAxis::AngularZ:		return c->m_springDamping[5];		break;
	}
	return 1.0f;
}

void DKGeneric6DofSpringConstraint::SetEquilibriumPoint(void)
{
	static_cast<btGeneric6DofSpringConstraint*>(this->impl)->setEquilibriumPoint();
}

void DKGeneric6DofSpringConstraint::SetEquilibriumPoint(ParamAxis axis)
{
	btGeneric6DofSpringConstraint* c = static_cast<btGeneric6DofSpringConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		c->setEquilibriumPoint(0);		break;
	case ParamAxis::LinearY:		c->setEquilibriumPoint(1);		break;
	case ParamAxis::LinearZ:		c->setEquilibriumPoint(2);		break;
	case ParamAxis::AngularX:		c->setEquilibriumPoint(3);		break;
	case ParamAxis::AngularY:		c->setEquilibriumPoint(4);		break;
	case ParamAxis::AngularZ:		c->setEquilibriumPoint(5);		break;
	}
}

void DKGeneric6DofSpringConstraint::SetEquilibriumPoint(ParamAxis axis, float val)
{
	btGeneric6DofSpringConstraint* c = static_cast<btGeneric6DofSpringConstraint*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		c->setEquilibriumPoint(0, val);		break;
	case ParamAxis::LinearY:		c->setEquilibriumPoint(1, val);		break;
	case ParamAxis::LinearZ:		c->setEquilibriumPoint(2, val);		break;
	case ParamAxis::AngularX:		c->setEquilibriumPoint(3, val);		break;
	case ParamAxis::AngularY:		c->setEquilibriumPoint(4, val);		break;
	case ParamAxis::AngularZ:		c->setEquilibriumPoint(5, val);		break;
	}
}

float DKGeneric6DofSpringConstraint::EquilibriumPoint(ParamAxis axis) const
{
	Generic6DofSpringConstraintExt* c = static_cast<Generic6DofSpringConstraintExt*>(this->impl);
	switch (axis)
	{
	case ParamAxis::LinearX:		return c->m_equilibriumPoint[0];		break;
	case ParamAxis::LinearY:		return c->m_equilibriumPoint[1];		break;
	case ParamAxis::LinearZ:		return c->m_equilibriumPoint[2];		break;
	case ParamAxis::AngularX:		return c->m_equilibriumPoint[3];		break;
	case ParamAxis::AngularY:		return c->m_equilibriumPoint[4];		break;
	case ParamAxis::AngularZ:		return c->m_equilibriumPoint[5];		break;
	}
	return 0.0f;
}

void DKGeneric6DofSpringConstraint::ResetContext(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofSpringConstraint*>(this->impl));
	btGeneric6DofSpringConstraint* c = static_cast<btGeneric6DofSpringConstraint*>(this->impl);

	bool springEnabled[6];
	float equilibriumPoint[6];
	float springStiffness[6];
	float springDamping[6];

	for (int i = 0; i < 6; ++i)
	{
		springEnabled[i] = static_cast<Generic6DofSpringConstraintExt*>(c)->m_springEnabled[i];
		springStiffness[i] = static_cast<Generic6DofSpringConstraintExt*>(c)->m_springStiffness[i];
		springDamping[i] = static_cast<Generic6DofSpringConstraintExt*>(c)->m_springDamping[i];
		equilibriumPoint[i] = static_cast<Generic6DofSpringConstraintExt*>(c)->m_equilibriumPoint[i];
	}

	DKGeneric6DofConstraint::ResetContext();

	for (int i = 0; i < 6; ++i)
	{
		static_cast<Generic6DofSpringConstraintExt*>(c)->enableSpring(i, springEnabled[i]);
		static_cast<Generic6DofSpringConstraintExt*>(c)->setStiffness(i, springStiffness[i]);
		static_cast<Generic6DofSpringConstraintExt*>(c)->setDamping(i, springDamping[i]);
		static_cast<Generic6DofSpringConstraintExt*>(c)->setEquilibriumPoint(i, equilibriumPoint[i]);
	}
}

void DKGeneric6DofSpringConstraint::ResetContextImpl(void)
{
	DKASSERT_DEBUG(dynamic_cast<btGeneric6DofSpringConstraint*>(this->impl));
	btGeneric6DofSpringConstraint* c = static_cast<btGeneric6DofSpringConstraint*>(this->impl);
	c->~btGeneric6DofSpringConstraint();
	NewConstraintImpl<btGeneric6DofSpringConstraint>::Reset(c, bodyA, bodyB, DKNSTransform::identity, DKNSTransform::identity, true);
}

DKObject<DKModel> DKGeneric6DofSpringConstraint::Clone(UUIDObjectMap& uuids) const
{
	DKObject<DKRigidBody> rbA = bodyA;
	DKObject<DKRigidBody> rbB = bodyB;
	return DKObject<DKGeneric6DofSpringConstraint>::New(rbA, rbB, FrameA(), FrameB())->Copy(uuids, this);
}

DKGeneric6DofSpringConstraint* DKGeneric6DofSpringConstraint::Copy(UUIDObjectMap& uuids, const DKGeneric6DofSpringConstraint* obj)
{
	if (DKGeneric6DofConstraint::Copy(uuids, obj))
	{
		Generic6DofSpringConstraintExt* dst = static_cast<Generic6DofSpringConstraintExt*>(this->impl);
		Generic6DofSpringConstraintExt* src = static_cast<Generic6DofSpringConstraintExt*>(obj->impl);

		for (int i = 0; i < 6; ++i)
		{
			dst->enableSpring(i, src->m_springEnabled[i]);
			dst->setStiffness(i, src->m_springStiffness[i]);
			dst->setDamping(i, src->m_springDamping[i]);
			dst->setEquilibriumPoint(i, src->m_equilibriumPoint[i]);
		}
		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKGeneric6DofSpringConstraint::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		void GetArrayValue(DKVariant& v, const bool* p, size_t count) const
		{
			v.SetValueType(DKVariant::TypeArray);
			for (size_t i = 0; i < count; ++i)
				v.Array().Add(DKVariant::VInteger(p[i]));
		}
		void GetArrayValue(DKVariant& v, const float* p, size_t count) const
		{
			v.SetValueType(DKVariant::TypeArray);
			for (size_t i = 0; i < count; ++i)
				v.Array().Add(DKVariant::VFloat(p[i]));
		}
		void SetArrayValue(const DKVariant& v, bool* p, size_t count) const
		{
			for (size_t i = 0; i < count; ++i)
				p[i] = v.Array().Value(i).Integer() != 0;
		}
		void SetArrayValue(const DKVariant& v, float* p, size_t count) const
		{
			for (size_t i = 0; i < count; ++i)
				p[i] = v.Array().Value(i).Float();
		}
		bool CheckArrayValueType(const DKVariant& v, DKVariant::Type type, size_t count) const
		{
			if (v.ValueType() == DKVariant::TypeArray && v.Array().Count() >= count)
			{
				for (size_t i = 0; i < count; ++i)
				{
					if (v.Array().Value(i).ValueType() != type)
						return false;
				}
				return true;
			}
			return false;
		}

		DKSerializer* Init(DKGeneric6DofSpringConstraint* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;
			this->SetResourceClass(L"DKGeneric6DofSpringConstraint");
			this->Bind(L"super", target->DKGeneric6DofConstraint::Serializer(), NULL);

			DKObject<DKOperation> dummyFunc = (DKOperation*)DKValue<int>(0);

			this->Bind(L"springEnabled",
				DKFunction([this](DKVariant& v) {GetArrayValue(v, this->springEnabled, 6); }),
				DKFunction([this](DKVariant& v) {SetArrayValue(v, this->springEnabled, 6); }),
				DKFunction([this](const DKVariant& v)->bool {return CheckArrayValueType(v, DKVariant::TypeInteger, 6); }),
				dummyFunc);

			this->Bind(L"equilibriumPoint",
				DKFunction([this](DKVariant& v) {GetArrayValue(v, this->equilibriumPoint, 6); }),
				DKFunction([this](DKVariant& v) {SetArrayValue(v, this->equilibriumPoint, 6); }),
				DKFunction([this](const DKVariant& v)->bool {return CheckArrayValueType(v, DKVariant::TypeFloat, 6); }),
				dummyFunc);

			this->Bind(L"springStiffness",
				DKFunction([this](DKVariant& v) {GetArrayValue(v, this->springStiffness, 6); }),
				DKFunction([this](DKVariant& v) {SetArrayValue(v, this->springStiffness, 6); }),
				DKFunction([this](const DKVariant& v)->bool {return CheckArrayValueType(v, DKVariant::TypeFloat, 6); }),
				dummyFunc);

			this->Bind(L"springDamping",
				DKFunction([this](DKVariant& v) {GetArrayValue(v, this->springDamping, 6); }),
				DKFunction([this](DKVariant& v) {SetArrayValue(v, this->springDamping, 6); }),
				DKFunction([this](const DKVariant& v)->bool {return CheckArrayValueType(v, DKVariant::TypeFloat, 6); }),
				dummyFunc);

			this->SetCallback(DKFunction([this](DKSerializer::State s)
			{
				if (s == StateSerializeBegin)
				{
					Generic6DofSpringConstraintExt* c = static_cast<Generic6DofSpringConstraintExt*>(target->impl);
					for (int i = 0; i < 6; ++i)
					{
						this->springEnabled[i] = c->m_springEnabled[i];
						this->springStiffness[i] = c->m_springStiffness[i];
						this->springDamping[i] = c->m_springDamping[i];
						this->equilibriumPoint[i] = c->m_equilibriumPoint[i];
					}
				}
				else if (s == StateDeserializeBegin)
				{
					for (int i = 0; i < 6; ++i)
					{
						this->springEnabled[i] = false;
						this->equilibriumPoint[i] = 0.0;
						this->springStiffness[i] = 0.0;
						this->springDamping[i] = 1.0;
					}
				}
				else if (s == StateDeserializeSucceed)
				{
					btGeneric6DofSpringConstraint* c = static_cast<btGeneric6DofSpringConstraint*>(target->impl);
					for (int i = 0; i < 6; ++i)
					{
						c->enableSpring(i, this->springEnabled[i]);
						c->setStiffness(i, this->springStiffness[i]);
						c->setDamping(i, this->springDamping[i]);
						c->setEquilibriumPoint(i, this->equilibriumPoint[i]);
					}
				}
			}));
			return this;
		}

		bool springEnabled[6];
		float equilibriumPoint[6];
		float springStiffness[6];
		float springDamping[6];
		DKObject<DKGeneric6DofSpringConstraint> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
