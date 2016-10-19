//
//  File: BulletUtils.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once

#include "../../../lib/Inc_BulletPhysics.h"
#include "../../DKFoundation.h"
#include "../DKVector3.h"
#include "../DKQuaternion.h"
#include "../DKMatrix3.h"
#include "../DKMatrix4.h"
#include "../DKTransform.h"
#include "../DKCollisionObject.h"
#include "../DKCollisionShape.h"
#include "../DKConstraint.h"
#include "../DKWorld.h"

////////////////////////////////////////////////////////////////////////////////
// BulletUtils.h
// Utilities for bridge functons between DK and BulletPhysics conversion.
////////////////////////////////////////////////////////////////////////////////

using namespace DKGL;

namespace DKGL
{
	class DKWorld::CollisionWorldContext
	{
	public:
		btCollisionConfiguration*	configuration;
		btCollisionDispatcher*		dispatcher;
		btBroadphaseInterface*		broadphase;
		btConstraintSolver*			solver;
		btCollisionWorld*			world;

		DKTimeTick					tick;
		DKTimeTick					internalTick;

		DKSpinLock					lock;
	};

	namespace Private
	{
		using CollisionWorldContext = DKWorld::CollisionWorldContext;

		template <typename BT> struct BulletObject : public BT
		{
			using BT::impl;
		};
		template <typename T> FORCEINLINE auto GetBulletObject(const T* p) -> decltype(BulletObject<T>::impl)
		{
			return static_cast<const BulletObject<T>*>(p)->impl;
		}

		////////////////////////////////////////////////////////////////////////////////
		// CollisionWorldContext
		FORCEINLINE CollisionWorldContext* BulletCollisionWorldContext(const DKWorld* obj)
		{
			struct SceneExt : public DKWorld
			{
				using DKWorld::context;
			};
			return static_cast<const SceneExt*>(obj)->context;
		}
		////////////////////////////////////////////////////////////////////////////////
		// btCollisionObject
		FORCEINLINE btCollisionObject* BulletCollisionObject(const DKCollisionObject* obj)
		{
			return static_cast<const BulletObject<DKCollisionObject>*>(obj)->impl;
		}
		////////////////////////////////////////////////////////////////////////////////
		// btCollisionShape
		FORCEINLINE btCollisionShape* BulletCollisionShape(const DKCollisionShape* obj)
		{
			return static_cast<const BulletObject<DKCollisionShape>*>(obj)->impl;
		}
		////////////////////////////////////////////////////////////////////////////////
		// btTypedConstraint
		FORCEINLINE btTypedConstraint* BulletTypedConstraint(const DKConstraint* obj)
		{
			//return GetBulletObject(obj);
			return static_cast<const BulletObject<DKConstraint>*>(obj)->impl;
		}
		// linear math type conversion
		FORCEINLINE btVector3 BulletVector3(const DKVector3& v)
		{
			return btVector3(v.x, v.y, v.z);
		}
		FORCEINLINE DKVector3 BulletVector3(const btVector3& v)
		{
			return DKVector3(v.x(), v.y(), v.z());
		}
		FORCEINLINE btMatrix3x3 BulletMatrix3(const DKMatrix3& m)
		{
			// transpose
			return btMatrix3x3(
				m.m[0][0], m.m[1][0], m.m[2][0],
				m.m[0][1], m.m[1][1], m.m[2][1],
				m.m[0][2], m.m[1][2], m.m[2][2]);
		}
		FORCEINLINE DKMatrix3 BulletMatrix3(const btMatrix3x3& m)
		{
			// transpose
			return DKMatrix3(
				m[0][0], m[1][0], m[2][0],
				m[0][1], m[1][1], m[2][1],
				m[0][2], m[1][2], m[2][2]);
		}
		FORCEINLINE btQuaternion BulletQuaternion(const DKQuaternion& q)
		{
			return btQuaternion(q.x, q.y, q.z, q.w);
		}
		FORCEINLINE DKQuaternion BulletQuaternion(const btQuaternion& q)
		{
			return DKQuaternion(q.x(), q.y(), q.z(), q.w());
		}
		FORCEINLINE btTransform BulletTransform(const DKMatrix3& m, const DKVector3& p)
		{
			return btTransform(BulletMatrix3(m), BulletVector3(p));
		}
		FORCEINLINE btTransform BulletTransform(const DKQuaternion& q, const DKVector3& p)
		{
			return btTransform(BulletQuaternion(q), BulletVector3(p));
		}
		FORCEINLINE btTransform BulletTransform(const DKNSTransform& t)
		{
			return btTransform(BulletQuaternion(t.orientation), BulletVector3(t.position));
		}
		FORCEINLINE DKNSTransform BulletTransform(const btTransform& t)
		{
			return DKNSTransform(BulletQuaternion(t.getRotation()), BulletVector3(t.getOrigin()));
		}

		// constraint creation template
		template <typename T> struct NewConstraintImpl
		{
			static btTransform CastArg(const DKNSTransform& t)	{ return BulletTransform(t); }
			static btQuaternion CastArg(const DKQuaternion& q)	{ return BulletQuaternion(q); }
			static btVector3 CastArg(const DKVector3& v) 		{ return BulletVector3(v); }
			static btMatrix3x3 CastArg(const DKMatrix3& m) 		{ return BulletMatrix3(m); }
			static bool CastArg(bool b)							{ return b; }
			static float CastArg(float f)						{ return f; }
			static btRigidBody& CastArg(DKRigidBody* rb)
			{
				if (rb)
				{
					btCollisionObject* co = BulletCollisionObject(rb);
					btRigidBody* body = btRigidBody::upcast(co);
					DKASSERT_DEBUG(body);
					return *body;
				}
				return btTypedConstraint::getFixedBody();
			}

			template <typename... Args> static T* Create(Args&&... args)
			{
				return new T(CastArg(args)...);
			}
			template <typename... Args> static T* Reset(T* ptr, Args&&... args)
			{
				return new (ptr)T(CastArg(args)...);
			}
		};
		// for serializer
		inline void TransformToVariant(DKVariant& v, const DKNSTransform& t)
		{
			v.SetValueType(DKVariant::TypePairs);
			v.Pairs().Insert(L"orientation", (const DKVariant::VQuaternion&)t.orientation);
			v.Pairs().Insert(L"position", (const DKVariant::VVector3&)t.position);
		}
		inline void VariantToTransform(const DKVariant& v, DKNSTransform& t)
		{
			t.orientation = v.Pairs().Find(L"orientation")->value.Quaternion();
			t.position = v.Pairs().Find(L"position")->value.Vector3();
		}
		inline bool CheckTransformVariant(const DKVariant& v)
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
	}
}
