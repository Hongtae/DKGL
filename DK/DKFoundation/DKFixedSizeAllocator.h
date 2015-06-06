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
	int UnitSize,			// allocation size (fixed size)
	int Alignment = 1,		// byte alignment
	int MaxUnits = 1024,		// max units per chunk
	typename Lock = DKSpinLock,
	typename BaseAllocator = DKMemoryDefaultAllocator // for alloc chunks.
	>
	class DKFixedSizeAllocator
	{
		static_assert(UnitSize > 0, "Size must be greater than zero.");
		static_assert(MaxUnits > 4, "MaxUnits must be greater than four.");
		static_assert(Alignment > 0, "Alignment must be greater than zero.");
		static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of two.");

		// maximum number of units per chunk.
		enum { MaxUnitsPerChunk = MaxUnits };

		union Unit
		{
			enum { AlignedUnitSize = UnitSize + ((UnitSize % Alignment) ? (Alignment - (UnitSize % Alignment)) : 0) };

			unsigned char data[AlignedUnitSize];
			int nextUnitIndex;
		};
		static_assert((sizeof(Unit) % Alignment) == 0, "Invalid unit alignment");

		// size of all units per chunk.
		enum { MaxUnitsPerChunkSize = sizeof(Unit) * MaxUnitsPerChunk };

		struct Chunk
		{
			enum { BufferLength = MaxUnitsPerChunkSize + Alignment - 1 };
			enum { EndOfUnits = -1 };

			int firstFreeUnitIndex;
			int lastFreeUnitIndex;
			Unit* units;		// aligned address
			char data[BufferLength];

			Chunk(void)
			: firstFreeUnitIndex(0)
			, lastFreeUnitIndex(MaxUnitsPerChunk - 1)
			{
				uintptr_t ptr = reinterpret_cast<uintptr_t>(&this->data[0]);
				if (ptr % Alignment)
					ptr += Alignment - (ptr % Alignment);
				DKASSERT_STD_DEBUG((ptr % Alignment) == 0);
				this->units = reinterpret_cast<Unit*>(ptr);
				for (int i = 0; i < MaxUnitsPerChunk - 1; ++i)
					this->units[i].nextUnitIndex = i + 1;
				this->units[MaxUnitsPerChunk - 1].nextUnitIndex = EndOfUnits;
			}
			~Chunk(void)
			{
				DKASSERT_STD_DEBUG(this->firstFreeUnitIndex != EndOfUnits);
				DKASSERT_STD_DEBUG(this->lastFreeUnitIndex != EndOfUnits);
			}
			FORCEINLINE void* Alloc(void)
			{
				if (this->firstFreeUnitIndex != EndOfUnits)
				{
					// chunk has one or more unoccupied units.
					DKASSERT_STD_DEBUG(this->firstFreeUnitIndex >= 0);
					DKASSERT_STD_DEBUG(this->lastFreeUnitIndex >= 0);

					Unit& unit = this->units[this->firstFreeUnitIndex];
					if (this->lastFreeUnitIndex == this->firstFreeUnitIndex)
						this->lastFreeUnitIndex = unit.nextUnitIndex;
					this->firstFreeUnitIndex = unit.nextUnitIndex;

					DKASSERT_STD_DEBUG((reinterpret_cast<uintptr_t>(&(unit.data[0])) % Alignment) == 0);

					return &(unit.data[0]);
				}
				return NULL;
			}
			FORCEINLINE bool Dealloc(uintptr_t p)
			{
				uintptr_t rangeBegin = this->FirstUnitAddress();
				if (p >= rangeBegin && p <= this->LastUnitAddress())
				{
					int index = (int)((p - rangeBegin) / sizeof(Unit));
					Unit& unit = this->units[index];
					unit.nextUnitIndex = EndOfUnits;

					if (this->lastFreeUnitIndex == EndOfUnits)
					{
						DKASSERT_STD_DEBUG(this->firstFreeUnitIndex == EndOfUnits);
						this->firstFreeUnitIndex = index;
						this->lastFreeUnitIndex = index;
					}
					else
					{
						this->units[this->lastFreeUnitIndex].nextUnitIndex = index;
						this->lastFreeUnitIndex = index;
					}
					return true;
				}
				return false;
			}
			FORCEINLINE uintptr_t FirstUnitAddress(void) const
			{
				return reinterpret_cast<uintptr_t>(&(units[0].data[0]));
			}
			FORCEINLINE uintptr_t LastUnitAddress(void) const
			{
				return reinterpret_cast<uintptr_t>(&(units[MaxUnitsPerChunk - 1].data[0]));
			}
		};

#pragma pack(push, 1)
		struct ChunkInfo
		{
			uintptr_t address;
			int offset;
			int occupied;
		};
#pragma pack(pop)

		using CriticalSection = DKCriticalSection<Lock>;

		template <size_t BaseAlignment> struct _RebindAlignment
		{
			enum { AlignedUnitSize = UnitSize + ((UnitSize % BaseAlignment) ? (BaseAlignment - (UnitSize % BaseAlignment)) : 0) };
			using Allocator = DKFixedSizeAllocator < AlignedUnitSize, BaseAlignment, MaxUnits, Lock, BaseAllocator > ;
		};

	public:
		enum { FixedLength = UnitSize };
		enum { BaseAlignment = Alignment };
		enum { ChunkSize = sizeof(Chunk) };

		template <size_t Align>
		using RebindAlignment = typename _RebindAlignment<Align>::Allocator;

		struct AllocatorInterface : public DKAllocator
		{
			virtual void Reserve(size_t) = 0;
			virtual bool HasAddress(void*) const = 0;
		};

		// DKAllocator instance (shared), with alignment
		static AllocatorInterface& AllocatorInstance(void)
		{
			// shared instance located in RebindAlignment<BaseAlignment>
			return RebindAlignment<BaseAlignment>::StaticAllocatorInstance();
		}

		void* Alloc(size_t s)
		{
			void* ptr = NULL;
			Chunk* ch = NULL;

			DKASSERT_STD_DEBUG(s <= FixedLength);
			if (s > FixedLength)
				return ptr;

			CriticalSection guard(lock);
			numAllocated++;

			if (cachedChunk && cachedChunk->occupied < MaxUnitsPerChunk)
			{
				ch = ChunkFromChunkInfo(cachedChunk);
				ptr = ch->Alloc();
				DKASSERT_STD_DEBUG(ptr);
				if (cachedChunk->occupied == 0)
				{
					DKASSERT_STD_DEBUG(emptyChunks > 0);
					emptyChunks--;
				}
				cachedChunk->occupied++;
				return ptr;
			}
			// find unoccupied unit from each chunks.
			for (size_t i = 0; i < numChunks; ++i)
			{
				if (chunkTable[i].occupied < MaxUnitsPerChunk)
				{
					cachedChunk = &chunkTable[i];
					ch = ChunkFromChunkInfo(cachedChunk);
					ptr = ch->Alloc();
					DKASSERT_STD_DEBUG(ptr);
					if (cachedChunk->occupied == 0)
					{
						DKASSERT_STD_DEBUG(emptyChunks > 0);
						emptyChunks--;
					}
					cachedChunk->occupied++;
					return ptr;
				}
			}
			// no space, create new chunk.
			ch = ::new (BaseAllocator::Alloc(sizeof(Chunk))) Chunk();
			ptr = ch->Alloc();
			DKASSERT_STD_DEBUG(ptr);
			numChunks++;
			chunkTable = (ChunkInfo*)BaseAllocator::Realloc(chunkTable, sizeof(ChunkInfo) * numChunks);
			ChunkInfo* info = &chunkTable[numChunks - 1];
			info->address = ch->FirstUnitAddress();
			info->offset = static_cast<unsigned int>(info->address - reinterpret_cast<uintptr_t>(ch));
			info->occupied = 1;
			SortChunkTable();
			cachedChunk = FindChunkInfo(reinterpret_cast<uintptr_t>(ptr));
			addressBegin = chunkTable[0].address;
			addressEnd = chunkTable[numChunks-1].address + MaxUnitsPerChunkSize;
			return ptr;
		}

		bool ConditionalDealloc(void* ptr)
		{
			uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
			CriticalSection guard(lock);
			if (numChunks > 0 && addr >= addressBegin && addr < addressEnd)
			{
				return FindChunkAndDealloc(addr);
			}
			return false;
		}

		void Dealloc(void* ptr)
		{
			uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
			CriticalSection guard(lock);
			if (FindChunkAndDealloc(addr))
				return;

			// error: ptr was not allocated from this allocator!
			DKASSERT_STD_DESC_DEBUG(false, "Given address was not allocated from this allocator!");
		}

		bool HasAddress(void* ptr) const
		{
			uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
			CriticalSection guard(lock);
			if (numChunks > 0 && addr >= addressBegin && addr < addressEnd)
			{
				return FindChunk(addr) != NULL;
			}
			return false;
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
					chunkTable = (ChunkInfo*)DKMemoryDefaultAllocator::Realloc(chunkTable, sizeof(ChunkInfo) * numChunksRequired);
					for (size_t i = numChunks; i < numChunksRequired; ++i)
					{
						Chunk* ch = ::new (BaseAllocator::Alloc(sizeof(Chunk))) Chunk();
						ChunkInfo& info = chunkTable[i];
						info.address = ch->FirstUnitAddress();
						info.offset = static_cast<unsigned int>(info.address - reinterpret_cast<uintptr_t>(ch));
						info.occupied = 0;
						emptyChunks++;
					}
					uintptr_t addr = chunkTable[numChunks].address;
					numChunks = numChunksRequired;
					SortChunkTable();
					if (cachedChunk == NULL || cachedChunk->occupied == MaxUnitsPerChunk)
						cachedChunk = FindChunkInfo(addr);
					DKASSERT_STD_DEBUG(cachedChunk != NULL);

					addressBegin = chunkTable[0].address;
					addressEnd = chunkTable[numChunks-1].address + MaxUnitsPerChunkSize;
				}
			}
		}

		size_t ConditionalPurge(size_t threshold)
		{
			bool shouldPurge = false;
			if (this->emptyChunks > 0)
			{
				if ((this->numChunks * MaxUnitsPerChunk) >=
					(this->numAllocated + threshold + MaxUnitsPerChunk))
					shouldPurge = true;
			}
			if (shouldPurge)
				return this->Purge();
			return 0;
		}

		size_t Purge(void)	// delete unoccupied chunks
		{
			CriticalSection guard(lock);
			if (emptyChunks > 0)
			{
				size_t availableChunks = 0;
				for (size_t i = 0; i < numChunks; ++i)
				{
					if (chunkTable[i].occupied == 0)
					{
						Chunk* ch = ChunkFromChunkInfo(&chunkTable[i]);
						ch->~Chunk();
						BaseAllocator::Free(ch);
						chunkTable[i].address = NULL;
					}
					else
					{
						availableChunks++;
					}
				}
				DKASSERT_STD_DEBUG(numChunks >= availableChunks);
				if (availableChunks != numChunks)
				{
					if (availableChunks > 0)
					{
						ChunkInfo* table = (ChunkInfo*)BaseAllocator::Alloc(sizeof(ChunkInfo) * availableChunks);
						cachedChunk = NULL;
						size_t index = 0;
						for (size_t i = 0; i < numChunks; ++i)
						{
							if (chunkTable[i].address)
							{
								table[index] = chunkTable[i];
								if (table[index].occupied < MaxUnitsPerChunk)
								{
									if (cachedChunk == NULL || cachedChunk->occupied < table[index].occupied)
										cachedChunk = &table[index];
								}
								index++;
							}
						}
						BaseAllocator::Free(chunkTable);
						chunkTable = table;
						numChunks = availableChunks;
						addressBegin = chunkTable[0].address;
						addressEnd = chunkTable[numChunks-1].address + MaxUnitsPerChunkSize;
					}
					else
					{
						DKASSERT_STD_DEBUG(numAllocated == 0);
						BaseAllocator::Free(chunkTable);
						chunkTable = NULL;
						cachedChunk = NULL;
						numChunks = 0;
						addressBegin = 0;
						addressEnd = 0;
					}
				}
				emptyChunks = 0;
				return (numChunks - availableChunks) * MaxUnitsPerChunkSize;
			}
			return 0;
		}

		DKFixedSizeAllocator(void)
		: chunkTable(NULL)
		, cachedChunk(NULL)
		, addressBegin(0)
		, addressEnd(0)
		, numAllocated(0)
		, numChunks(0)
		, emptyChunks(0)
		{
		}

		~DKFixedSizeAllocator(void)
		{
			DKASSERT_STD_DEBUG(numAllocated == 0);
			if (numChunks > 0)
			{
				DKASSERT_STD_DEBUG(chunkTable != NULL);
				for (size_t i = 0; i < numChunks; ++i)
				{
					Chunk* ch = ChunkFromChunkInfo(&chunkTable[i]);
					ch->~Chunk();
					BaseAllocator::Free(ch);
				}
				BaseAllocator::Free(chunkTable);
			}
		}

	private:
		FORCEINLINE Chunk* ChunkFromChunkInfo(const ChunkInfo* info) const
		{
			return reinterpret_cast<Chunk*>(info->address - info->offset);
		}
		FORCEINLINE void SortChunkTable(void)
		{
			if (numChunks > 1)
			{
				std::sort(&chunkTable[0], &chunkTable[numChunks],
						  [](const ChunkInfo& lhs, const ChunkInfo& rhs)
						  {
							  return lhs.address < rhs.address;
						  });
			}
		}
		/* lower-bound search */
		ChunkInfo* FindChunkInfo(uintptr_t addr, size_t start, size_t count) const
		{
			if (count > 2)
			{
				size_t med = count / 2;
				size_t medIndex = start + med;
				if (addr < chunkTable[medIndex].address)
					return FindChunkInfo(addr, start, med);
				else if (addr > chunkTable[medIndex + 1].address)
					return FindChunkInfo(addr, medIndex + 1, count - med - 1);
				if (addr == chunkTable[medIndex + 1].address)
					return &chunkTable[medIndex + 1];
				return &chunkTable[medIndex];
			}
			if (count > 1)
			{
				if (addr >= chunkTable[start + 1].address)
					return &chunkTable[start + 1];
			}
			if (count > 0)
			{
				if (addr >= chunkTable[start].address)
					return &chunkTable[start];
			}
			return NULL;
		}
		FORCEINLINE ChunkInfo* FindChunkInfo(uintptr_t addr) const
		{
			return FindChunkInfo(addr, 0, numChunks);
		}
		FORCEINLINE Chunk* FindChunk(uintptr_t addr) const
		{
			ChunkInfo* info = const_cast<DKFixedSizeAllocator*>(this)->FindChunkInfo(addr);
			if (info && addr < info->address + MaxUnitsPerChunkSize)
				return ChunkFromChunkInfo(info);
			return NULL;
		}
		FORCEINLINE bool FindChunkAndDealloc(uintptr_t addr)
		{
			ChunkInfo* info = FindChunkInfo(addr);
			if (info && addr < info->address + MaxUnitsPerChunkSize)
			{
				Chunk* ch = ChunkFromChunkInfo(info);
				if (ch->Dealloc(addr))
				{
					numAllocated--;
					DKASSERT_STD_DEBUG(info->occupied > 0);
					info->occupied--;

					if (cachedChunk == NULL || cachedChunk->occupied < info->occupied)
						cachedChunk = info;

					if (info->occupied == 0)
						emptyChunks++;

					return true;
				}
			}
			return false;
		}

		// Create static instance. (shared)
		// This function was declared as private, to prevent each other template
		// argumented classes creates it's own instance. Use AllocatorInstance().
		static AllocatorInterface& StaticAllocatorInstance(void)
		{
			struct AllocatorWrapper : public AllocatorInterface
			{
				void* Alloc(size_t s) override				{ return allocator.Alloc(s); }
				void Dealloc(void* p) override				{ return allocator.Dealloc(p); }
				void Reserve(size_t s) override				{ return allocator.Reserve(s); }
				size_t Purge(void) override					{ return allocator.Purge(); }
				bool HasAddress(void* p) const override		{ return allocator.HasAddress(p); }


				DKMemoryLocation Location(void) const override	{ return (DKMemoryLocation)BaseAllocator::Location; }
				DKFixedSizeAllocator allocator;
			};
			static DKAllocatorChain::StaticInitializer init; // extend allocator life cycle.
			static AllocatorWrapper* instance = new AllocatorWrapper();
			return *instance;
		}

		ChunkInfo* chunkTable;
		ChunkInfo* cachedChunk;		// for fast-alloc
		uintptr_t addressBegin;
		uintptr_t addressEnd;
		size_t numAllocated;
		size_t numChunks;
		size_t emptyChunks;
		Lock lock;
	};
}
