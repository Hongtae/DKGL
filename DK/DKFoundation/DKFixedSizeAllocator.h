//
//  File: DKFixedSizeAllocator.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include <algorithm>
#include "../DKinclude.h"
#include "DKAllocator.h"
#include "DKMemory.h"
#include "DKSpinLock.h"
#include "DKCriticalSection.h"

namespace DKFoundation
{
	/// @brief An allocator which can allocate memory of fixed length.
	/// it is useful to template collection classes like DKMap, DKSet.
	///
	/// @tparam UnitSize       allocation size (fixed size)
	/// @tparam Alignment      byte alignment (default:1)
	/// @tparam MaxUnits       max units per chunk (default:1024)
	/// @tparam Lock           locking class
	/// @tparam BaseAllocator  internal allocator (for internal-table, small size)
	/// @tparam UnitAllocator  unit chunk allocator. (large size)
	template <
		unsigned int UnitSize,				// allocation size (fixed size)
		unsigned int Alignment = 1,			// byte alignment
		unsigned int MaxUnits = 1024,		// max units per chunk
		typename Lock = DKSpinLock,
		typename BaseAllocator = DKMemoryDefaultAllocator, // info table allocator. (small)
		typename UnitAllocator = DKMemoryDefaultAllocator  // unit chunk allocator. (large)
	>
	class DKFixedSizeAllocator
	{
		template <unsigned int, unsigned int, unsigned int, typename, typename, typename>
			friend class DKFixedSizeAllocator;
		static_assert(UnitSize > 0, "Size must be greater than zero.");
		static_assert(MaxUnits > 1, "MaxUnits must be greater than one.");
		static_assert(Alignment > 0, "Alignment must be greater than zero.");
		static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of two.");

		// maximum number of units per chunk.
		enum : uint32_t { MaxUnitsPerChunk = MaxUnits };
		enum : uint32_t { AlignedUnitSize = (UnitSize + (Alignment - 1)) & ~(Alignment - 1) };
		union Unit
		{
			uint8_t data[AlignedUnitSize];
			uint32_t nextUnitIndex;
		};
		static_assert((sizeof(Unit) % Alignment) == 0, "Invalid unit alignment");

		using Index = unsigned int;
		enum : Index { EndOfUnits = (Index)-1 };

		struct ChunkInfo
		{
			uintptr_t address;
			Index freeUnitIndex;
			uint16_t offset;
			uint16_t occupied;
		};

		// size of all units per chunk.
		enum : size_t { MaxUnitsPerChunkSize = sizeof(Unit) * MaxUnitsPerChunk };

		using CriticalSection = DKCriticalSection < Lock > ;

		template <unsigned int BaseAlignment> struct _RebindAlignment
		{
			enum { AlignedUnitSize = UnitSize + ((UnitSize % BaseAlignment) ? (BaseAlignment - (UnitSize % BaseAlignment)) : 0) };
			using Allocator = DKFixedSizeAllocator < AlignedUnitSize, BaseAlignment, MaxUnits, Lock, BaseAllocator, UnitAllocator > ;
		};

	public:
		enum { FixedLength = UnitSize };
		enum { BaseAlignment = Alignment };
		enum { AlignedChunkSize = MaxUnitsPerChunkSize + Alignment - 1 };

		template <unsigned int Align>
		using RebindAlignment = typename _RebindAlignment<Align>::Allocator;

		struct AllocatorInterface : public DKAllocator
		{
			virtual void Reserve(size_t) = 0;
			virtual void* AlignedChunkAddress(void*) const = 0;
		};

		/// DKAllocator instance (shared), with alignment
		static DKAllocator& AllocatorInstance(void)
		{
			// shared instance located in RebindAlignment<BaseAlignment>
			return RebindAlignment<BaseAlignment>::StaticAllocatorInstance();
		}

		void* Alloc(size_t s)
		{
			DKASSERT_MEM_DEBUG(s <= FixedLength);
			if (s > FixedLength)
				return NULL;

			CriticalSection guard(lock);

			if (cachedChunk && cachedChunk->occupied < MaxUnitsPerChunk)
			{
				uintptr_t ptr = AllocUnit(cachedChunk);
				DKASSERT_MEM_DEBUG(ptr);
				return reinterpret_cast<void*>(ptr);
			}
			// find unoccupied unit from each chunks.
			for (size_t i = 0; i < numChunks; ++i)
			{
				if (chunkTable[i].occupied < MaxUnitsPerChunk)
				{
					cachedChunk = &chunkTable[i];
					uintptr_t ptr = AllocUnit(cachedChunk);
					DKASSERT_MEM_DEBUG(ptr);
					return reinterpret_cast<void*>(ptr);
				}
			}
			// no space, create new chunk.
			cachedChunk = NULL;
			if (numChunks > 0)
			{
				ChunkInfo* table = (ChunkInfo*)BaseAllocator::Realloc(chunkTable, sizeof(ChunkInfo) * (numChunks + 1));
				if (table == NULL) // out of memory!
					return NULL;
				chunkTable = table;

				ChunkInfo chunk;
				if (!AllocChunk(&chunk))
					return NULL;	// out of memory!

				uintptr_t pos = reinterpret_cast<uintptr_t>(
															std::upper_bound(&chunkTable[0], &chunkTable[numChunks], chunk.address,
																			 [](uintptr_t lhs, const ChunkInfo& rhs)
																			 {
																				 return lhs < rhs.address;
																			 }));
				size_t chunkIndex = (pos - reinterpret_cast<uintptr_t>(&chunkTable[0])) / sizeof(ChunkInfo);

				if (chunkIndex < numChunks)
				{
#if 1
					memmove(&chunkTable[chunkIndex + 1], &chunkTable[chunkIndex], sizeof(ChunkInfo) * (numChunks - chunkIndex));
#else
					for (size_t i = numChunks; i > chunkIndex; --i)
						chunkTable[i] = chunkTable[i-1];
#endif
				}
				chunkTable[chunkIndex] = chunk;
				cachedChunk = &chunkTable[chunkIndex];
			}
			else
			{
				chunkTable = (ChunkInfo*)BaseAllocator::Alloc(sizeof(ChunkInfo) * (numChunks + 1));
				if (chunkTable == NULL)
					return NULL; // out of memory!

				cachedChunk = &chunkTable[numChunks];
				if (!AllocChunk(cachedChunk)) // out of memory!
				{
					BaseAllocator::Free(chunkTable);
					chunkTable = NULL;
					cachedChunk = NULL;
					return NULL;
				}
			}
			DKASSERT_MEM_DEBUG(cachedChunk);
			numChunks++;

			uintptr_t ptr = AllocUnit(cachedChunk);
			DKASSERT_MEM_DEBUG(ptr);
			return reinterpret_cast<void*>(ptr);
		}

		void Dealloc(void* ptr)
		{
			if (ptr)
			{
				CriticalSection guard(lock);
				if (FindChunkAndDealloc(reinterpret_cast<uintptr_t>(ptr)))
					return;

				// error: ptr was not allocated from this allocator!
				DKASSERT_MEM_DESC_DEBUG(false, "Given address was not allocated from this allocator!");
			}
		}

		bool ConditionalDealloc(void* ptr)
		{
			if (ptr)
			{
				CriticalSection guard(lock);
				if (numChunks > 0)
				{
					return FindChunkAndDealloc(reinterpret_cast<uintptr_t>(ptr));
				}
			}
			return false;
		}

		bool ConditionalDeallocAndPurge(void* ptr, size_t threshold, size_t* bytesPurged)
		{
			if (ptr)
			{
				CriticalSection guard(lock);
				if (numChunks > 0)
				{
					if (FindChunkAndDealloc(reinterpret_cast<uintptr_t>(ptr)))
					{
						if (this->emptyChunks > 0)
						{
							if ((this->numChunks * MaxUnitsPerChunk) >=
								(this->numAllocated + threshold + MaxUnitsPerChunk))
							{
								size_t purged = PurgeInternal();
								if (bytesPurged)
									*bytesPurged = purged;
							}
						}
						return true;
					}
				}
			}
			return false;
		}

		/// returns Chunk starting address if ptr was allocated from this object.
		void* AlignedChunkAddress(void* ptr) const
		{
			if (ptr)
			{
				CriticalSection guard(lock);
				if (numChunks > 0)
				{
					ChunkInfo* info = FindChunkInfo(reinterpret_cast<uintptr_t>(ptr));
					if (info)
						return reinterpret_cast<void*>(info->address);
				}
			}
			return NULL;
		}

		void Reserve(size_t n)		///< preallocate
		{
			if (n > 0)
			{
				size_t numChunksRequired = 0;
				while (numChunksRequired * MaxUnitsPerChunk < n)
					numChunksRequired++;

				CriticalSection guard(lock);
				if (numChunksRequired > numChunks)
				{
					ChunkInfo* table = (ChunkInfo*)BaseAllocator::Realloc(chunkTable, sizeof(ChunkInfo) * numChunksRequired);
					if (table)
					{
						chunkTable = table;
						for (size_t i = numChunks; i < numChunksRequired; ++i)
						{
							if (!AllocChunk(&chunkTable[i]))
							{
								// out of memory!
								break;
							}
							numChunks++;
						}
						if (numChunks > 0)
						{
							// save last chunk's address.
							uintptr_t addr = chunkTable[numChunks-1].address;
							SortChunkTable();
							if (cachedChunk == NULL || cachedChunk->occupied == MaxUnitsPerChunk)
								cachedChunk = FindChunkInfo(addr);
							DKASSERT_MEM_DEBUG(cachedChunk != NULL);
						}
					}
				}
			}
		}

		size_t ConditionalPurge(size_t threshold)
		{
			bool shouldPurge = false;
			CriticalSection guard(lock);
			if (this->emptyChunks > 0)
			{
				if ((this->numChunks * MaxUnitsPerChunk) >=
					(this->numAllocated + threshold + MaxUnitsPerChunk))
					shouldPurge = true;
			}
			if (shouldPurge)
				return this->PurgeInternal();
			return 0;
		}

		size_t Purge(void)	///< delete unoccupied chunks
		{
			CriticalSection guard(lock);
			return PurgeInternal();
		}

		size_t Size(void) const
		{
			CriticalSection guard(lock);
			return numChunks * (MaxUnitsPerChunkSize + sizeof(ChunkInfo));
		}

		size_t NumberOfAllocatedUnits(void) const
		{
			CriticalSection guard(lock);
			return numAllocated;
		}

		DKFixedSizeAllocator(void)
			: chunkTable(NULL)
			, cachedChunk(NULL)
			, numAllocated(0)
			, numChunks(0)
			, emptyChunks(0)
		{
		}

		~DKFixedSizeAllocator(void) noexcept(!DKGL_MEMORY_DEBUG)
		{
			DKASSERT_MEM_DEBUG(numAllocated == 0);
			if (numChunks > 0)
			{
				DKASSERT_MEM_DEBUG(chunkTable != NULL);
				for (size_t i = 0; i < numChunks; ++i)
				{
					DKASSERT_MEM_DEBUG(chunkTable[i].occupied == 0);
					FreeChunk(&chunkTable[i]);
				}
				DKASSERT_MEM_DEBUG(emptyChunks == 0);
				BaseAllocator::Free(chunkTable);
			}
		}

		DKFixedSizeAllocator(const DKFixedSizeAllocator&) = delete;
		DKFixedSizeAllocator& operator = (const DKFixedSizeAllocator&) = delete;

	private:
		FORCEINLINE bool AllocChunk(ChunkInfo* info)
		{
			uintptr_t ptr = reinterpret_cast<uintptr_t>(UnitAllocator::Alloc(AlignedChunkSize));
			if (ptr)
			{
				if (ptr % Alignment)
				{
					info->offset = Alignment - (ptr % Alignment);
					info->address = ptr + info->offset;
				}
				else
				{
					info->offset = 0;
					info->address = ptr;
				}
				DKASSERT_MEM_DEBUG((info->address % Alignment) == 0);
				Unit* units = reinterpret_cast<Unit*>(info->address);
				for (unsigned int i = 0, n = MaxUnitsPerChunk - 1; i < n; ++i)
					units[i].nextUnitIndex = i + 1;
				units[MaxUnitsPerChunk - 1].nextUnitIndex = EndOfUnits;
				info->freeUnitIndex = 0;
				info->occupied = 0;
				emptyChunks++;
				return true;
			}
			return false;
		}
		FORCEINLINE void FreeChunk(ChunkInfo* info)
		{
			DKASSERT_MEM_DEBUG(info->freeUnitIndex != EndOfUnits);
			DKASSERT_MEM_DEBUG(info->occupied == 0);

			UnitAllocator::Free(reinterpret_cast<void*>(info->address - info->offset));
			info->address = 0;

			DKASSERT_MEM_DEBUG(emptyChunks > 0);
			emptyChunks--;
		}
		FORCEINLINE uintptr_t AllocUnit(ChunkInfo* info)
		{
			if (info->freeUnitIndex != EndOfUnits)
			{
				// chunk has one or more unoccupied units.
				Unit* unit = &reinterpret_cast<Unit*>(info->address)[info->freeUnitIndex];
				info->freeUnitIndex = unit->nextUnitIndex;

				DKASSERT_MEM_DEBUG((reinterpret_cast<uintptr_t>(unit) % Alignment) == 0);

				if (info->occupied == 0)
				{
					DKASSERT_MEM_DEBUG(emptyChunks > 0);
					emptyChunks--;
				}
				info->occupied++;
				numAllocated++;

				return reinterpret_cast<uintptr_t>(unit);
			}
			return NULL;
		}
		bool IsUnitOccupied(ChunkInfo* info, int index) const
		{
			const Unit* units = reinterpret_cast<const Unit*>(info->address);
			Index i = info->freeUnitIndex;
			while (i != EndOfUnits)
			{
				if (i == index)
					return false;
				i = units[i].nextUnitIndex;
			}
			return true;
		}
		FORCEINLINE void FreeUnit(ChunkInfo* info, uintptr_t p)
		{
			DKASSERT_MEM_DEBUG(p >= info->address && p < info->address + MaxUnitsPerChunkSize);

			int index = (int)((p - info->address) / sizeof(Unit));
			DKASSERT_MEM_DEBUG(index >= 0 && index < MaxUnitsPerChunk);

			// IsUnitOccupied is slow, used only DEBUG build.
			DKASSERT_MEM_DEBUG(IsUnitOccupied(info, index));	//debug check!

			Unit* units = reinterpret_cast<Unit*>(info->address);
			units[index].nextUnitIndex = info->freeUnitIndex;
			info->freeUnitIndex = index;
			DKASSERT_MEM_DEBUG(info->occupied > 0);
			info->occupied--;

			if (info->occupied == 0)
				emptyChunks++;

			DKASSERT_MEM_DEBUG(numAllocated > 0);
			numAllocated--;
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
		FORCEINLINE ChunkInfo* FindChunkInfo(uintptr_t addr) const
		{
			uintptr_t pos = reinterpret_cast<uintptr_t>(
														std::upper_bound(&chunkTable[0], &chunkTable[numChunks], addr,
																		 [](uintptr_t lhs, const ChunkInfo& rhs)
																		 {
																			 return lhs < rhs.address;
																		 }));
			size_t index = ((pos - reinterpret_cast<uintptr_t>(&chunkTable[0])) / sizeof(ChunkInfo)) - 1;
			if (index < numChunks && addr < chunkTable[index].address + MaxUnitsPerChunkSize)
				return &chunkTable[index];
			return NULL;
		}
		FORCEINLINE bool FindChunkAndDealloc(uintptr_t addr)
		{
			ChunkInfo* info = FindChunkInfo(addr);
			if (info)
			{
				FreeUnit(info, addr);
				if (cachedChunk == NULL || cachedChunk->occupied < info->occupied)
					cachedChunk = info;
				return true;
			}
			return false;
		}
		FORCEINLINE size_t PurgeInternal(void)	// delete unoccupied chunks
		{
			if (emptyChunks > 0)
			{
				size_t numChunksPrev = numChunks;
				size_t availableChunks = 0;
				for (size_t i = 0; i < numChunks; ++i)
				{
					if (chunkTable[i].occupied == 0)
					{
						FreeChunk(&chunkTable[i]);
					}
					else
					{
						availableChunks++;
					}
				}
				DKASSERT_MEM_DEBUG(numChunks >= availableChunks);
				if (availableChunks != numChunks)
				{
					if (availableChunks > 0)
					{
						ChunkInfo* table = (ChunkInfo*)BaseAllocator::Alloc(sizeof(ChunkInfo) * availableChunks);
						if (table)
						{
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
						}
						else
						{
							// out of memory! nothing changed.
						}
					}
					else
					{
						DKASSERT_MEM_DEBUG(numAllocated == 0);
						BaseAllocator::Free(chunkTable);
						chunkTable = NULL;
						cachedChunk = NULL;
						numChunks = 0;
					}
				}
				DKASSERT_MEM_DEBUG(emptyChunks == 0);
				return (numChunksPrev - numChunks) * MaxUnitsPerChunkSize;
			}
			return 0;
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
				void* AlignedChunkAddress(void* p) const override	{ return allocator.AlignedChunkAddress(p); }


				DKMemoryLocation Location(void) const override	{ return (DKMemoryLocation)BaseAllocator::Location; }
				DKFixedSizeAllocator allocator;
			};
			static DKAllocatorChain::Maintainer init; // extend allocator life cycle.
			static AllocatorWrapper* instance = new AllocatorWrapper();
			return *instance;
		}
		
		ChunkInfo* chunkTable;
		ChunkInfo* cachedChunk;		// for fast-alloc
		size_t numAllocated;
		size_t numChunks;
		size_t emptyChunks;
		Lock lock;
	};
}
