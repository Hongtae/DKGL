//
//  File: DKFixedSizeAllocator.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKTypes.h"
#include "DKTypeTraits.h"
#include "DKAllocatorChain.h"
#include "DKAllocator.h"
#include "DKMemory.h"
#include "DKSpinLock.h"
#include "DKCriticalSection.h"

////////////////////////////////////////////////////////////////////////////////
// DKFixedSizeAllocator
// an allocator which can allocate memory of fixed length.
// it is useful to template collection classes like DKMap, DKSet.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <
		size_t UnitSize,			// allocation size (fixed size)
		size_t MaxUnits = 32,		// max units per chunk
		typename Lock = DKSpinLock,
		typename BaseAllocator = DKMemoryDefaultAllocator // for alloc chunks.
	>
	class DKFixedSizeAllocator : public DKAllocatorChain
	{
		static_assert(UnitSize > 0, "Size must be greater than zero.");
		static_assert(DKNumMatches<MaxUnits, 8, 16, 32, 64>() == 1,
					  "MaxUnits must be one of 8,16,32,64.");

		// C++11 does not allow 'explicit spealization',
		// to compile properly, we need specify one or more types which is not
		// used but required. (type 'U' in below template struct)
		template <int, typename U=void> struct _BitMaskType;
		template <typename U> struct _BitMaskType< 8, U> { using Type = unsigned char; };
		template <typename U> struct _BitMaskType<16, U> { using Type = unsigned short; };
		template <typename U> struct _BitMaskType<32, U> { using Type = unsigned int; };
		template <typename U> struct _BitMaskType<64, U> { using Type = unsigned long long; };

		using BitMaskType = typename _BitMaskType<MaxUnits>::Type;
		static_assert(sizeof(BitMaskType) * 8 == MaxUnits, "BitMaskType is invalid.");
		static const BitMaskType fullOccupied = (BitMaskType)-1;

		enum {maxUnitsPerChunk = MaxUnits};

		using Unit = unsigned char[UnitSize];
		struct Chunk
		{
			Unit units[maxUnitsPerChunk];
			BitMaskType occupied;
			Chunk* next; /* linked-list */
		};
		static_assert(DKTypeTraits<Chunk>::IsPod(), "Invalid chunk type");

		Chunk* firstChunk;
		size_t maxAllocated;
		size_t numAllocated;
		size_t maxChunks;
		size_t numChunks;

		using CriticalSection = DKCriticalSection<Lock>;
		Lock lock;

	public:
		enum { FixedLength = UnitSize };

		static DKFixedSizeAllocator& Instance(void)
		{
			static StaticInitializer init; // extend allocator life cycle.
			static DKFixedSizeAllocator* instance = new DKFixedSizeAllocator();
			return *instance;
		}

		// DKAllocator interface. Useful to DKObject<T> allocation.
		static DKAllocator& AllocatorInstance(void)
		{
			struct AllocatorWrapper : public DKAllocator
			{
				void* Alloc(size_t s) override					{ return Instance().Alloc(s); }
				void Dealloc(void* p) override					{ return Instance().Dealloc(p); }
				DKMemoryLocation Location(void) const override	{ return (DKMemoryLocation)BaseAllocator::Location; }
			};
			static StaticInitializer init; // extend allocator life cycle.
			static AllocatorWrapper* instance = new AllocatorWrapper();
			return *instance;
		}

		void* Alloc(size_t s)
		{
			DKASSERT_STD_DEBUG(s == FixedLength);
			CriticalSection guard(lock);
			numAllocated++;
			if (maxAllocated < numAllocated)
				maxAllocated = numAllocated;

			// find unoccupied unit from each chunks
			for (Chunk* ch = firstChunk; ch; ch = ch->next)
			{
				if (ch->occupied != fullOccupied)
				{
					// chunk has one or more unoccupied units.
					for (int i = 0; i < maxUnitsPerChunk; ++i)
					{
						unsigned int occupied = (ch->occupied >> i) & 1;
						if (!occupied)
						{
							ch->occupied |= (1 << i);
							return ch->units[i];
						}
					}
				}
			}
			// no space, create new chunk.
			numChunks++;
			Chunk* ch = (Chunk*)BaseAllocator::Alloc(sizeof(Chunk));
			ch->occupied = 1;
			ch->next = NULL;
			// add chunk to linked list
			if (firstChunk == NULL)
				firstChunk = ch;
			else
			{
				Chunk* last = firstChunk;
				while (last->next)
					last = last->next;
				last->next = ch;
			}
			return ch->units[0];
		}

		void Dealloc(void* ptr)
		{
			uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
			CriticalSection guard(lock);
			for (Chunk* ch = firstChunk; ch; ch = ch->next)
			{
				uintptr_t rangeBegin = reinterpret_cast<uintptr_t>(ch->units[0]);
				uintptr_t rangeEnd = reinterpret_cast<uintptr_t>(ch->units[maxUnitsPerChunk-1]);
				if (addr >= rangeBegin && addr <= rangeEnd)
				{
					unsigned int index = (unsigned int)((addr - rangeBegin) / sizeof(Unit));
					ch->occupied ^= (1 << index);
					numAllocated--;
					return;
				}
			}
			// error: ptr was not allocated from this allocator!
			DKASSERT_STD_DESC_DEBUG(false, "Given address was not allocated from this allocator!");
		}

		void Purge(void)	// delete unoccupied chunks
		{
			CriticalSection guard(lock);
			for (Chunk* ch = firstChunk; ch; ch = ch->next)
			{
				if (ch->next && ch->next->occupied == 0)
				{
					Chunk* freeChunk = ch->next;
					ch->next = freeChunk->next;
					BaseAllocator::Free(freeChunk);
					numChunks--;
				}
			}
			if (firstChunk && firstChunk->occupied == 0)
			{
				Chunk* freeChunk = firstChunk;
				firstChunk = freeChunk->next;
				BaseAllocator::Free(freeChunk);
				numChunks--;
			}
		}

		virtual ~DKFixedSizeAllocator(void)
		{
			DKASSERT_STD_DEBUG(numAllocated == 0);
			Chunk* nextChunk = firstChunk;
			while (nextChunk)
			{
				Chunk* ch = nextChunk;
				nextChunk = ch->next;

				BaseAllocator::Free(ch);
			}
		}

	protected:
		DKFixedSizeAllocator(void)
		: firstChunk(NULL)
		, numAllocated(0)
		, maxAllocated(0)
		, numChunks(0)
		, maxChunks(0)
		{
		}
	};
}
