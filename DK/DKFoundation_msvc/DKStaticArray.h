//
//  File: DKStaticArray.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypeTraits.h"
#include "DKFunction.h"

#ifndef DKSTATICARRAY_USE_STL_SORT
// Set 1 if you want to use stl sort (std::sort) as your default sort function.
// std::sort could be faster.
#define DKSTATICARRAY_USE_STL_SORT	0
#endif

#ifndef DKSTATICARRAY_USE_STATIC_ROTATE
// Set 1 if you don't want additional memory allocation for item rotation.
// For lots of elements, memory allocated rotation is faster normally.
#define DKSTATICARRAY_USE_STATIC_ROTATE	0
#endif

#if DKSTATICARRAY_USE_STL_SORT
#include <algorithm>
#endif

////////////////////////////////////////////////////////////////////////////////
// DKStaticArray
// A simple array with fixed length (non-allocationg).
// This class provides sorting algorithm.
// You can use std::sort for default sorting algorithm
// by defining DKSTATICARRAY_USE_STL_SORT=1
//
// This class using external array (by pointer), of course, not thread safe.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	// iterator class for range-based for loop
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
		DKArrayRBIterator& operator ++ (void)	// prefix++
		{
			++position;
			return *this;
		}
		ITEMREF operator * (void)
		{
			DKASSERT_DEBUG(container.Count() >= position);
			return container.Value(position);
		}
	};

	template <typename VALUE> class DKStaticArray
	{
	public:
		typedef size_t					Index;
		typedef DKTypeTraits<VALUE>		ValueTraits;

		static const Index invalidIndex = (size_t)-1;

		// Iterator class for range-based for loop
		typedef DKArrayRBIterator<DKStaticArray, VALUE&>				RBIterator;
		typedef DKArrayRBIterator<const DKStaticArray, const VALUE&>	ConstRBIterator;
		RBIterator begin(void)				{return RBIterator(*this, 0);}
		ConstRBIterator begin(void) const	{return ConstRBIterator(*this, 0);}
		RBIterator end(void)				{return RBIterator(*this, this->Count());}
		ConstRBIterator end(void) const		{return ConstRBIterator(*this, this->Count());}

		DKStaticArray(void)
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
		~DKStaticArray(void)
		{
		}
		bool IsEmpty(void) const
		{
			return count == 0;
		}
		size_t Count(void) const
		{
			return count;
		}
		void Clear(void)
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
		operator VALUE* (void)
		{
			if (count > 0)
				return data;
			return NULL;
		}
		operator const VALUE* (void) const
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
				size_t left = n % count;
				if (left > 0)
				{
#if DKSTATICARRAY_USE_STATIC_ROTATE
					size_t first = 0;
					size_t next = left;
					size_t mid = left;
					while (first != next)
					{
						Swap(first, next);
						++first;
						++next;
						if (next == count)
							next = mid;
						else if (first == mid)
							mid = next;						
					}
#else
					size_t right = count - left;
					if (right < left)
					{
						void* tmp = DKMemoryHeapAlloc(sizeof(VALUE) * right);
						memcpy(tmp, &data[left], sizeof(VALUE) * right);
						memmove(&data[right], data, sizeof(VALUE) * left);
						memcpy(data, tmp, sizeof(VALUE) * right);
						DKMemoryHeapFree(tmp);
					}
					else
					{
						void* tmp = DKMemoryHeapAlloc(sizeof(VALUE) * left);
						memcpy(tmp, data, sizeof(VALUE) * left);
						memmove(data, &data[left], sizeof(VALUE) * right);
						memcpy(&data[right], tmp, sizeof(VALUE) * left);
						DKMemoryHeapFree(tmp);
					}
#endif
				}
			}
		}
		void RightRotate(size_t n)
		{
			if (count > 1)
			{
				return LeftRotate(count - (n % count));
			}	
		}
		void Swap(Index v1, Index v2)
		{
			DKASSERT_DEBUG(v1 < count);
			DKASSERT_DEBUG(v2 < count);

			if (v1 != v2)
			{
				unsigned char* p1 = reinterpret_cast<unsigned char*>(&data[v1]);
				unsigned char* p2 = reinterpret_cast<unsigned char*>(&data[v2]);
				for (int i = 0; i < sizeof(VALUE); ++i)
				{
					unsigned char tmp = p1[i];
					p1[i] = p2[i];
					p2[i] = tmp;
				}
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
			std::sort(&data[0], &data[0] + count, cmp);
#else
			SortLoop<CompareFunc>(0, count, cmp);
#endif
			}
		}
		// enumerate all items.
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
		template <typename CompareFunc> Index Partition(Index begin, Index end, Index pivot, CompareFunc cmp)
		{
			Index store = begin;
			Swap(pivot, end);
			for (Index i = begin; i < end; i++)
			{
				if (cmp(data[i], data[end]))
				{
					Swap(store, i);
					store++;
				}
			}
			Swap(end, store);
			return store;
		}
		// determines position of sorting pivot.
		// TODO: a better algorithm needed.
		template <typename CompareFunc> Index PivotMedian(Index begin, Index end, CompareFunc cmp)
		{
			Index pivot = begin + (end - begin)/2;

			if (cmp(data[begin], data[pivot]) && cmp(data[end], data[begin]))
				pivot = begin;
			else if (cmp(data[end], data[pivot]) && cmp(data[begin], data[end]))
				pivot = end;
			return pivot;
		}
		template <typename CompareFunc> void InsertionSort(Index begin, Index end, CompareFunc cmp)
		{
			for (Index i = begin + 1; i < end; i++)
			{
				for (Index k = i ; k > begin && cmp(data[k], data[k-1]); k--)
					Swap(k, k-1);
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
				Swap(pos, great);
				Heapify<CompareFunc>(begin, great, size, cmp);
			}
		}
		template <typename CompareFunc> void BuildHeap(Index begin, size_t size, CompareFunc cmp)
		{
			for (size_t i = ((size - 1) / 2) + 1; i > 0; --i)
				Heapify<CompareFunc>(begin, begin+(i-1), size, cmp);
		}
		template <typename CompareFunc> void HeapSort(Index begin, Index end, CompareFunc cmp)
		{
			size_t size = end - begin;
			BuildHeap<CompareFunc>(begin, size, cmp);
			for (size_t i = size; i > 0; i--)
			{
				Swap(begin, begin+size-1);
				size--;
				Heapify<CompareFunc>(begin, begin, size, cmp);
			}
		}
		template <typename CompareFunc> void SortLoop(Index begin, Index end, CompareFunc cmp)
		{
			Index factor = end - begin;
			if (factor > 32)
			{
				Index pivot = PivotMedian<CompareFunc>(begin, end-1, cmp);
				pivot = Partition<CompareFunc>(begin, end-1, pivot, cmp);

				SortLoop<CompareFunc>(begin, pivot, cmp);
				SortLoop<CompareFunc>(pivot+1, end, cmp);
			}
			else if (factor > 16)
			{
				HeapSort<CompareFunc>(begin, end, cmp);
			}
			else if (factor > 2)
			{
				InsertionSort<CompareFunc>(begin, end, cmp);
			}
			else if (factor > 1)
			{
				if (cmp(data[begin+1], data[begin]))
					Swap(begin, begin+1);
			}
		}

		VALUE*	data;
		size_t	count;
	};
}
