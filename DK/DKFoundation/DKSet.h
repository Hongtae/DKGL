//
//  File: DKSet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include <initializer_list>
#include "../DKInclude.h"
#include "DKAVLTree.h"
#include "DKDummyLock.h"
#include "DKCriticalSection.h"
#include "DKTypeTraits.h"

////////////////////////////////////////////////////////////////////////////////
// DKSet
// a set container class. using AVLTree (see DKAVLTree.h) internally.
//
// VALUE: value type
// LOCK: thread-lock type
// COMPARE: value comparison function
//
// Note:
//   if two set objects has same VALUE but different LOCK, COMPARE,
//   Union(), Intersect() are available only.
////////////////////////////////////////////////////////////////////////////////


namespace DKFoundation
{
	// compare VALUE, VALUE
	template <typename VALUE> struct DKSetComparison
	{
		int operator () (const VALUE& lhs, const VALUE& rhs) const
		{
			if (lhs > rhs)
				return 1;
			else if (lhs < rhs)
				return -1;
			return 0;
		}
	};

	template <typename VALUE, typename LOCK = DKDummyLock, typename COMPARE = DKSetComparison<VALUE>, typename ALLOC = DKMemoryDefaultAllocator>
	class DKSet
	{
	public:
		typedef VALUE													Value;
		typedef LOCK													Lock;
		typedef COMPARE													Compare;
		typedef ALLOC													Allocator;
		typedef DKAVLTree<Value, Value, Compare, Compare, DKTreeValueCopy<VALUE>, Allocator>	Container;
		typedef DKCriticalSection<Lock>									CriticalSection;
		typedef DKTypeTraits<Value>										ValueTraits;

		// lock is public. allow object being locked manually.
		// ContainsNoLock(), CountNoLock() is available when object has been locked.
		Lock	lock;

		DKSet(void)
		{
		}
		DKSet(DKSet&& s)
			: container(static_cast<Container&&>(s.container))
		{
		}
		// copy constructor. same type of DKSet object are allowed only.
		// template constructor not works on MSVC
		DKSet(const DKSet& s)
		{
			CriticalSection guard(s.lock);
			container = s.container;
		}
		DKSet(const Value* v, size_t n)
		{
			for (size_t i = 0; i < n; ++i)
				container.Insert(v[i]);
		}
		DKSet(std::initializer_list<Value> il)
		{
			for (const Value& v : il)
				container.Insert(v);
		}
		~DKSet(void)
		{
		}
		void Insert(const Value& v)
		{
			CriticalSection guard(lock);
			container.Insert(v);
		}
		void Insert(const Value* v, size_t n)
		{
			CriticalSection guard(lock);
			for (size_t i = 0; i < n; ++i)
				container.Insert(v[i]);
		}
		void Insert(std::initializer_list<Value> il)
		{
			CriticalSection guard(lock);
			for (const Value& v : il)
				container.Insert(v);
		}
		template <typename L, typename C, typename A> DKSet& Union(const DKSet<VALUE,L,C,A>& s)
		{
			CriticalSection guard(lock);
			s.EnumerateForward([this](const VALUE& val) { container.Insert(val); });
			return *this;
		}
		template <typename L, typename C, typename A> DKSet& Intersect(const DKSet<VALUE,L,C,A>& s)
		{
			CriticalSection guard(lock);
			s.EnumerateForward([this](const VALUE& val) {this->container.Remove(val);});
			return *this;
		}
		void Remove(const Value& v)
		{
			CriticalSection guard(lock);
			container.Remove(v);
		}
		void Remove(std::initializer_list<Value> il)
		{
			CriticalSection guard(lock);
			container.Remove(il);
		}
		void Clear(void)
		{
			CriticalSection guard(lock);
			container.Clear();
		}
		bool Contains(const Value& v) const
		{
			CriticalSection guard(lock);
			return container.Find(v) != NULL;
		}
		bool ContainsNoLock(const Value& v) const
		{
			return container.Find(v) != NULL;
		}
		bool IsEmpty(void) const
		{
			CriticalSection guard(lock);
			return container.Count() == 0;
		}
		size_t Count(void) const
		{
			CriticalSection guard(lock);
			return container.Count();
		}
		size_t CountNoLock(void) const
		{
			return container.Count();
		}
		DKSet& operator = (DKSet&& s)
		{
			if (this != &s)
			{
				CriticalSection guard(lock);
				container = static_cast<Container&&>(s.container);
			}
			return *this;
		}
		DKSet& operator = (const DKSet& s)
		{
			if (this != &s)
			{
				CriticalSection guardOther(s.lock);
				CriticalSection guardSelf(lock);

				container = s.container;
			}
			return *this;
		}
		DKSet& operator = (std::initializer_list<Value> il)
		{
			CriticalSection guard(lock);
			container.Clear();
			for (const Value& v : il)
				container.Insert(v);
			return *this;
		}
		// lambda enumerator (const VALUE&) or (const VALUE&, bool*) are allowed.
		// enumerating objects are READ-ONLY. values cannot be modified.
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
	private:
		// lambda enumerator (const VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			container.EnumerateForward([&enumerator](const VALUE& val, bool*) {enumerator(val);});
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			container.EnumerateBackward([&enumerator](const VALUE& val, bool*) {enumerator(val);});
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			container.EnumerateForward(enumerator);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			container.EnumerateBackward(enumerator);
		}

		Container container;
	};
}

