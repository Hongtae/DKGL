//
//  File: DKQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include <initializer_list>
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKDummyLock.h"
#include "DKCriticalSection.h"
#include "DKMemory.h"
#include "DKFunction.h"

////////////////////////////////////////////////////////////////////////////////
// DKQueue
// queue class, items can be added or removed from both sides (head, tail).
//
// Note:
//   PushFront() with multiple items: returns first item's pointer. (temporal)
//   PushBack() with multiple items: returns last item's pointer. (temporal)
//   Do not store pointer address returned by above functions!
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename VALUE, typename LOCK = DKDummyLock, typename ALLOC = DKMemoryDefaultAllocator>
	class DKQueue
	{
		enum {InitialSize = 32,};
	public:
		typedef LOCK						Lock;
		typedef DKCriticalSection<Lock>		CriticalSection;
		typedef ALLOC						Allocator;

		// lock is public. enables object could be locked from outside.
		// casting to const VALUE*, CountNoLock(), are allowed when object has been locked.
		Lock	lock;

		DKQueue(void)
			: begin(0)
			, count(0)
			, maxSize(0)
			, data(NULL)
		{
		}
		DKQueue(const VALUE* v, size_t c)
			: begin(0)
			, count(0)
			, maxSize(0)
			, data(NULL)
		{
			PushBack(v, c);
		}
		DKQueue(const VALUE& v, size_t c)
			: begin(0)
			, count(0)
			, maxSize(0)
			, data(NULL)
		{
			PushBack(v, c);
		}
		DKQueue(DKQueue&& queue)
			: begin(0)
			, count(0)
			, maxSize(0)
			, data(NULL)
		{
			begin = queue.begin;
			count = queue.count;
			maxSize = queue.maxSize;
			data = queue.data;
			
			queue.begin = 0;
			queue.count = 0;
			queue.maxSize = 0;
			queue.data = NULL;
		}
		DKQueue(const DKQueue& queue)
			: begin(0)
			, count(0)
			, maxSize(0)
			, data(NULL)
		{
			PushBack(queue);
		}
		~DKQueue(void)
		{
			Clear();
			if (data)
				Allocator::Free(data);
		}
		void Clear(void)
		{
			CriticalSection guard(lock);
			if (data && count)
			{
				for (size_t i = begin; i < begin+count; i++)
				{
					data[i].~VALUE();
				}
			}
			count = 0;
			Balance();
		}
		VALUE* PushFront(const DKQueue& queue)
		{
			CriticalSection guard(queue.lock);
			return PushFront((const VALUE*)queue, queue.count);
		}
		VALUE* PushFront(const VALUE* values, size_t n)
		{
			CriticalSection guard(lock);
			ReserveFront(n);
			for (size_t i = 0; i < n; i++)
			{
				::new(&data[--begin]) VALUE(values[n - i - 1]);
				count++;
			}
			Balance();
			if (count > 0)
				return &data[begin];
			return NULL;
		}
		VALUE* PushFront(const VALUE& value, size_t n)
		{
			CriticalSection guard(lock);
			ReserveFront(n);
			for (size_t i = 0; i < n; i++)
			{
				::new(&data[--begin]) VALUE(value);
				count++;
			}
			Balance();
			if (count > 0)
				return &data[begin];
			return NULL;
		}
		VALUE* PushFront(const VALUE& value)
		{
			return PushFront(value, 1);
		}
		VALUE* PushFront(std::initializer_list<VALUE> il)
		{
			CriticalSection guard(lock);
			ReserveFront(il.size());

			for (const VALUE& v : il)
			{
				::new(&data[--begin]) VALUE(v);
				count++;
			}
			Balance();
			if (count > 0)
				return &data[begin];
			return NULL;
		}
		VALUE* PushBack(const DKQueue& queue)
		{
			CriticalSection	guard(queue.lock);
			return PushBack((const VALUE*)queue, queue.count);
		}
		VALUE* PushBack(const VALUE* values, size_t n)
		{
			CriticalSection guard(lock);
			ReserveBack(n);
			for (size_t i = 0; i < n; i++)
			{
				::new(&data[begin+count]) VALUE(values[i]);
				count++;
			}
			Balance();
			if (count > 0)
				return &data[begin+count-1];
			return NULL;
		}
		VALUE* PushBack(const VALUE& value, size_t n)
		{
			CriticalSection guard(lock);
			ReserveBack(n);
			for (size_t i = 0; i < n; i++)
			{
				::new(&data[begin+count]) VALUE(value);
				count++;
			}
			Balance();
			if (count > 0)
				return &data[begin+count-1];
			return NULL;
		}
		VALUE* PushBack(const VALUE& value)
		{
			return PushBack(value, 1);
		}
		VALUE* PushBack(std::initializer_list<VALUE> il)
		{
			CriticalSection guard(lock);
			ReserveBack(il.size());
			for (const VALUE& v : il)
			{
				::new(&data[begin+count]) VALUE(v);
				count++;
			}
			Balance();
			if (count > 0)
				return &data[begin+count-1];
			return NULL;
		}
		bool IsEmpty(void) const
		{
			CriticalSection guard(lock);
			return count == 0;
		}
		bool PopFront(VALUE& ret)
		{
			CriticalSection guard(lock);

			if (count > 0)
			{
				ret = data[begin];
				data[begin].~VALUE();
				begin++;
				count--;
				Balance();
				return true;
			}
			return false;
		}
		VALUE PopFront(void)
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > 0);	// Error! queue is empty!

			VALUE ret = data[begin];
			data[begin].~VALUE();
			begin++;
			count--;
			Balance();
			return ret;
		}
		bool PopBack(VALUE& ret)
		{
			CriticalSection guard(lock);

			if (count > 0)
			{
				ret = data[begin+count-1];
				data[begin+count-1].~VALUE();
				count--;
				Balance();
				return true;
			}
			return false;
		}
		VALUE PopBack(void)
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > 0);	// Error! queue is empty!

			VALUE ret = data[begin+count-1];
			data[begin+count-1].~VALUE();
			count--;
			Balance();
			return ret;
		}
		VALUE& Front(void)
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > 0);	// Error! queue is empty!
			return data[begin];
		}
		const VALUE& Front(void) const
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > 0);	// Error! queue is empty!
			return data[begin];
		}
		bool Front(VALUE& ret) const
		{
			CriticalSection guard(lock);			
			if (count > 0)
			{
				ret = data[begin];
				return true;
			}
			return false;
		}
		VALUE& Back(void)
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > 0);	// Error! queue is empty!
			return data[begin+count-1];
		}
		const VALUE& Back(void) const
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > 0);	// Error! queue is empty!
			return data[begin+count-1];
		}
		bool Back(VALUE& ret) const
		{
			CriticalSection guard(lock);			
			if (count > 0)
			{
				ret = data[begin+count-1];
				return true;
			}
			return false;
		}
		// copy value. thread-safe.
		bool CopyValue(VALUE& value, unsigned long index) const
		{
			CriticalSection guard(lock);
			if (count > index)
			{
				value = data[begin+index];
				return true;
			}
			return false;
		}
		// since Value() returns reference,
		// be aware of items modifications by other threads.
		// Use CopyValue() for multi-threaded.
		VALUE& Value(unsigned long index)
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > index);
			return data[begin+index];
		}
		const VALUE& Value(unsigned long index) const
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > index);
			return data[begin+index];
		}
		// type-casting, object should be locked before calling this operator.
		operator VALUE* (void)
		{
			if (count > 0)
				return &data[begin];
			return NULL;
		}
		// type-casting, object should be locked before calling this operator.
		operator const VALUE* (void) const
		{
			if (count > 0)
				return &data[begin];
			return NULL;
		}
		size_t Count(void) const
		{
			CriticalSection guard(lock);
			return count;
		}
		// CountNoLock: counting objects without locking.
		// Usable when objects has been locked.
		size_t CountNoLock(void) const
		{
			return count;
		}
		DKQueue& operator = (DKQueue&& queue)
		{
			if (this != &queue)
			{
				CriticalSection guard(lock);
				CriticalSection guard2(queue.lock);
				
				if (data && count)
				{
					for (size_t i = begin; i < begin+count; i++)
					{
						data[i].~VALUE();
					}
				}
				if (data)
					Allocator::Free(data);

				begin = queue.begin;
				count = queue.count;
				maxSize = queue.maxSize;
				data = queue.data;

				queue.begin = 0;
				queue.count = 0;
				queue.maxSize = 0;
				queue.data = NULL;
			}
			return *this;
		}
		DKQueue& operator = (const DKQueue& queue)
		{
			if (this != &queue)
			{
				Clear();
				PushBack(queue);
			}
			return *this;
		}
		DKQueue& operator = (std::initializer_list<VALUE> il)
		{
			Clear();
			PushBack(il);
			return *this;
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

			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");

			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		// lambda enumerator (const VALUE&) or (const VALUE&, bool*) function type.
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
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>)
		{
			CriticalSection guard(lock);
			for (size_t i = 0; i < count; ++i)
				enumerator(data[begin+i]);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>)
		{
			CriticalSection guard(lock);
			for (size_t i = 1; i <= count; ++i)
				enumerator(data[begin+count-i]);
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			for (size_t i = 0; i < count; ++i)
				enumerator(data[begin+i]);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			for (size_t i = 1; i <= count; ++i)
				enumerator(data[begin+count-i]);
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>)
		{
			CriticalSection guard(lock);
			bool stop = false;
			for (size_t i = 0; i < count && !stop; ++i)
				enumerator(data[begin+i], &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>)
		{
			CriticalSection guard(lock);
			bool stop = false;
			for (size_t i = 1; i <= count && !stop; ++i)
				enumerator(data[begin+count-i], &stop);
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			bool stop = false;
			for (size_t i = 0; i < count && !stop; ++i)
				enumerator(data[begin+i], &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			bool stop = false;
			for (size_t i = 1; i <= count && !stop; ++i)
				enumerator(data[begin+count-i], &stop);
		}
		void ReserveFront(size_t n)
		{
			if (begin > n)
				return;

			size_t reqSize = maxSize+n;
			size_t newSize = maxSize > InitialSize ? maxSize : InitialSize;
			while (reqSize > newSize)
				newSize += newSize / 2;

			VALUE* dataNew = (VALUE*)Allocator::Alloc(sizeof(VALUE) * newSize);
			size_t offset = newSize - maxSize;
			if (data)
			{
				if (count)
					memcpy(&dataNew[begin+offset], &data[begin], sizeof(VALUE) * count);
				Allocator::Free(data);
			}
			maxSize += offset;
			begin += offset;
			data = dataNew;
		}
		void ReserveBack(size_t n)
		{
			if (maxSize >= begin + count + n)
				return;

			size_t reqSize = begin+count+n;
			size_t newSize = maxSize > InitialSize ? maxSize : InitialSize;
			while (reqSize > newSize)
				newSize += newSize / 2;

			VALUE* dataNew = (VALUE*)Allocator::Alloc(sizeof(VALUE) * newSize);
			if (data)
			{
				if (count)
					memcpy(&dataNew[begin], &data[begin], sizeof(VALUE) * count);
				Allocator::Free(data);
			}
			maxSize = newSize;
			data = dataNew;
		}
		void Balance(void)
		{
			size_t frontSpace = begin;
			size_t backSpace = maxSize - (begin+count);
			size_t begin2 = begin;

			if ((frontSpace > (backSpace + count) * 2) || (backSpace > (frontSpace + count) * 2))		// 앞뒤 공간차이가 나머지의 두배보다 크게 난다.
				begin2 = maxSize / 2;

			if (begin2 != begin)
			{
				if (count > 0)
					memmove(&data[begin2], &data[begin], sizeof(VALUE) * count);
				begin = begin2;
			}
		}

		size_t	begin;
		size_t	count;
		size_t	maxSize;
		VALUE* data;
	};
}
