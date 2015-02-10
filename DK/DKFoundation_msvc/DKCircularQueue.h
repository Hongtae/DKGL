//
//  File: DKCircularQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKDummyLock.h"
#include "DKCriticalSection.h"
#include "DKMemory.h"
#include "DKFunction.h"
#include "DKArray.h"

////////////////////////////////////////////////////////////////////////////////
// DKCircularQueue
// circulate limited size of queue.
// queue object never overflow.
//
// items can be added at head, tail only
// if item count have reached to limited size when adding new item,
// the item at opposite side will be overwritten.
//
// Example:
//   DKCircularQueue<int> queue(3);		// set maximum length to 3
//   queue.Append(1);		// [1]
//   queue.Append(2);		// [1, 2]
//   queue.Append(3);		// [1, 2, 3]
//   queue.Append(4);		// [2, 3, 4] item(1) has been truncated. (append)
//   queue.Prepend(5);		// [5, 2, 3] item(4) has been truncated. (prepend)
//   queue.Prepend(6);		// [6, 5, 2] item(3) has been truncated. (prepend)
//
// Note:
//  using CopyValue() to retrieve item for thread-safe.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename VALUE, typename LOCK = DKDummyLock, typename ALLOC = DKMemoryDefaultAllocator>
	class DKCircularQueue
	{
	public:
		enum {MinimumCapacity = 2,};
		typedef DKArray<VALUE, DKDummyLock, ALLOC> Container;
		typedef LOCK Lock;
		typedef DKCriticalSection<LOCK>	CriticalSection;
		typedef typename Container::Index Index;
		typedef typename Container::Allocator Allocator;

		explicit DKCircularQueue(size_t capacity_)
			: position(0)
			, capacity(Max<size_t>(capacity_, MinimumCapacity))
		{
			DKASSERT_DEBUG(capacity_ >= MinimumCapacity);
			container.Reserve(capacity);
		}

		DKCircularQueue(DKCircularQueue&& q)
		{
			position = q.position;
			capacity = q.capacity;
			container = static_cast<Container&&>(q.container);
		}

		template <typename L, typename A>
		DKCircularQueue(const DKCircularQueue<VALUE, L, A>& q)
			: position(0)
			, capacity(MinimumCapacity)
		{
			DKCriticalSection<L> guard(q.lock);
			capacity = q.capacity;
			container.Reserve(capacity);
			for (size_t i = 0; i < q.container.Count(); ++i)
			{
				container.Add(q.ValueNL(i));
			}
			position = container.Count() % capacity;
		}

		DKCircularQueue(size_t capacity_, std::initializer_list<VALUE> il)
			: position(0)
			, capacity(Max<size_t>(capacity_, MinimumCapacity))
		{
			DKASSERT_DEBUG(capacity_ >= MinimumCapacity);
			container.Reserve(capacity);
			for (const VALUE& v : il)
				AppendNL(v);
		}

		~DKCircularQueue(void)
		{
		}

		DKCircularQueue& operator = (DKCircularQueue&& q)
		{
			if (this != &q)
			{
				CriticalSection guard(lock);
				position = q.position;
				capacity = q.capacity;
				container = static_cast<Container&&>(q.container);
			}
			return *this;
		}

		template <typename L, typename A>
		DKCircularQueue& operator = (const DKCircularQueue<VALUE, L, A>& q)
		{
			if (this != &q)
			{
				DKCriticalSection<L> guard1(q.lock);
				CriticalSection guard2(lock);

				container.~Container();
				new(&container) Container();

				capacity = q.capacity;
				container.Reserve(capacity);
				for (size_t i = 0; i < q.container.Count(); ++i)
				{
					container.Add(q.ValueNL(i));
				}
				position = container.Count() % capacity;
			}
			return *this;
		}

		DKCircularQueue& operator = (std::initializer_list<VALUE> il)
		{
			CriticalSection guard(lock);
			container.Clear();
			position = 0;

			for (const VALUE& v : il)
				AppendNL(v);
			DKASSERT_DEBUG(container.Count() <= capacity);
			return *this;
		}

		DKCircularQueue& operator += (std::initializer_list<VALUE> il)
		{
			CriticalSection guard(lock);
			for (const VALUE& v : il)
				AppendNL(v);
			DKASSERT_DEBUG(container.Count() <= capacity);
			return *this;
		}

		bool IsEmpty(void) const
		{
			CriticalSection guard(lock);
			return container.IsEmpty();
		}

		size_t Capacity(void) const
		{
			CriticalSection guard(lock);
			return capacity;
		}

		size_t Count(void) const
		{
			CriticalSection guard(lock);
			return container.Count();
		}

		void Clear(void)
		{
			CriticalSection guard(lock);
			container.Clear();
			position = 0;
		}

		void Reset(size_t cap)
		{
			DKASSERT_DEBUG(cap >= MinimumCapacity);
			CriticalSection guard(lock);
			container.~Container();
			new(&container) Container();
			position = 0;
			capacity = Max<size_t>(cap, MinimumCapacity);
			container.Reserve(capacity);			
		}
		// append item. if length exceed to limit, front item will be truncated.
		void Append(const VALUE& value)
		{
			CriticalSection guard(lock);
			AppendNL(value);
			DKASSERT_DEBUG(container.Count() <= capacity);
		}
		// prepend item, if length exceed to limit, last item will be truncated.
		void Prepend(const VALUE& value)
		{
			CriticalSection guard(lock);
			PrependNL(value);
			DKASSERT_DEBUG(container.Count() <= capacity);
		}

		void RemoveFront(void)
		{
			CriticalSection guard(lock);
			size_t count = container.Count();
			DKASSERT_DEBUG(count <= capacity);
			if (count == capacity)
			{
				container.LeftRotate(position+1);
				container.Remove(count-1);
			}
			else if (count > 0)
			{
				DKASSERT_DEBUG(position > 0);
				container.Remove(0);
			}
			position = container.Count();		
			DKASSERT_DEBUG(container.Count() <= capacity);
		}

		void RemoveBack(void)
		{
			CriticalSection guard(lock);
			size_t count = container.Count();
			DKASSERT_DEBUG(count <= capacity);
			if (count == capacity)
			{
				container.LeftRotate(position);
				container.Remove(count-1);
			}
			else if (count > 0)
			{
				DKASSERT_DEBUG(position > 0);
				container.Remove(count-1);
			}
			position = container.Count();		
			DKASSERT_DEBUG(container.Count() <= capacity);
		}

		bool CopyValue(VALUE& value, Index index) const
		{
			CriticalSection guard(lock);
			size_t count = container.Count();
			if (count > index)
			{
				value = ValueNL(index);
				return true;
			}
			return false;
		}

		VALUE& Value(Index index)
		{
			CriticalSection guard(lock);
			return ValueNL(index);
		}

		const VALUE& Value(Index index) const
		{
			CriticalSection guard(lock);
			return ValueNL(index);
		}

		VALUE& Front(void)
		{
			return Value(0);
		}

		const VALUE& Front(void) const
		{
			return Value(0);
		}

		VALUE& Back(void)
		{
			CriticalSection guard(lock);
			return ValueNL(container.Count() - 1);
		}

		const VALUE& Back(void) const
		{
			CriticalSection guard(lock);
			return ValueNL(container.Count() - 1);
		}

		// EnumerateForward / EnumerateBackward: enumerate all items.
		// You cannot insert, remove items while enumerating. (container is read-only)
		// enumerator can be lambda or any function type that can receive arguments (VALUE&) or (VALUE&, bool*)
		// (VALUE&, bool*) type can cancel iteration by set boolean value to true.
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum { ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>::Result };
			enum { ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>::Result };
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum { ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>::Result };
			enum { ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>::Result };
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		// lambda enumerator (const VALUE&) or (const VALUE&, bool*) function type.
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum { ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>::Result };
			enum { ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>::Result };
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum { ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>::Result };
			enum { ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>::Result };
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
	private:
		// lambda enumerator (VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>)
		{
			CriticalSection guard(lock);
			size_t count = container.Count();
			for (Index i = 0; i < count; ++i)
				enumerator(ValueNL(i));
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>)
		{
			CriticalSection guard(lock);
			size_t count = container.Count();
			for (Index i = 1; i <= count; ++i)
				enumerator(ValueNL(count - i));
		}
		// lambda enumerator (const VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			size_t count = container.Count();
			for (Index i = 0; i < count; ++i)
				enumerator(ValueNL(i));
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			size_t count = container.Count();
			for (Index i = 1; i <= count; ++i)
				enumerator(ValueNL(count - i));
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>)
		{
			CriticalSection guard(lock);
			bool stop = false;
			size_t count = container.Count();
			for (Index i = 0; i < count && !stop; ++i)
				enumerator(ValueNL(i), &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>)
		{
			CriticalSection guard(lock);
			bool stop = false;
			size_t count = container.Count();
			for (Index i = 1; i <= count && !stop; ++i)
				enumerator(ValueNL(count - i), &stop);
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			bool stop = false;
			size_t count = container.Count();
			for (Index i = 0; i < count && !stop; ++i)
				enumerator(ValueNL(i), &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			bool stop = false;
			size_t count = container.Count();
			for (Index i = 1; i <= count && !stop; ++i)
				enumerator(ValueNL(count - i), &stop);
		}
		VALUE& ValueNL(Index index)
		{
			size_t count = container.Count();
			DKASSERT_DEBUG(index >= 0);
			DKASSERT_DEBUG(count > 0 && count <= capacity);

			if (count == capacity)
			{
				Index pos = (index + position) % capacity;
				return container.Value(pos);
			}
			return container.Value(index);
		}
		const VALUE& ValueNL(Index index) const
		{
			size_t count = container.Count();
			DKASSERT_DEBUG(index >= 0);
			DKASSERT_DEBUG(count > 0 && count <= capacity);

			if (count == capacity)
			{
				Index pos = (index + position) % capacity;
				return container.Value(pos);
			}
			return container.Value(index);
		}
		void AppendNL(const VALUE& value)
		{
			size_t count = container.Count();
			DKASSERT_DEBUG(count <= capacity);
			if (count == capacity)
			{
				container.Value(position) = value;
				position = (position+1) % capacity;
			}
			else
			{
				container.Add(value);
				count = container.Count();
				position = (position+1) % capacity;
			}
			DKASSERT_DEBUG(container.Count() <= capacity);
		}
		void PrependNL(const VALUE& value)
		{
			size_t count = container.Count();
			DKASSERT_DEBUG(count <= capacity);
			if (count == capacity)
			{
				if (position > 0)
				position--;
				else
				position = count - 1;
				container.Value(position) = value;
			}
			else
			{
				container.Insert(value, 0);
				count = container.Count();
				position = (position+1) % capacity;
			}
			DKASSERT_DEBUG(container.Count() <= capacity);
		}

		Container container;
		Lock lock;
		size_t capacity;
		size_t position;
	};
}
