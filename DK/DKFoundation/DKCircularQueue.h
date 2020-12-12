//
//  File: DKCircularQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKMemory.h"
#include "DKFunction.h"
#include "DKArray.h"

namespace DKFoundation
{
	/**
	 @brief
	 circulate limited size of queue.
	 queue object never overflow.

	 items can be added at head, tail only
	 if item count have reached to limited size when adding new item,
	 the item at opposite side will be overwritten.

	 @code
	   DKCircularQueue<int> queue(3);  // set maximum length to 3
	   queue.Append(1);   // [1]
	   queue.Append(2);   // [1, 2]
	   queue.Append(3);   // [1, 2, 3]
	   queue.Append(4);   // [2, 3, 4] item(1) has been truncated. (append)
	   queue.Prepend(5);  // [5, 2, 3] item(4) has been truncated. (prepend)
	   queue.Prepend(6);  // [6, 5, 2] item(3) has been truncated. (prepend)
	 @endcode

	 */
	template <typename VALUE, typename ALLOC = DKMemoryDefaultAllocator>
	class DKCircularQueue
	{
	public:
		enum {MinimumCapacity = 2,};
		typedef DKArray<VALUE, ALLOC> Container;
		typedef typename Container::Index Index;
		typedef typename Container::Allocator Allocator;

		constexpr static size_t NodeSize()	{ return Allocator::NodeSize(); }

		explicit DKCircularQueue(size_t capacity_)
			: position(0)
			, capacity(Max(capacity_, size_t(MinimumCapacity)))
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

		template <typename T>
		DKCircularQueue(const DKCircularQueue<VALUE, T>& q)
			: position(0)
			, capacity(MinimumCapacity)
		{
			capacity = q.capacity;
			container.Reserve(capacity);
			for (size_t i = 0; i < q.container.Count(); ++i)
			{
				container.Add(q.Value(i));
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
				AppendT(v);
		}

		~DKCircularQueue()
		{
		}

		DKCircularQueue& operator = (DKCircularQueue&& q)
		{
			if (this != &q)
			{
				position = q.position;
				capacity = q.capacity;
				container = static_cast<Container&&>(q.container);
			}
			return *this;
		}

		template <typename T>
		DKCircularQueue& operator = (const DKCircularQueue<VALUE, T>& q)
		{
			if (this != &q)
			{
				container.~Container();
				new(&container) Container();

				capacity = q.capacity;
				container.Reserve(capacity);
				for (size_t i = 0; i < q.container.Count(); ++i)
				{
					container.Add(q.Value(i));
				}
				position = container.Count() % capacity;
			}
			return *this;
		}

		DKCircularQueue& operator = (std::initializer_list<VALUE> il)
		{
			container.Clear();
			position = 0;

			for (const VALUE& v : il)
				AppendT(v);
			DKASSERT_DEBUG(container.Count() <= capacity);
			return *this;
		}

		DKCircularQueue& operator += (std::initializer_list<VALUE> il)
		{
			for (const VALUE& v : il)
				AppendT(v);
			DKASSERT_DEBUG(container.Count() <= capacity);
			return *this;
		}

		bool IsEmpty() const
		{
			return container.IsEmpty();
		}

		size_t Capacity() const
		{
			return capacity;
		}

		size_t Count() const
		{
			return container.Count();
		}

		void Clear()
		{
			container.Clear();
			position = 0;
		}

		void Reset(size_t cap)
		{
			DKASSERT_DEBUG(cap >= MinimumCapacity);
			container.~Container();
			new(&container) Container();
			position = 0;
			capacity = Max<size_t>(cap, MinimumCapacity);
			container.Reserve(capacity);			
		}

		/// append item. if length exceed to limit, front item will be truncated.
		void Append(const VALUE& value)
		{
			AppendT(value);
			DKASSERT_DEBUG(container.Count() <= capacity);
		}
		void Append(VALUE&& value)
		{
			AppendT(static_cast<VALUE&&>(value));
			DKASSERT_DEBUG(container.Count() <= capacity);
		}

		/// prepend item, if length exceed to limit, last item will be truncated.
		void Prepend(const VALUE& value)
		{
			PrependT(value);
			DKASSERT_DEBUG(container.Count() <= capacity);
		}
		void Prepend(VALUE&& value)
		{
			PrependT(static_cast<VALUE&&>(value));
			DKASSERT_DEBUG(container.Count() <= capacity);
		}
		void RemoveFront()
		{
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

		void RemoveBack()
		{
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

		VALUE& Value(Index index)
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
		const VALUE& Value(Index index) const
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
		VALUE& Front()
		{
			return Value(0);
		}

		const VALUE& Front() const
		{
			return Value(0);
		}

		VALUE& Back()
		{
			return Value(container.Count() - 1);
		}

		const VALUE& Back() const
		{
			return Value(container.Count() - 1);
		}

		/// EnumerateForward / EnumerateBackward: enumerate all items.
		/// You cannot insert, remove items while enumerating. (container is read-only)
		/// enumerator can be lambda or any function type that can receive arguments (VALUE&) or (VALUE&, bool*)
		/// (VALUE&, bool*) type can cancel iteration by set boolean value to true.
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		/// lambda enumerator (const VALUE&) or (const VALUE&, bool*) function type.
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
	private:
		// lambda enumerator (VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>)
		{
			size_t count = container.Count();
			for (Index i = 0; i < count; ++i)
				enumerator(Value(i));
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>)
		{
			size_t count = container.Count();
			for (Index i = 1; i <= count; ++i)
				enumerator(Value(count - i));
		}
		// lambda enumerator (const VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>) const
		{
			size_t count = container.Count();
			for (Index i = 0; i < count; ++i)
				enumerator(Value(i));
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>) const
		{
			size_t count = container.Count();
			for (Index i = 1; i <= count; ++i)
				enumerator(Value(count - i));
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>)
		{
			bool stop = false;
			size_t count = container.Count();
			for (Index i = 0; i < count && !stop; ++i)
				enumerator(Value(i), &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>)
		{
			bool stop = false;
			size_t count = container.Count();
			for (Index i = 1; i <= count && !stop; ++i)
				enumerator(Value(count - i), &stop);
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>) const
		{
			bool stop = false;
			size_t count = container.Count();
			for (Index i = 0; i < count && !stop; ++i)
				enumerator(Value(i), &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>) const
		{
			bool stop = false;
			size_t count = container.Count();
			for (Index i = 1; i <= count && !stop; ++i)
				enumerator(Value(count - i), &stop);
		}

		template <typename T> void AppendT(T&& value)
		{
			size_t count = container.Count();
			DKASSERT_DEBUG(count <= capacity);
			if (count == capacity)
			{
				container.Value(position) = std::forward<T>(value);
				position = (position+1) % capacity;
			}
			else
			{
				container.Add(std::forward<T>(value));
				count = container.Count();
				position = (position+1) % capacity;
			}
			DKASSERT_DEBUG(container.Count() <= capacity);
		}
		template <typename T> void PrependT(T&& value)
		{
			size_t count = container.Count();
			DKASSERT_DEBUG(count <= capacity);
			if (count == capacity)
			{
				if (position > 0)
				position--;
				else
				position = count - 1;
				container.Value(position) = std::forward<T>(value);
			}
			else
			{
				container.Insert(std::forward<T>(value), 0);
				count = container.Count();
				position = (position+1) % capacity;
			}
			DKASSERT_DEBUG(container.Count() <= capacity);
		}

		Container container;
		size_t capacity;
		size_t position;
	};
}
