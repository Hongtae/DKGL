//
//  File: DKObject.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypes.h"
#include "DKTypeTraits.h"
#include "DKAllocator.h"
#include "DKObjectRefCounter.h"

#define DKOBJECT_NEW			new(DKAllocator::DefaultAllocator())

namespace DKFoundation
{
	/**
	 @brief a simple smart pointer object.
	 you can use this class without using ref-counted object.
	 this class provide 'weak-reference' feature also.
	
	 an object which allocated by DKAllocator is ref-counted automatically.
	 you can use ref-counted state with your own class/struct which is not
	 allocated by DKAllocator, you should provide your allocator in this case.
	 even POD types are supported. if you don't provide your allocator,
	 object becomes not-ref-counted state and you must delete manually.
	
	 You can share ownership between multiple DKObject by assign or copy constructor.
	 You can even share ownership with raw pointer or weak reference.
	
	 Example:
	 @code
	   DKObject<OBJECT> obj1 = DKOBJECT_NEW Object();  // create new instance.
	   DKObject<OBJECT> obj2 = obj1;          // share ownership with copy constructor.
	   OBJECT* raw_ptr = obj2;                // cast to raw-pointer.
	   DKObject<OBJECT> obj3 = raw_ptr;       // share ownership with raw-pointer.
	   DKObject<OBJECT>::Ref weak_ref = obj1; // cast to weak-reference.
	   DKObject<OBJECT> obj4 = weak_ref;      // share ownership with weak-reference.
	 @endcode
	
	 How to create object by using DKObject:
	 @code
	 1. DKObject::New() function
	     DKObject<OBJECT> p1 = DKObject<OBJECT>::New();
	     DKObject<OBJECT> p2 = DKObject<OBJECT>::New( arg1, arg2 ...);
	
	 2. DKOBJECT_NEW macro
	     DKObject<OBJECT> p1 = DKOBJECT_NEW OBJECT();
	     DKObject<OBJECT> p2 = DKOBJECT_NEW OBJECT( arg1, arg2, ...);
	
	 2. DKObject::Alloc() with custom allocator
	     DKObject<OBJECT> p1 = DKObject<OBJECT>::Alloc( myAllocator );
	     DKObject<OBJECT> p1 = DKObject<OBJECT>::Alloc( myAllocator, ...);
	 @endcode
	
	 About DKObject::New() parameters:
	 @code
	 1. no-arguments (default constructor)
	     DKObject<OBJECT> p = DKObject<OBJECT>::New();
	 2. using copy constructor
	     DKObject<OBJECT> p = DKObject<OBJECT>::New(obj);
	 3. multiple arguments for constructor
	     DKObject<OBJECT> p = DKObject<OBJECT>::New(p1,p2,..);
	 4. using DKOBJECT_NEW macro
		   DKObject<OBJECT> p = DKOBJECT_NEW OBJECT(p1,p2,..);
	 @endcode
	
	 Using weak-reference:
	 @code
	     DKObject<OBJECT> p = DKObject<OBJECT>::New();
	     DKObject<OBJECT>::Ref ref = p; // get weak-ref from p
	     p = NULL; // destory p
	     DKObject<OBJECT> p2 = ref;  // p2 = NULL (ref invalidated)
	 @endcode
	
	 @note
	   1. You cannot use DKObject<void>
	   2. if you have multiple-inheritanced class which does not polymorphic type,
	      then you will lost object if you cast your object to other types.
	 */
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
			Ref() : base(nullptr), refId(0) {}
			Ref(const Ref& r) : base(r.base), refId(r.refId) {}
			Ref& operator = (const Ref& ref)
			{
                base = ref.base;
				refId = ref.refId;
				return *this;
			}
		private:
            void* base;
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
		~DKObject()
		{
			_ReleaseObject(_target);
		}
		// pointer operators
		T* operator ->()						{return _target;}
		const T* operator ->() const			{return _target;}
		T& operator * ()						{return *_target;}
		const T& operator * () const			{return *_target;}
		// type-casting operators
		operator T* ()							{return _target;}
		operator const T* () const				{return _target;}
		// get raw-pointer
		T* Ptr()								{return _target;}
		const T* Ptr() const					{return _target;}

		template <typename R> constexpr static bool IsConvertible()
		{
			return DKTypeConversionTest<T, R>();
		}
		template <typename R> R* SafeCast()
		{
			return Private::SafeCaster<T, R, IsConvertible<R>()>::Cast(_target);
		}
		template <typename R> const R* SafeCast() const
		{
			return Private::SafeCaster<T, R, IsConvertible<R>()>::Cast(_target);
		}
		template <typename R> R* StaticCast()
		{
			return static_cast<R*>(_target);
		}
		template <typename R> const R* StaticCast() const
		{
			return static_cast<const R*>(_target);
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
			if (BaseAddress(_target) != ref.base)
			{
				T* old = _target;
				_target = _RetainObject(ref);
				_ReleaseObject(old);
			}
			return *this;
		}
		/// casting Ref (weak-ref)
		operator Ref () const
		{
			Ref ref;
			RefCounter::RefIdValue refId;
            void* base = BaseAddress(_target);
			if (base && RefCounter::RefId(base, &refId))
			{
				ref.base = base;
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
			return new(DKAllocator::DefaultAllocator()) T(std::forward<Args>(args)...);
		}
		DKAllocator* Allocator() const
		{
			if (_target)
				return RefCounter::Allocator(BaseAddress(_target));
			return NULL;
		}
		bool IsManaged() const
		{
			if (_target && RefCounter::RefId(BaseAddress(_target), NULL))
				return true;
			return false;
		}
		bool IsShared() const
		{
			RefCounter::RefCountValue ref = 0;
			if (_target && RefCounter::RefCount(BaseAddress(_target), &ref))
				return ref > 1;
			return false;
		}
		RefCounter::RefCountValue SharingCount() const
		{
			RefCounter::RefCountValue ref = 0;
			if (_target && RefCounter::RefCount(BaseAddress(_target), &ref))
				return ref;
			return 0;
		}
		/// determine base address of polymorphic type.
		/// For an object have multiple inheritance, it returns starting address.
		void* BaseAddress() const
		{
			return BaseAddress(_target);
		}
	private:
		static T* _RetainObject(const Ref& ref)
		{
			if (ref.base && RefCounter::IncrementRefCount(ref.base, ref.refId))
				return static_cast<T*>(ref.base);
			return nullptr;
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
				void* addr = BaseAddress(p);
				if (RefCounter::DecrementRefCountAndUnsetIfZero(addr, &allocator))
				{
					p->~T();
					if (allocator)
					{
						allocator->Dealloc(addr);
					}
					else
					{
						// leak!
						DKASSERT_MEM_DESC_DEBUG(false, "Leak: Memory Allocator not found!");
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

	template <typename T> using DKWeakRef = typename DKObject<T>::Ref;

	/// To provide external linkage for internal object
	class DKUnknown
	{
	public:
		virtual ~DKUnknown() {}
	};
}
