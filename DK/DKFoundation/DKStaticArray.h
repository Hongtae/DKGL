//
//  File: DKStaticArray.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypeTraits.h"
#include "DKFunction.h"

#ifndef DKSTATICARRAY_USE_STL_SORT
/// Set 1 if you want to use stl sort (std::sort) as your default sort function.
/// std::sort could be faster.
#define DKSTATICARRAY_USE_STL_SORT	0
#endif

#if DKSTATICARRAY_USE_STL_SORT
#include <algorithm>
#endif

namespace DKFoundation
{
	/// DKStaticArray iterator class for range-based for loop
	template <typename CONTAINER, typename ITEMREF>
	struct DKArrayRBIterator
	{
		typedef typename CONTAINER::Index POSITION;

		CONTAINER& container;
		POSITION position;

		DKArrayRBIterator(CONTAINER& c, POSITION pos)
			: container(c), position(pos)
		{
			DKASSERT_DEBUG(container.Count() >= position);
		}
		bool operator != (const DKArrayRBIterator& it) const
		{
			return this->position != it.position;
		}
		DKArrayRBIterator& operator ++ ()	// prefix++
		{
			++position;
			return *this;
		}
		ITEMREF operator * ()
		{
			DKASSERT_DEBUG(container.Count() >= position);
			return container.Value(position);
		}
	};

	/// A simple array with fixed length (static).
	/// This class provides sorting algorithm.
	/// You can use std::sort for default sorting algorithm
	/// by defining DKSTATICARRAY_USE_STL_SORT=1
	///
	/// This class using external array (by pointer), not thread safe.
	template <typename VALUE> class DKStaticArray
	{
	public:
		typedef size_t					Index;
		typedef DKTypeTraits<VALUE>		ValueTraits;

		constexpr static size_t NodeSize()	{ return sizeof(VALUE); }

		enum { UseMemoryCopy = 0 };
		using SwapMethod = DKNumber<UseMemoryCopy>;

		enum : Index { IndexNotFound = ~Index(0) };

		/// Iterator class for range-based for loop
		typedef DKArrayRBIterator<DKStaticArray, VALUE&>				RBIterator;
		typedef DKArrayRBIterator<const DKStaticArray, const VALUE&>	ConstRBIterator;
		RBIterator begin()				{return RBIterator(*this, 0);}
		ConstRBIterator begin() const	{return ConstRBIterator(*this, 0);}
		RBIterator end()				{return RBIterator(*this, this->Count());}
		ConstRBIterator end() const		{return ConstRBIterator(*this, this->Count());}

		DKStaticArray()
			: data(NULL), count(0)
		{
		}
		DKStaticArray(VALUE* v, size_t c)
			: data(NULL), count(0)
		{
			Reset(v, c);
		}
		DKStaticArray(const DKStaticArray& v)
			: data(NULL), count(0)
		{
			Reset(v);
		}
		~DKStaticArray()
		{
		}
		bool IsEmpty() const
		{
			return count == 0;
		}
		size_t Count() const
		{
			return count;
		}
		void Clear()
		{
			Reset(NULL, 0);
		}
		VALUE& Value(Index index)
		{
			DKASSERT_DEBUG(count > index);
			return data[index];
		}
		const VALUE& Value(Index index) const
		{
			DKASSERT_DEBUG(count > index);
			return data[index];
		}
		operator VALUE* ()
		{
			if (count > 0)
				return data;
			return NULL;
		}
		operator const VALUE* () const
		{
			if (count > 0)
				return data;
			return NULL;
		}
		DKStaticArray& Reset(VALUE* v, size_t c)
		{
			if (v && c > 0)
			{
				data = v;
				count = c;
			}
			else
			{
				data = NULL;
				count = 0;
			}
			return *this;
		}
		DKStaticArray& Reset(const DKStaticArray& v)
		{
			return Reset(v.data, v.count);
		}
		DKStaticArray& operator = (const DKStaticArray& v)
		{
			return Reset(v);
		}
		void LeftRotate(size_t n)
		{
			if (count > 1)
			{
				LeftRotate(n, SwapMethod());
			}
		}
		void RightRotate(size_t n)
		{
			if (count > 1)
			{
				return LeftRotate(count - (n % count));
			}	
		}
		template <typename T, typename Comparator>
		Index LowerBound(T&& value, Comparator&& cmp) const
		{
			Index begin = 0;
			size_t count = this->count;
			size_t mid;
			while (count > 0)
			{
				mid = count / 2;
				if (cmp(data[begin + mid], value))
				{
					begin += mid + 1;
					count -= mid + 1;
				}
				else
					count = mid;
			}
			return begin;
		}
		template <typename T, typename Comparator>
		Index UpperBound(T&& value, Comparator&& cmp) const
		{
			Index begin = 0;
			size_t count = this->count;
			size_t mid;
			while (count > 0)
			{
				mid = count / 2;
				if (!cmp(value, data[begin + mid]))
				{
					begin += mid + 1;
					count -= mid + 1;
				}
				else
					count = mid;
			}
			return begin;
		}
		void Swap(Index v1, Index v2)
		{
			DKASSERT_DEBUG(v1 < count);
			DKASSERT_DEBUG(v2 < count);

			if (v1 != v2)
			{
				Swap(v1, v2, SwapMethod());
			}
		}
		void Sort(const DKFunctionSignature<bool (const VALUE&, const VALUE&)>* cmp)
		{
			Sort<const Comparator&>(Comparator(cmp));
		}
		template <typename CompareFunc> void Sort(CompareFunc cmp)
		{
			if (count > 1)
			{
#if DKSTATICARRAY_USE_STL_SORT
				std::sort(std::addressof(data[0]), std::addressof(data[count]), cmp);
#else
				size_t depth = 1;
				for (size_t n = count; n; n >>= 1)
					++depth;
				depth = (depth << 1);

				using CompareFuncRef = typename DKTypeTraits<CompareFunc>::ReferredType&;
				SortLoop<CompareFuncRef>(0, count-1, depth, cmp);
#endif
			}
		}
		/// enumerate all items.
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
			for (Index i = 0; i < count; ++i)
				enumerator(data[i]);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>)
		{
			for (Index i = 1; i <= count; ++i)
				enumerator(data[count - i]);
		}
		// lambda enumerator (const VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>) const
		{
			for (Index i = 0; i < count; ++i)
				enumerator(data[i]);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>) const
		{
			for (Index i = 1; i <= count; ++i)
				enumerator(data[count - i]);
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>)
		{
			bool stop = false;
			for (Index i = 0; i < count && !stop; ++i)
				enumerator(data[i], &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>)
		{
			bool stop = false;
			for (Index i = 1; i <= count && !stop; ++i)
				enumerator(data[count - i], &stop);
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>) const
		{
			bool stop = false;
			for (Index i = 0; i < count && !stop; ++i)
				enumerator(data[i], &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>) const
		{
			bool stop = false;
			for (Index i = 1; i <= count && !stop; ++i)
				enumerator(data[count - i], &stop);
		}

		FORCEINLINE void LeftRotate(size_t n, const DKNumber<0>&)
		{
			size_t left = n % count;
			if (left > 0)
			{
				size_t first = 0;
				size_t next = left;
				size_t mid = left;
				while (first != next)
				{
					Swap(first, next, SwapMethod());
					++first;
					++next;
					if (next == count)
						next = mid;
					else if (first == mid)
						mid = next;
				}
			}
		}
		FORCEINLINE void LeftRotate(size_t n, const DKNumber<1>&)
		{
			size_t left = n % count;
			if (left > 0)
			{
				size_t right = count - left;
				if (right < left)
				{
					void* tmp = DKMalloc(sizeof(VALUE) * right);
					memcpy(tmp, std::addressof(data[left]), sizeof(VALUE) * right);
					memmove(std::addressof(data[right]), data, sizeof(VALUE) * left);
					memcpy(data, tmp, sizeof(VALUE) * right);
					DKFree(tmp);
				}
				else
				{
					void* tmp = DKMalloc(sizeof(VALUE) * left);
					memcpy(tmp, data, sizeof(VALUE) * left);
					memmove(data, std::addressof(data[left]), sizeof(VALUE) * right);
					memcpy(std::addressof(data[right]), tmp, sizeof(VALUE) * left);
					DKFree(tmp);
				}
			}
		}

		struct Comparator
		{
			typedef DKFunctionSignature<bool (const VALUE&, const VALUE&)> Function;
			Comparator(const Function* f) : func(f) {}
			bool operator() (const VALUE& lhs, const VALUE& rhs) const
			{
				return func->Invoke(lhs, rhs);
			}
			const Function* func;
		};

		FORCEINLINE void Swap(Index v1, Index v2, const DKNumber<0>&)
		{
			DKASSERT_DEBUG(v1 < count);
			DKASSERT_DEBUG(v2 < count);
			DKASSERT_DEBUG(v1 != v2);

			VALUE tmp = std::move(data[v1]);
			data[v1] = std::move(data[v2]);
			data[v2] = std::move(tmp);
		}
		FORCEINLINE void Swap(Index v1, Index v2, const DKNumber<1>&)
		{
			DKASSERT_DEBUG(v1 < count);
			DKASSERT_DEBUG(v2 < count);
			DKASSERT_DEBUG(v1 != v2);

			uint8_t tmp[sizeof(VALUE)];
			memcpy(tmp, std::addressof(data[v1]), sizeof(VALUE));
			memcpy(std::addressof(data[v1]), std::addressof(data[v2]), sizeof(VALUE));
			memcpy(std::addressof(data[v2]), tmp, sizeof(VALUE));
		}

		template <typename CompareFunc> Index Partition(Index begin, Index end, CompareFunc cmp)
		{
			Index medium = begin + ((end - begin + 1) >> 1);

			Index pivotIndex;
			if (cmp(data[begin], data[medium]))
			{
				if (cmp(data[medium], data[end]))
					pivotIndex = medium;
				else if (cmp(data[begin], data[end]))
					pivotIndex = end;
				else
					pivotIndex = begin;
			}
			else if (cmp(data[begin], data[end]))
				pivotIndex = begin;
			else if (cmp(data[medium], data[end]))
				pivotIndex = end;
			else
				pivotIndex = medium;

			VALUE pivot = data[pivotIndex];

			while (true)
			{
				while (cmp(data[begin], pivot))
					++begin;
				while (cmp(pivot, data[end]))
					--end;
				if (!(begin < end))
					break;

				Swap(begin, end, SwapMethod());

				++begin;
				--end;
			}
			return begin;
		}
		template <typename CompareFunc> void InsertionSort(Index begin, Index end, CompareFunc cmp)
		{
			for (Index i = begin + 1; i <= end; ++i)
			{
				for (Index k = i ; k > begin && cmp(data[k], data[k-1]); --k)
					Swap(k, k-1, SwapMethod());
			}
		}
		template <typename CompareFunc> void Heapify(Index begin, Index pos, size_t size, CompareFunc cmp)
		{
			Index i = pos - begin;
			Index left = begin + (2*i)+1;
			Index great = pos;
			Index right = begin + (2*i)+2;
			if ((left < begin + size) && cmp(data[pos], data[left]))
			{
				great = left;
			}
			if ((right < begin + size) && cmp(data[great], data[right]))
			{
				great = right;
			}
			if (great != pos)
			{
				Swap(pos, great, SwapMethod());
				Heapify<CompareFunc>(begin, great, size, cmp);
			}
		}
		template <typename CompareFunc> void BuildHeap(Index begin, size_t size, CompareFunc cmp)
		{
			for (size_t i = ((size - 1) >> 1) + 1; i > 0; --i)
				Heapify<CompareFunc>(begin, begin+(i-1), size, cmp);
		}
		template <typename CompareFunc> void HeapSort(Index begin, Index end, CompareFunc cmp)
		{
			size_t size = end - begin + 1;
			BuildHeap<CompareFunc>(begin, size, cmp);
			while (--size)
			{
				Swap(begin, begin + size, SwapMethod());
				Heapify<CompareFunc>(begin, begin, size, cmp);
			}
		} 

		template <typename CompareFunc> void SortLoop(Index begin, Index end, size_t depth, CompareFunc cmp)
		{
			Index c = end - begin + 1;
#if 0
			while (c > 16)
			{
				if (depth)
				{
					--depth;
					Index pivot = Partition<CompareFunc>(begin, end, cmp);
					SortLoop<CompareFunc>(pivot, end, depth, cmp);
					end = pivot - 1;
					c = end - begin + 1;
				}
				else
				{
					HeapSort<CompareFunc>(begin, end, cmp);
					return;
				}
			}
#else
			if (c > 16)
			{
				if (depth)
				{
					--depth;
					Index pivot = Partition<CompareFunc>(begin, end, cmp);
					SortLoop<CompareFunc>(begin, pivot-1, depth, cmp);
					SortLoop<CompareFunc>(pivot, end, depth, cmp);
				}
				else
				{
					HeapSort<CompareFunc>(begin, end, cmp);
				}
			}
			else
#endif
			if (c > 2)
			{
				InsertionSort<CompareFunc>(begin, end, cmp);
			}
			else if (c > 1)
			{
				if (cmp(data[end], data[begin]))
					Swap(begin, end, SwapMethod());
			}
		}

		VALUE*	data;
		size_t	count;
	};
}
