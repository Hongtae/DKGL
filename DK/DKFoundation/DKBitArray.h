//
//  File: DKBitArray.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include <initializer_list>
#include "../DKInclude.h"
#include "DKTypeTraits.h"
#include "DKMemory.h"
#include "DKFunction.h"

namespace DKFoundation
{
	/// @brief A special array that contains bit data.
	/// This class does not support enumeration or pointer-casting operator,
	/// because of only two values (true, false) are acceptable.
	template <typename ALLOC = DKMemoryDefaultAllocator>
	class DKBitArray
	{
	public:
		using Allocator = ALLOC;
		using Index = size_t;

		using Unit = size_t;
		enum { BitsPerUnit = sizeof(Unit) * 8 };

		enum : Index { IndexNotFound = ~Index(0) };

		typedef DKArrayRBIterator<DKBitArray, bool>				RBIterator;						///<  implementation for range-based-for-loop.
		typedef DKArrayRBIterator<const DKBitArray, bool>		ConstRBIterator;				///<  implementation for range-based-for-loop.
		RBIterator begin()				{ return RBIterator(*this, 0); }					///<  implementation for range-based-for-loop.
		ConstRBIterator begin() const	{ return ConstRBIterator(*this, 0); }				///<  implementation for range-based-for-loop.
		RBIterator end()				{ return RBIterator(*this, this->Count()); }		///<  implementation for range-based-for-loop.
		ConstRBIterator end() const		{ return ConstRBIterator(*this, this->Count()); }	///<  implementation for range-based-for-loop.

		DKBitArray() : data(NULL), count(0), capacity(0)
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
				Reserve(il.size());
				for (bool b : il)
				{
					count++;
					SetValue(count-1, b);
				}
			}
		}
		~DKBitArray()
		{
			if (data)
				Allocator::Free(data);
		}
		bool IsEmpty() const
		{
			return count == 0;
		}
		/// append one item to tail.
		Index Add(bool value)
		{
			return Add(value, 1);
		}
		/// append 's' length of values to tail.
		Index Add(const bool* values, size_t s)
		{
			Index offset = count;
			ReserveItemCaps(s);
			count += s;
			for (Index i = 0; i < s; ++i)
				SetValue(offset + i, values[i]);
			return offset;
		}
		/// append value to tail 's' times. (value x s)
		Index Add(bool value, size_t s)
		{
			Index offset = count;
			ReserveItemCaps(s);
			count += s;
			for (Index i = 0; i < s; ++i)
				SetValue(offset + i, value);
			return offset;
		}
		/// append other array's elements to tail.
		template <typename T>
		Index Add(const DKBitArray<T>& value)
		{
			Index offset = count;
			ReserveItemCaps(value.count);
			count += value.count;
			for (Index i = 0; i < value.count; ++i)
				SetValue(offset + i, value.Value(i));
			return offset;
		}
		/// append initializer-list items to tail.
		Index Add(std::initializer_list<bool> il)
		{
			Index offset = count;
			size_t s = il.size();
			ReserveItemCaps(s);
			for (bool b : il)
			{
				size_t i = count++;
				SetValue(offset + i, b);
			}
			return offset;
		}
		/// insert array's elements into position 'pos'.
		template <typename T>
		Index Insert(const DKBitArray<T>& value, Index pos)
		{
			size_t s = value.count;
			if (s > 0)
			{
				if (pos >= count)
				{
					pos = count;
					ReserveItemCaps(s);
					count += s;
				}
				else
					RightShiftFromIndex(pos, s);
				for (size_t i = 0; i < s ; ++i)
				{
					SetValue(pos+i, value.Value(i));
				}
			}
			return pos;
		}
		/// insert one value into position 'pos'.
		Index Insert(bool value, Index pos)
		{
			if (pos >= count)
			{
				pos = count;
				ReserveItemCaps(1);
				count += 1;
			}
			else
				RightShiftFromIndex(pos, 1);
			SetValue(pos, value);
			return pos;
		}
		/// insert 's' length of values into position 'pos'.
		Index Insert(const bool* values, size_t s, Index pos)
		{
			if (pos >= count)
			{
				pos = count;
				ReserveItemCaps(s);
				count += s;
			}
			else
				RightShiftFromIndex(pos, s);
			for (size_t i = 0; i < s; ++i)
				SetValue(pos + i, values[i]);
			return pos;
		}
		/// insert value 's' times into position 'pos'.
		Index Insert(bool value, size_t s, Index pos)
		{
			if (pos >= count)
			{
				pos = count;
				ReserveItemCaps(s);
				count += s;
			}
			else
				RightShiftFromIndex(pos, s);
			for (size_t i = 0; i < s; ++i)
				SetValue(pos + i, value);
			return pos;
		}
		/// insert initializer-list into position 'pos'.
		Index Insert(std::initializer_list<bool> il, Index pos)
		{
			size_t s = il.size();
			if (pos >= count)
			{
				pos = count;
				ReserveItemCaps(s);
				count += s;
			}
			else
				RightShiftFromIndex(pos, s);
			for (bool value : il)
			{
				SetValue(pos++, value);
			}
			return pos - s;
		}
		/// remove 'c' items at pos. (c = count)
		size_t Remove(Index pos, size_t c = 1)
		{
			if (pos < count && c > 0)
				LeftShiftFromIndex(pos, c);
			return count;
		}

		void Clear()
		{
			count = 0;
		}
		size_t Count() const
		{
			return count;
		}
		size_t Capacity() const
		{
			return capacity;
		}
		void ShrinkToFit()
		{
			size_t countBytes = UnitLengthForBits(count);
			size_t capacityBytes = UnitLengthForBits(capacity);
			if (countBytes != capacityBytes)
			{
				DKASSERT_DEBUG(capacityBytes > countBytes);
				DKASSERT_DEBUG(data);
				if (countBytes > 0)
				{
					Unit* tmp = (Unit*)Allocator::Realloc(data, countBytes);
					DKASSERT_DESC_DEBUG(tmp, "Out of memory!");
					if (tmp)
					{
						data = tmp;
						capacity = countBytes * 8;
					}
				}
				else
				{
					DKASSERT_DEBUG(count == 0);
					Allocator::Free(data);
					data = 0;
					capacity = 0;
				}
			}
		}
		void Resize(size_t s)
		{
			if (count < s)	// extend
				Reserve(s);
			count = s;
		}
		void Resize(size_t s, bool val)
		{
			if (count < s)	// extend
			{
				Reserve(s);
				Index offset = count;
				count = s;
				for (Index i = offset; i < s; ++i)
					SetValue(i, val);
			}
			else
				count = s;
		}
		void Reserve(size_t c)
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
		void SetValue(Index i, bool b)
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
		bool Value(Index i) const
		{
			DKASSERT_DEBUG(count > i);
			size_t unitIdx = i / BitsPerUnit;
			size_t bitsIdx = i % BitsPerUnit;

			return (data[unitIdx] >> bitsIdx) & 1;
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
		void LeftShiftFromIndex(Index i, size_t c)
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
		void RightShiftFromIndex(Index i, size_t c)
		{
			DKASSERT_DEBUG(i < count);
			if (c > 0)
			{
				ReserveItemCaps(c);

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

		void ReserveItemCaps(size_t c)
		{
			if (c > 0)
			{
				if (capacity < c + count || count == capacity)
				{
					Reserve(count + ((count/2) > c ? (count/2): c ));
				}
			}
		}
		size_t UnitLengthForBits(size_t numBits)
		{
			return (numBits / BitsPerUnit) + ((numBits % BitsPerUnit) ? 1 : 0);
		}
		
		
		Unit*		data;
		size_t		count;		// num items
		size_t		capacity;	// bits (not bytes)
	};
}
