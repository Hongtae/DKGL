//
//  File: DKObject.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypes.h"
#include "DKTypeTraits.h"
#include "DKAllocator.h"
#include "DKObjectRefCounter.h"

////////////////////////////////////////////////////////////////////////////////
// DKObject
// a simple smart pointer object.
// you can use this class without using ref-counted object.
// this class provide 'weak-reference' feature also.
//
// an object which allocated by DKAllocator is ref-counted automatically.
// you can use ref-counted state with your own class/struct which is not
// allocated by DKAllocator, you should provide your allocator in this case.
// even POD types are supported. if you don't provide your allocator,
// object becomes not-ref-counted state and you must delete manually.
//
// You can share ownership between multiple DKObject by assign or copy constructor.
// You can even share ownership with raw pointer or weak reference.
//
// Example:
//   DKObject<OBJECT> obj1 = DKOBJECT_NEW Object();  // create new instance.
//   DKObject<OBJECT> obj2 = obj2;          // share ownership with copy constructor.
//   OBJECT* raw_ptr = obj2;                // cast to raw-pointer.
//   DKObject<OBJECT> obj3 = raw_ptr;       // share ownership with raw-pointer.
//   DKObject<OBJECT>::Ref weak_ref = obj1; // cast to weak-reference.
//   DKObject<OBJECT> obj4 = weak_ref;      // share ownership with weak-reference.
//
//
// How to create object by using DKObject:
// 1. DKObject::New()
//     DKObject<OBJECT> p1 = DKObject<OBJECT>::New();
//     DKObject<OBJECT> p2 = DKObject<OBJECT>::New( arg1, arg2 ...);
//
// 2. DKOBJECT_NEW 매크로
//     DKObject<OBJECT> p1 = DKOBJECT_NEW OBJECT();
//     DKObject<OBJECT> p2 = DKOBJECT_NEW OBJECT( arg1, arg2, ...);
//
// 2. DKObject::Alloc()  - using custom allocator
//     DKObject<OBJECT> p1 = DKObject<OBJECT>::Alloc( myAllocator );
//     DKObject<OBJECT> p1 = DKObject<OBJECT>::Alloc( myAllocator, ...);
//
//
// About DKObject::New() parameters:
// 1. no-arguments (default constructor)
//     DKObject<OBJECT> p = DKObject<OBJECT>::New();
// 2. using copy constructor
//     DKObject<OBJECT> p = DKObject<OBJECT>::New(obj);
// 3. multiple arguments for constructor
//     DKObject<OBJECT> p = DKObject<OBJECT>::New(p1,p2,..);
// 4. using DKOBJECT_NEW macro
//	   DKObject<OBJECT> p = DKOBJECT_NEW OBJECT(p1,p2,..);
//
// Using weak-reference:
//     DKObject<OBJECT> p = DKObject<OBJECT>::New();
//     DKObject<OBJECT>::Ref ref = p; // get weak-ref from p
//     p = NULL; // destory p
//     DKObject<OBJECT> p2 = ref;  // p2 = NULL (ref invalidated)
//
// Note:
//   1. You cannot use DKObject<void>
//   2. if you have multiple-inheritanced class which does not polymorphic type,
//      then you will lost object if you cast your object to other types.
////////////////////////////////////////////////////////////////////////////////

#define DKOBJECT_NEW			new(DKFoundation::DKAllocator::DefaultAllocator())

namespace DKFoundation
{
	template <typename T> class DKObject
	{
	public:
		using TypeTraits = DKTypeTraits<T>;
		static_assert( TypeTraits::IsReference == 0, "Reference type cannot be used!");

		constexpr static bool IsPolymorphic() {return TypeTraits::IsPolymorphic();}
		
		using RefCounter = DKObjectRefCounter;
		class Ref
		{
		public:
			Ref(void) : ptr(NULL), refId(0) {}
			Ref(const Ref& r) : ptr(r.ptr), refId(r.refId) {}
			Ref& operator = (const Ref& ref)
			{
				ptr = ref.ptr;
				refId = ref.refId;
				return *this;
			}
		private:
			T* ptr;
			RefCounter::RefIdValue refId;
			friend class DKObject;
		};
		DKObject(T* p = NULL) : _target(_RetainObject(p))
		{
		}
		DKObject(DKObject&& obj) : _target(NULL)
		{
			_target = obj._target;
			obj._target = NULL;
		}
		DKObject(const DKObject& obj) : _target(_RetainObject(obj._target))
		{
		}
		DKObject(const Ref& ref) : _target(_RetainObject(ref))
		{
		}
		~DKObject(void)
		{
			_ReleaseObject(_target);
		}
		// pointer operators
		T* operator ->(void)						{return _target;}
		const T* operator ->(void) const			{return _target;}
		T& operator * (void)						{return *_target;}
		const T& operator * (void) const			{return *_target;}
		// type-casting operators
		operator T* (void)							{return _target;}
		operator const T* (void) const				{return _target;}
		// get raw-pointer
		T* Ptr(void)								{return _target;}
		const T* Ptr(void) const					{return _target;}

		template <typename R> constexpr static bool IsConvertible(void)
		{
			return DKTypeConversionTest<T, R>();
		}
		template <typename R> R* SafeCast(void)
		{
			if (IsConvertible<R>())					// up casting
				return static_cast<R*>(_target);
			return dynamic_cast<R*>(_target);		// else down casting
		}
		template <typename R> const R* SafeCast(void) const
		{
			if (IsConvertible<R>())						// up casting
				return static_cast<const R*>(_target);
			return dynamic_cast<const R*>(_target);		// else down casting
		}
		template <typename R> R* StaticCast(void)
		{
			return static_cast<R*>(_target);
		}
		template <typename R> const R* StaticCast(void) const
		{
			return static_cast<const R*>(_target);
		}
		template <typename R> R* ReinterpretCast(void)
		{
			return reinterpret_cast<R*>(_target);
		}
		template <typename R> const R* ReinterpretCast(void) const
		{
			return reinterpret_cast<const R*>(_target);
		}
		DKObject& operator = (DKObject&& obj)
		{
			if (_target != obj._target)
			{
				T* tmp = _target;
				_target = obj._target;
				obj._target = NULL;
				_ReleaseObject(tmp);
			}
			return *this;
		}
		DKObject& operator = (const DKObject& obj)
		{
			return operator = (obj._target);
		}
		DKObject& operator = (T* p)
		{
			if (_target != p)
			{
				T* old = _target;
				_target = _RetainObject(p);
				_ReleaseObject(old);
			}
			return *this;
		}
		DKObject& operator = (const Ref& ref)
		{
			if (_target != ref.ptr)
			{
				T* old = _target;
				_target = _RetainObject(ref);
				_ReleaseObject(old);
			}
			return *this;
		}
		// casting Ref (weak-ref)
		operator Ref (void) const
		{
			Ref ref;
			RefCounter::RefIdValue refId;
			if (_target && RefCounter::RefId(BaseAddress(_target), &refId))
			{
				ref.ptr = _target;
				ref.refId = refId;
			}
			return ref;
		}

		template <typename... Args> static DKObject Alloc(DKAllocator& alloc, Args&&... args)
		{
			return new(alloc) T(std::forward<Args>(args)...);
		}
		template <typename... Args> static DKObject New(Args&&... args)
		{
			return new(DKAllocator::DefaultAllocator(DKMemoryLocationHeap)) T(std::forward<Args>(args)...);
		}
		DKAllocator* Allocator(void) const
		{
			if (_target)
				return RefCounter::Allocator(BaseAddress(_target));
			return NULL;
		}
		bool IsManaged(void) const
		{
			if (_target && RefCounter::RefId(BaseAddress(_target), NULL))
				return true;
			return false;
		}
		bool IsShared(void) const
		{
			RefCounter::RefCountValue ref = 0;
			if (_target && RefCounter::RefCount(BaseAddress(_target), &ref))
				return ref > 1;
			return false;
		}
		RefCounter::RefCountValue SharingCount(void) const
		{
			RefCounter::RefCountValue ref = 0;
			if (_target && RefCounter::RefCount(BaseAddress(_target), &ref))
				return ref;
			return 0;
		}
		// polymorphic determine
		void* BaseAddress(void) const
		{
			return BaseAddress(_target);
		}
	private:
		static T* _RetainObject(const Ref& ref)
		{
			if (ref.ptr && RefCounter::IncrementRefCount(BaseAddress(ref.ptr), ref.refId))
				return ref.ptr;
			return NULL;
		}
		static T* _RetainObject(T* p)
		{
			if (p)
				RefCounter::IncrementRefCount(BaseAddress(p));
			return p;
		}
		static void _ReleaseObject(T* p)
		{
			if (p)
			{
				// decrease ref-count, delete if ref-count becomes zero.
				DKAllocator* allocator = NULL;
				if (RefCounter::DecrementRefCountAndUnsetIfZero(BaseAddress(p), &allocator))
				{
					if (allocator)
					{
						// get base-address before calling destructor!
						// we will lost polymorphic type info.
						void* ptr = BaseAddress(p);
						p->~T();
						allocator->Dealloc(ptr);
					}
				}
			}
		}
		static void* BaseAddress(T* p)
		{
			return DKTypeBaseAddressCast<T>(p);
		}
		T* _target;
	};
	template <typename T> class DKObject<T*>;
	template <typename T> class DKObject<T&>;
	template <typename T> class DKObject<T&&>;

	// To provide external linkage for internal object
	class DKUnknown
	{
	public:
		virtual ~DKUnknown(void) {}
	};
}
