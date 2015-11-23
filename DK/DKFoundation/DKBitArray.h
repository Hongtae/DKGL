//
//  File: DKBitArray.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include <initializer_list>
#include "DKTypeTraits.h"
#include "DKDummyLock.h"
#include "DKCriticalSection.h"
#include "DKMemory.h"
#include "DKFunction.h"


////////////////////////////////////////////////////////////////////////////////
// DKBitArray
// A special array that contains bit.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename LOCK = DKDummyLock, typename ALLOC = DKMemoryDefaultAllocator>
	class DKBitArray
	{
	public:
		using Lock = LOCK;
		using Allocator = ALLOC;
		using Index = size_t;
		using CriticalSection = DKCriticalSection < Lock > ;

		using Unit = size_t;
		enum { BitsPerUnit = sizeof(Unit) * 8 };

		enum : Index { IndexNotFound = (Index)-1 };

		Lock lock;

		// implementation for range-based-for-loop.
		typedef DKArrayRBIterator<DKBitArray, bool>				RBIterator;
		typedef DKArrayRBIterator<const DKBitArray, bool>		ConstRBIterator;
		RBIterator begin(void)				{ return RBIterator(*this, 0); }
		ConstRBIterator begin(void) const	{ return ConstRBIterator(*this, 0); }
		RBIterator end(void)				{ return RBIterator(*this, this->Count()); }
		ConstRBIterator end(void) const		{ return ConstRBIterator(*this, this->Count()); }

		DKBitArray(void) : data(NULL), count(0), capacity(0)
		{
		}
		DKBitArray(const bool* values, size_t count) : data(NULL), count(0), capacity(0)
		{
			Add(values, count);
		}
		DKBitArray(bool value, size_t count) : data(NULL), count(0), capacity(0)
		{
			Add(value, count);
		}
		DKBitArray(const DKBitArray& a) : data(NULL), count(0), capacity(0)
		{
			Add(a);
		}
		DKBitArray(DKBitArray&& a) : data(a.data), count(a.count), capacity(a.capacity)
		{
			a.data = NULL;
			a.count = 0;
			a.capacity = 0;
		}
		DKBitArray(std::initializer_list<bool> il) : data(NULL), count(0), capacity(0)
		{
			if (il.size() > 0)
			{
				ReserveNL(il.size());
				for (bool b : il)
				{
					count++;
					SetValueAtIndexNL(count-1, b);
				}
			}
		}
		~DKBitArray(void)
		{
			if (data)
				Allocator::Free(data);
		}
		bool IsEmpty(void) const
		{
			CriticalSection guard(lock);
			return count == 0;
		}
		// append one item to tail.
		Index Add(bool value)
		{
			return Add(value, 1);
		}
		// append 's' length of values to tail.
		Index Add(const bool* values, size_t s)
		{
			CriticalSection guard(lock);
			Index offset = count;
			ReserveItemCapsNL(s);
			count += s;
			for (Index i = 0; i < s; ++i)
				SetValueAtIndexNL(offset + i, values[i]);
			return offset;
		}
		// append value to tail 's' times. (value x s)
		Index Add(bool value, size_t s)
		{
			CriticalSection guard(lock);
			Index offset = count;
			ReserveItemCapsNL(s);
			count += s;
			for (Index i = 0; i < s; ++i)
				SetValueAtIndexNL(offset + i, value);
			return offset;
		}
		// append other array's elements to tail.
		template <typename T, typename U>
		Index Add(const DKBitArray<T, U>& value)
		{
			typename DKBitArray<T, U>::CriticalSection guard(value.lock);
			CriticalSection guard2(lock);
			Index offset = count;
			ReserveItemCapsNL(value.count);
			count += value.count;
			for (Index i = 0; i < value.count; ++i)
				SetValueAtIndexNL(offset + i, value.Value(i));
			return offset;
		}
		// append initializer-list items to tail.
		Index Add(std::initializer_list<bool> il)
		{
			CriticalSection guard(lock);
			Index offset = count;
			size_t s = il.size();
			ReserveItemCapsNL(s);
			for (bool b : il)
			{
				size_t i = count++;
				SetValueAtIndexNL(offset + i, b);
			}
			return offset;
		}
		// insert array's elements into position 'pos'.
		template <typename ...Args>
		Index Insert(const DKBitArray<Args...>& value, Index pos)
		{
			typename DKBitArray<Args...>::CriticalSection guard(value.lock);
			CriticalSection guard2(lock);
			size_t s = value.count;
			if (s > 0)
			{
				if (pos >= count)
				{
					pos = count;
					ReserveItemCapsNL(s);
					count += s;
				}
				else
					RightShiftFromIndexNL(pos, s);
				for (size_t i = 0; i < s ; ++i)
				{
					SetValueAtIndexNL(pos+i, value.ValueAtIndexNL(i));
				}
			}
			return pos;
		}
		// insert one value into position 'pos'.
		Index Insert(bool value, Index pos)
		{
			CriticalSection guard(lock);
			if (pos >= count)
			{
				pos = count;
				ReserveItemCapsNL(1);
				count += 1;
			}
			else
				RightShiftFromIndexNL(pos, 1);
			SetValueAtIndexNL(pos, value);
			return pos;
		}
		// insert 's' length of values into position 'pos'.
		Index Insert(const bool* values, size_t s, Index pos)
		{
			CriticalSection guard(lock);
			if (pos >= count)
			{
				pos = count;
				ReserveItemCapsNL(s);
				count += s;
			}
			else
				RightShiftFromIndexNL(pos, s);
			for (size_t i = 0; i < s; ++i)
				SetValueAtIndexNL(pos + i, values[i]);
			return pos;
		}
		// insert value 's' times into position 'pos'.
		Index Insert(bool value, size_t s, Index pos)
		{
			CriticalSection guard(lock);
			if (pos >= count)
			{
				pos = count;
				ReserveItemCapsNL(s);
				count += s;
			}
			else
				RightShiftFromIndexNL(pos, s);
			for (size_t i = 0; i < s; ++i)
				SetValueAtIndexNL(pos + i, value);
			return pos;
		}
		// insert initializer-list into position 'pos'.
		Index Insert(std::initializer_list<bool> il, Index pos)
		{
			CriticalSection guard(lock);
			size_t s = il.size();
			if (pos >= count)
			{
				pos = count;
				ReserveItemCapsNL(s);
				count += s;
			}
			else
				RightShiftFromIndexNL(pos, s);
			for (bool value : il)
			{
				SetValueAtIndexNL(pos++, value);
			}
			return pos - s;
		}
		// remove 'c' items at pos. (c = count)
		size_t Remove(Index pos, size_t c = 1)
		{
			CriticalSection guard(lock);
			if (pos < count && c > 0)
				LeftShiftFromIndexNL(pos, c);
			return count;
		}

		void Clear(void)
		{
			CriticalSection guard(lock);
			count = 0;
		}
		size_t Count(void) const
		{
			CriticalSection guard(lock);
			return count;
		}
		size_t Capacity(void) const
		{
			CriticalSection guard(lock);
			return capacity;
		}
		void Resize(size_t s)
		{
			CriticalSection guard(lock);
			if (count < s)	// extend
				ReserveNL(s);
			count = s;
		}
		void Resize(size_t s, bool val)
		{
			CriticalSection guard(lock);
			if (count < s)	// extend
			{
				ReserveNL(s);
				Index offset = count;
				count = s;
				for (Index i = offset; i < s; ++i)
					SetValueAtIndexNL(i, val);
			}
			else
				count = s;
		}
		void Reserve(size_t c)
		{
			CriticalSection guard(lock);
			ReserveNL(c);
		}
		void SetValue(Index i, bool b)
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > i);
			SetValueAtIndexNL(i, b);
		}
		bool Value(Index i) const
		{
			CriticalSection guard(lock);
			DKASSERT_DEBUG(count > i);
			return ValueAtIndexNL(i);
		}

		DKBitArray& operator = (DKBitArray&& other)
		{
			if (this != &other)
			{
				Clear();
				if (data)
					Allocator::Free(data);
				data = other.data;
				count = other.count;
				capacity = other.capacity;
				other.data = NULL;
				other.count = 0;
				other.capacity = 0;
			}
			return *this;
		}
		DKBitArray& operator = (const DKBitArray& value)
		{
			if (this == &value)
				return *this;

			Clear();
			Add(value);
			return *this;
		}
		DKBitArray& operator = (std::initializer_list<bool> il)
		{
			Clear();
			Add(il);
			return *this;
		}
		DKBitArray operator + (bool v) const
		{
			DKBitArray	ret(*this);
			ret.Add(v);
			return ret;
		}
		DKBitArray operator + (const DKBitArray& value) const
		{
			DKBitArray ret(*this);
			ret.Add(value);
			return ret;
		}
		DKBitArray operator + (std::initializer_list<bool> il) const
		{
			DKBitArray ret(*this);
			ret.Add(il);
			return ret;
		}
		DKBitArray& operator += (bool v) const
		{
			Add(v);
			return *this;
		}
		DKBitArray& operator += (const DKBitArray& value)
		{
			Add(value);
			return *this;
		}
		DKBitArray& operator += (std::initializer_list<bool> il)
		{
			Add(il);
			return *this;
		}

	private:
		void LeftShiftFromIndexNL(Index i, size_t c)
		{
			DKASSERT_DEBUG(i < count);
			if (c > 0)
			{
				if ( c + i >= count )
				{
					count = i;
					return;
				}

				size_t numUnits = c / BitsPerUnit;	// num units to shift
				size_t numBits = c % BitsPerUnit;	// num bits to shift

				size_t index = i / BitsPerUnit;		// unit index where 'i' exists
				const size_t unitCount = UnitLengthForBits(count);

				unsigned int firstBitsOffset = i % BitsPerUnit;
				Unit firstUnitBits = data[index] & (((Unit)1 << firstBitsOffset) - 1);

				if (numBits > 0)
				{
					Unit mask = ((Unit)1 << numBits) - 1;
					Unit tmp = 0;
					Index next = index + numUnits;

					while (next + 1 < unitCount)
					{
						tmp = (data[next + 1] & mask) << (BitsPerUnit - numBits);
						data[index] = tmp | (data[next] >> numBits);
						++index;
						++next;
					}
					data[index] = data[next] >> numBits;
				}
				else if (numUnits > 0)	/*numBits == 0*/
				{
					Index next = index + numUnits;
					while (next < unitCount)
					{
						data[index] = data[next];
						++index;
						++next;
					}
				}

				if (firstBitsOffset)	// restore first unit's bits (at index i)
				{
					index = i / BitsPerUnit;
					data[index] = (data[index] & ~(((Unit)1 << firstBitsOffset) - 1)) | firstUnitBits;
				}

				count -= c;
			}
		}
		void RightShiftFromIndexNL(Index i, size_t c)
		{
			DKASSERT_DEBUG(i < count);
			if (c > 0)
			{
				ReserveItemCapsNL(c);

				size_t numUnits = c / BitsPerUnit;	// num units to shift
				size_t numBits = c % BitsPerUnit;	// num bits to shift

				size_t index = i / BitsPerUnit;		// unit index where 'i' exists
				const size_t unitCount = UnitLengthForBits(count + c);

				unsigned int firstBitsOffset = i % BitsPerUnit;
				Unit firstUnitBits = data[index] & (((Unit)1 << firstBitsOffset) - 1);

				if (numBits > 0)
				{
					Unit mask = ~(((Unit)1 << (BitsPerUnit-numBits)) - 1);
					Unit tmp = 0;

					Index idx = unitCount - numUnits - 1;
					while (idx >= index && idx < unitCount)
					{
						tmp = idx > 0 ? ((data[idx-1] & mask) >> (BitsPerUnit - numBits)) : 0;
						data[idx + numUnits] = tmp | (data[idx] << numBits);
						--idx;
					}
				}
				else if (numUnits > 0)	/*numBits == 0*/
				{
					Index idx = unitCount - numUnits - 1;
					while (idx >= index && idx < unitCount)
					{
						data[idx + numUnits] = data[idx];
						--idx;
					}
				}

				if (firstBitsOffset)	// restore first unit's bits (at index i)
				{
					index = i / BitsPerUnit;
					data[index] = (data[index] & ~(((Unit)1 << firstBitsOffset) - 1)) | firstUnitBits;
				}

				count += c;
			}
		}

		void SetValueAtIndexNL(Index i, bool b)
		{
			DKASSERT_DEBUG(count > i);
			size_t unitIdx = i / BitsPerUnit;
			size_t bitsIdx = i % BitsPerUnit;

			Unit bmask = ((Unit)1 << bitsIdx);
			if (b)
				data[unitIdx] |= bmask;
			else
				data[unitIdx] &= ~bmask;
		}
		bool ValueAtIndexNL(Index i) const
		{
			DKASSERT_DEBUG(count > i);
			size_t unitIdx = i / BitsPerUnit;
			size_t bitsIdx = i % BitsPerUnit;

			return (data[unitIdx] >> bitsIdx) & 1;
		}
		void ReserveNL(size_t c)
		{
			if (c <= capacity)
				return;

			size_t reqBytes = sizeof(Unit) * UnitLengthForBits(c);
			DKASSERT_DEBUG(reqBytes);

			Unit* old = data;
			if (data)
				data = (Unit*)Allocator::Realloc(data, reqBytes);
			else
				data = (Unit*)Allocator::Alloc(reqBytes);

			DKASSERT_DESC_DEBUG(data, "Out of memory!");

			if (data)
				capacity = reqBytes * 8;
			else	// out of memory!
				data = old;
		}
		void ReserveItemCapsNL(size_t c)
		{
			if (c > 0)
			{
				if (capacity < c + count || count == capacity)
				{
					ReserveNL(count + ((count/2) > c ? (count/2): c ));
				}
			}
		}
		size_t UnitLengthForBits(size_t numBits)
		{
			return (numBits / BitsPerUnit) + ((numBits % BitsPerUnit) ? 1 : 0);
		}
		
		
		Unit*		data;
		size_t		count;
		size_t		capacity;
	};
}
