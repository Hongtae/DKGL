//
//  File: DKFixedSizeAllocator.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include <algorithm>
#include "../DKinclude.h"
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
		size_t UnitSize,				// allocation size (fixed size)
		size_t Alignment = 1,
		size_t MaxUnits = 1024,			// max units per chunk
		typename Lock = DKSpinLock,
		typename BaseAllocator = DKMemoryDefaultAllocator // for alloc chunks.
	>
	class DKFixedSizeAllocator
	{
		static_assert(UnitSize > 0, "Size must be greater than zero.");
		static_assert(MaxUnits > 4, "MaxUnits must be greater than four.");
		static_assert(Alignment > 0, "Alignment must be greater than zero.");
		static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of two.");

		enum { MaxUnitsPerChunk = MaxUnits };

		union Unit
		{
			enum { AlignedUnitSize = UnitSize + ((UnitSize % Alignment) ? (Alignment - (UnitSize % Alignment)) : 0) };

			unsigned char data[AlignedUnitSize];
			int nextUnitIndex;
		};
		static_assert((sizeof(Unit) % Alignment) == 0, "Invalid unit alignment");

		struct Chunk
		{
			enum { BufferLength = sizeof(Unit) * MaxUnitsPerChunk + Alignment - 1 };
			enum { EndOfUnits = -1 };

			Unit* units;
			int firstFreeUnitIndex;
			int lastFreeUnitIndex;
			int numFreeUnits;
			unsigned char data[BufferLength];

			Chunk(void)
				: firstFreeUnitIndex(0)
				, lastFreeUnitIndex(MaxUnitsPerChunk-1)
				, numFreeUnits(MaxUnitsPerChunk)
			{
				uintptr_t ptr = reinterpret_cast<uintptr_t>(&this->data[0]);
				if (ptr % Alignment)
					ptr += Alignment - (ptr % Alignment);
				DKASSERT_STD_DEBUG((ptr % Alignment) == 0);
				this->units = reinterpret_cast<Unit*>(ptr);
				for (int i = 0; i < MaxUnitsPerChunk-1; ++i)
					this->units[i].nextUnitIndex = i+1;
				this->units[MaxUnitsPerChunk-1].nextUnitIndex = EndOfUnits;
			}
			~Chunk(void)
			{
				DKASSERT_STD_DEBUG(this->firstFreeUnitIndex >= 0);
				DKASSERT_STD_DEBUG(this->lastFreeUnitIndex >= 0);
				DKASSERT_STD_DEBUG(this->numFreeUnits == MaxUnitsPerChunk);
			}
			void* Alloc(void)
			{
				DKASSERT_STD_DEBUG(this->numFreeUnits <= MaxUnitsPerChunk);
				if (this->numFreeUnits > 0)
				{
					// chunk has one or more unoccupied units.
					DKASSERT_STD_DEBUG(this->firstFreeUnitIndex >= 0);
					DKASSERT_STD_DEBUG(this->lastFreeUnitIndex >= 0);

					Unit& unit = this->units[this->firstFreeUnitIndex];
					if (this->lastFreeUnitIndex == this->firstFreeUnitIndex)
						this->lastFreeUnitIndex = unit.nextUnitIndex;
					this->firstFreeUnitIndex = unit.nextUnitIndex;
					this->numFreeUnits--;

					DKASSERT_STD_DEBUG((reinterpret_cast<uintptr_t>(&(unit.data[0])) % Alignment) == 0);

					return &(unit.data[0]);
				}
				return NULL;
			}
			bool Dealloc(uintptr_t p)
			{
				uintptr_t rangeBegin = this->FirstUnitAddress();
				if (p >= rangeBegin && p <= this->LastUnitAddress())
				{
					int index = (int)((p - rangeBegin) / sizeof(Unit));
					Unit& unit = this->units[index];
					unit.nextUnitIndex = EndOfUnits;

					if (this->lastFreeUnitIndex == EndOfUnits)
					{
						DKASSERT_STD_DEBUG(numFreeUnits == 0);
						DKASSERT_STD_DEBUG(this->firstFreeUnitIndex == EndOfUnits);
						this->firstFreeUnitIndex = index;
						this->lastFreeUnitIndex = index;
					}
					else
					{
						this->units[this->lastFreeUnitIndex].nextUnitIndex = index;
						this->lastFreeUnitIndex = index;
					}
					this->numFreeUnits++;
					DKASSERT_STD_DEBUG(this->numFreeUnits <= MaxUnitsPerChunk);
					return true;
				}
				return false;
			}
			uintptr_t FirstUnitAddress(void) const
			{
				return reinterpret_cast<uintptr_t>(&(units[0].data[0]));
			}
			uintptr_t LastUnitAddress(void) const
			{
				return reinterpret_cast<uintptr_t>(&(units[MaxUnitsPerChunk - 1].data[0]));
			}
		};

		struct ChunkTable
		{
			uintptr_t address;
			Chunk* chunk;
		};

		void SortChunkTable(void)
		{
			if (numChunks > 1)
			{
				std::sort(&chunkTable[0], &chunkTable[numChunks - 1],
						  [](const ChunkTable& lhs, const ChunkTable& rhs)
						  {
							  return lhs.address < rhs.address;
						  });
			}
		}
		Chunk* FindChunk(uintptr_t addr, size_t start, size_t count)
		{
			if (count > 2)
			{
				size_t med = count / 2;
				size_t medIndex = start + med;
				if (addr < chunkTable[medIndex].address)
					return FindChunk(addr, start, med);
				else if (addr > chunkTable[medIndex + 1].address)
					return FindChunk(addr, medIndex + 1, count - med - 1);
				if (addr == chunkTable[medIndex + 1].address)
					return chunkTable[medIndex + 1].chunk;
				return chunkTable[medIndex].chunk;
			}
			if (count > 1)
			{
				if (addr >= chunkTable[start+1].address)
					return chunkTable[start+1].chunk;
			}
			if (count > 0)
			{
				if (addr >= chunkTable[start].address)
					return chunkTable[start].chunk;
			}
			return NULL;
		}

		ChunkTable* chunkTable;
		Chunk* cachedChunk;				// for fast-alloc
		size_t maxAllocated;
		size_t numAllocated;
		size_t maxChunks;
		size_t numChunks;

		using CriticalSection = DKCriticalSection<Lock>;
		Lock lock;

		template <size_t BaseAlignment> struct _RebindAlignment
		{
			enum { AlignedUnitSize = UnitSize + ((UnitSize % BaseAlignment) ? (BaseAlignment - (UnitSize % BaseAlignment)) : 0) };
			using Allocator = DKFixedSizeAllocator<AlignedUnitSize, BaseAlignment, MaxUnits, Lock, BaseAllocator>;
		};

	public:
		enum { FixedLength = UnitSize };
		enum { BaseAlignment = Alignment };

		template <size_t Align>
		using RebindAlignment = typename _RebindAlignment<Align>::Allocator;

		// DKAllocator interface. Useful to DKObject<T> allocation.
		static DKAllocator& AllocatorInstance(void)
		{
			struct AllocatorWrapper : public DKAllocator
			{
				using Allocator = RebindAlignment<BaseAlignment>;
				void* Alloc(size_t s) override	{ return allocator.Alloc(s); }
				void Dealloc(void* p) override	{ allocator.Dealloc(p); }
				void Purge(void) override		{ allocator.Purge(); }

				DKMemoryLocation Location(void) const override	{ return (DKMemoryLocation)BaseAllocator::Location; }
				Allocator allocator;
			};
			static DKAllocatorChain::StaticInitializer init; // extend allocator life cycle.
			static AllocatorWrapper* instance = new AllocatorWrapper();
			return *instance;
		}

		void* Alloc(size_t s)
		{
			DKASSERT_STD_DEBUG(s <= FixedLength);
			CriticalSection guard(lock);
			numAllocated++;
			if (maxAllocated < numAllocated)
				maxAllocated = numAllocated;

			if (cachedChunk)
			{
				void* p = cachedChunk->Alloc();
				if (p)
					return p;
			}
			// find unoccupied unit from each chunks.
			for (size_t i = 0; i < numChunks; ++i)
			{
				Chunk* ch = chunkTable[i].chunk;
				DKASSERT_STD_DEBUG(ch != NULL);
				void* p = ch->Alloc();
				if (p)
				{
					cachedChunk = ch;
					return p;
				}
			}
			// no space, create new chunk.
			Chunk* ch = ::new (BaseAllocator::Alloc(sizeof(Chunk))) Chunk();
			numChunks++;
			chunkTable = (ChunkTable*)BaseAllocator::Realloc(chunkTable, sizeof(ChunkTable) * numChunks);
			ChunkTable& table = chunkTable[numChunks-1];
			table.address = ch->FirstUnitAddress();
			table.chunk = ch;
			SortChunkTable();
			cachedChunk = ch;

			if (maxChunks < numChunks)
				maxChunks = numChunks;

			return ch->Alloc();
		}

		bool ConditionalDealloc(void* ptr)
		{
			uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
			CriticalSection guard(lock);
			if (numChunks > 0)
			{
				if (addr >= chunkTable[0].address &&
					addr <= chunkTable[numChunks-1].address + sizeof(Unit) * MaxUnitsPerChunk)
				{
					Chunk* ch = FindChunk(addr, 0, numChunks);
					if (ch && ch->Dealloc(addr))
					{
						numAllocated--;

						if (cachedChunk == NULL || cachedChunk->numFreeUnits < ch->numFreeUnits)
							cachedChunk = ch;

						return true;
					}
				}
			}
			return false;
		}

		void Dealloc(void* ptr)
		{
			uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
			CriticalSection guard(lock);

			Chunk* ch = FindChunk(addr, 0, numChunks);
			if (ch && ch->Dealloc(addr))
			{
				numAllocated--;

				if (cachedChunk == NULL || cachedChunk->numFreeUnits < ch->numFreeUnits)
					cachedChunk = ch;

				return;
			}
			// error: ptr was not allocated from this allocator!
			DKASSERT_STD_DESC_DEBUG(false, "Given address was not allocated from this allocator!");
		}

		void Reserve(size_t n)		// preallocate
		{
			if (n > 0)
			{
				size_t numChunksRequired = 0;
				while (numChunksRequired * MaxUnitsPerChunk < n)
					numChunksRequired++;

				CriticalSection guard(lock);
				if (numChunksRequired > numChunks)
				{
					chunkTable = (ChunkTable*)DKMemoryDefaultAllocator::Realloc(chunkTable, sizeof(ChunkTable) * numChunksRequired);
					for (size_t i = numChunks; i < numChunksRequired; ++i)
					{
						Chunk* ch = ::new (BaseAllocator::Alloc(sizeof(Chunk))) Chunk();
						ChunkTable& table = chunkTable[i];
						table.address = ch->FirstUnitAddress();
						table.chunk = ch;
					}
					cachedChunk = chunkTable[numChunks].chunk;
					numChunks = numChunksRequired;
					SortChunkTable();

					if (maxChunks < numChunks)
						maxChunks = numChunks;
				}
			}
		}

		void Purge(void)	// delete unoccupied chunks
		{
			CriticalSection guard(lock);
			size_t availableChunks = 0;
			for (size_t i = 0; i < numChunks; ++i)
			{
				Chunk* ch = chunkTable[i].chunk;
				if (ch->numFreeUnits == MaxUnitsPerChunk)
				{
					ch->~Chunk();
					BaseAllocator::Free(ch);
					chunkTable[i].chunk = NULL;
				}
				else
				{
					availableChunks++;
				}
			}
			if (availableChunks != numChunks)
			{
				if (availableChunks > 0)
				{
					ChunkTable* table = (ChunkTable*)BaseAllocator::Alloc(sizeof(ChunkTable) * availableChunks);
					size_t index = 0;
					for (size_t i = 0; i < numChunks; ++i)
					{
						if (chunkTable[i].chunk)
							table[index++] = chunkTable[i];
					}
					BaseAllocator::Free(chunkTable);
					chunkTable = table;
				}
				else
				{
					BaseAllocator::Free(chunkTable);
					chunkTable = NULL;
				}
				numChunks = availableChunks;
				cachedChunk = NULL;
			}
		}

		DKFixedSizeAllocator(void)
			: chunkTable(NULL)
			, cachedChunk(NULL)
			, numAllocated(0)
			, maxAllocated(0)
			, numChunks(0)
			, maxChunks(0)
		{
		}
		
		virtual ~DKFixedSizeAllocator(void)
		{
			DKASSERT_STD_DEBUG(numAllocated == 0);
			if (numChunks > 0)
			{
				DKASSERT_STD_DEBUG(chunkTable != NULL);
				for (size_t i = 0; i < numChunks; ++i)
				{
					Chunk* ch = chunkTable[i].chunk;
					ch->~Chunk();
					BaseAllocator::Free(ch);
				}
				BaseAllocator::Free(chunkTable);
			}
		}
	};
}
