//
//  File: DKOrderedArray.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKInclude.h"
#include "DKTypeTraits.h"
#include "DKDummyLock.h"
#include "DKCriticalSection.h"
#include "DKMemory.h"
#include "DKArray.h"

////////////////////////////////////////////////////////////////////////////////
// DKOrderedArray
// simple array class, items be ordered always.
// provides fast bisectional search operation.
//
// You must provide compare-function of your items
// You can use DKArraySortAscending, DKArraySortDescending for your items
// comparison function, if your item(VALUE) has comparison operators.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename VALUE, typename LOCK = DKDummyLock, typename ALLOC = DKMemoryDefaultAllocator>
	class DKOrderedArray
	{
	public:
		typedef LOCK						Lock;
		typedef ALLOC						Allocator;
		typedef DKArray<VALUE, DKDummyLock, Allocator>	Container;
		typedef typename Container::Index	Index;
		typedef DKCriticalSection<LOCK>		CriticalSection;
		typedef DKTypeTraits<VALUE>			ValueTraits;

		// OrderFunc, comparison function returns boolean,
		// for ascending array, return lhs < rhs,
		// for descending array, return lhs > rhs.
		typedef bool (*OrderFunc)(const VALUE& lhs, const VALUE& rhs);

		// EqualFunc, test items are equal. return true if both items are equal.
		typedef bool (*EqualFunc)(const VALUE& lhs, const VALUE& rhs);

		static const Index invalidIndex = (Index)-1;
		// lock is public. lock object from outside!
		// You can call type-casting operator and CountNoLock() only while object is locked.
		LOCK	lock;

		// iterator class for range-based for loop
		// Note:
		//  while iterating by range-based-iterator, object is not locked state.
		//  And you can retrieve item as const VALUE& type (READ-ONLY) while iterating.
		typedef DKArrayRBIterator<DKOrderedArray, const VALUE&>			RBIterator;
		typedef DKArrayRBIterator<const DKOrderedArray, const VALUE&>	ConstRBIterator;
		RBIterator begin(void)				{return RBIterator(*this, 0);}
		ConstRBIterator begin(void) const	{return ConstRBIterator(*this, 0);}
		RBIterator end(void)				{return RBIterator(*this, this->Count());}
		ConstRBIterator end(void) const		{return ConstRBIterator(*this, this->Count());}

		DKOrderedArray(OrderFunc func)
			: orderFunc(func)
		{
			DKASSERT_DEBUG(orderFunc);
		}
		DKOrderedArray(DKOrderedArray&& v)
			: orderFunc(v.orderFunc)
			, container(static_cast<Container&&>(v.container))
		{
			DKASSERT_DEBUG(orderFunc);
		}
		DKOrderedArray(const DKOrderedArray& v)
			: orderFunc(v.orderFunc)
		{
			DKASSERT_DEBUG(orderFunc);
			Insert(v);
		}
		DKOrderedArray(const DKOrderedArray& v, OrderFunc func)
			: orderFunc(func)
		{
			DKASSERT_DEBUG(orderFunc);
			Insert(v);
		}
		DKOrderedArray(const VALUE* v, size_t c, OrderFunc func)
			: orderFunc(func)
		{
			DKASSERT_DEBUG(orderFunc);
			Insert(v, c);
		}
		DKOrderedArray(const VALUE& v, size_t c, OrderFunc func)
			: orderFunc(func)
			, container(v, c)
		{
			DKASSERT_DEBUG(orderFunc);
		}
		DKOrderedArray(std::initializer_list<VALUE> il, OrderFunc func)
			: orderFunc(func)
			, container(il)
		{
			DKASSERT_DEBUG(orderFunc);
		}
		~DKOrderedArray(void)
		{
		}
		DKOrderedArray& operator = (DKOrderedArray&& v)
		{
			if (this != &v)
			{
				CriticalSection guard(lock);
				orderFunc = v.orderFunc;
				container = static_cast<Container&&>(v.container);
			}
			return *this;
		}
		DKOrderedArray& operator = (const DKOrderedArray& v)
		{
			if (this != &v)
			{
				CriticalSection guard(lock);
				orderFunc = v.orderFunc;
				container = v.container;
			}
			return *this;
		}
		bool IsEmpty(void) const
		{
			CriticalSection guard(lock);
			return container.IsEmpty();
		}
		template <typename T> size_t Insert(const DKOrderedArray<VALUE, T>& value)
		{
			typename DKOrderedArray<VALUE, T>::CriticalSection guard(value.lock);
			return Insert((const VALUE*)value, value.Count());
		}
		template <typename T> size_t Insert(const DKArray<VALUE, T>& value)
		{
			typename DKArray<VALUE, T>::CriticalSection guard(value.lock);
			return Insert((const VALUE*)value, value.Count());
		}
		size_t Insert(const VALUE& value)
		{
			CriticalSection guard(lock);
			return InsertContainer(value, 1);
		}
		size_t Insert(const VALUE& value, size_t s)
		{
			CriticalSection guard(lock);
			return InsertContainer(value, s);
		}
		size_t Insert(const VALUE* value, size_t s)
		{
			CriticalSection guard(lock);
			container.Reserve(container.Count() + s);
			for (size_t i = 0; i < s; i++)
				InsertContainer(value[i], 1);
			return container.Count();
		}
		size_t Insert(std::initializer_list<VALUE> il)
		{
			CriticalSection guard(lock);
			container.Reserve(container.Count() + il.size());
			for (const VALUE& v : il)
				InsertContainer(v, 1);
			return container.Count();
		}
		size_t Remove(Index pos)
		{
			CriticalSection guard(lock);
			return container.Remove(pos);
		}
		size_t Remove(Index pos, size_t c)
		{
			CriticalSection guard(lock);
			return container.Remove(pos, c);
		}
		void Clear(void)
		{
			CriticalSection guard(lock);
			container.Clear();
		}
		size_t Count(void) const
		{
			CriticalSection guard(lock);
			return container.Count();
		}
		// CountNoLock: call this function when object has been locked already.
		size_t CountNoLock(void) const
		{
			return container.Count();
		}
		void Reserve(size_t c)
		{
			CriticalSection guard(lock);
			container.Reserve(c);
		}
		const VALUE& Value(Index index) const
		{
			CriticalSection guard(lock);
			return container.Value(index);
		}
		// to access items directly, when object has been locked already.
		operator const VALUE* (void) const
		{
			return (const VALUE*)container;
		}
		// EnumerateForward / EnumerateBackward: enumerate all items.
		// You cannot insert, remove items while enumerating. (container is read-only)
		// enumerator can be lambda or any function type that can receive arguments (VALUE&) or (VALUE&, bool*)
		// (VALUE&, bool*) type can cancel iteration by set boolean value to true.
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			CriticalSection guard(lock);
			container.EnumerateForward(std::forward<T>(enumerator));
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			CriticalSection guard(lock);
			container.EnumerateBackward(std::forward<T>(enumerator));
		}
		// lambda enumerator (const VALUE&) or (const VALUE&, bool*) function type.
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			CriticalSection guard(lock);
			container.EnumerateForward(std::forward<T>(enumerator));
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			CriticalSection guard(lock);
			container.EnumerateBackward(std::forward<T>(enumerator));
		}
		Index Find(const VALUE& value, EqualFunc equalFunc = DKArrayCompareEqual<VALUE>) const
		{
			CriticalSection guard(lock);
			return FindNoLock(value, equalFunc);
		}
		Index FindNoLock(const VALUE& value, EqualFunc equalFunc = DKArrayCompareEqual<VALUE>) const
		{
			Index index = 0;
			size_t count = container.Count();
			while (count > 2)
			{
				Index middle = index + count /2;
				if (equalFunc(value, container.Value(middle)))
					return middle;
				if (orderFunc(value, container.Value(middle)))
				{
					count = middle - index;
				}
				else
				{
					Index right = middle+1;
					count -= right - index;
					index = right;
				}
			}
			for (size_t i = 0; i < count; ++i)
			{
				if (equalFunc(value, container.Value(index)))
					return index;
				++index;
			}
			return invalidIndex;
		}
		// Find approximal value. (returns Index of value found.)
		// if greater is true, the result will be index of value or least greater value.
		Index FindApprox(const VALUE& value, bool greater) const
		{
			CriticalSection guard(lock);
			return FindApproxNoLock(value, greater);
		}
		Index FindApproxNoLock(const VALUE& value, bool greater) const
		{
			Index begin = 0;
			size_t count = container.Count();

			while (count > 2)
			{
				Index middle = begin + count / 2;
				if (orderFunc(value, container.Value(middle))) // value < middle
				{
					count = middle - begin + 1;
				}
				else
				{
					count = begin + count - middle;
					begin = middle;
				}
			}

			if (greater)
			{
				for (size_t i = 0; i < count; ++i)
				{
					if (orderFunc(container.Value(begin), value) == false)
						return begin;
					++begin;
				}
			}
			else
			{
				while (count > 0)
				{
					count--;
					if (orderFunc(value, container.Value(begin+count)) == false)
						return begin+count;
				}
			}
			return invalidIndex;
		}
	private:
		// Insert one value 'VALUE', 'c' times. (value x c)
		size_t InsertContainer(const VALUE& value, size_t c)
		{
			Index index = 0;
			size_t count = container.Count();
			while (count > 2)
			{
				Index middle = index + count /2;
				if (orderFunc(value, container.Value(middle)))
				{
					count = middle - index;
				}
				else
				{
					Index right = middle+1;
					count -= right - index;
					index = right;
				}
			}
			for (size_t i = 0; i < count; i++)
			{
				if (orderFunc(value, container.Value(index)))
					break;
				index++;
			}
			container.Insert(value, c, index);
			return container.Count();
		}

		OrderFunc	orderFunc;
		Container	container;
	};
}