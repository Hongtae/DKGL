//
//  File: DKMemory.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <new>

#ifdef _WIN32
#include <windows.h>
#else
#if defined(__APPLE__) && defined(__MACH__)
#include <mach/vm_statistics.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif

#include "DKMap.h"
#include "DKMemory.h"
#include "DKSpinLock.h"
#include "DKCriticalSection.h"
#include "DKLog.h"
#include "DKString.h"
#include "DKUtils.h"
#include "DKUuid.h"
#include "DKFixedSizeAllocator.h"


#define DKLog(...)	fprintf(stderr, __VA_ARGS__)

namespace DKFoundation
{
	namespace Private
	{
		struct SystemHeapAllocator
		{
#ifdef _WIN32
			static HANDLE heap;
#endif
			FORCEINLINE static void* Alloc(size_t s)
			{
#ifdef _WIN32
				return ::HeapAlloc(heap, 0, s);
#else
				return ::malloc(s);
#endif
			}
			FORCEINLINE static void* Realloc(void* p, size_t s)
			{
#ifdef _WIN32
				if (p == NULL)
					return Alloc(s);
				if (s == 0)
				{
					Free(p);
					return NULL;
				}
				return ::HeapReAlloc(heap, 0, p, s);
#else
				return ::realloc(p, s);
#endif
			}
			FORCEINLINE static void Free(void* p)
			{
#ifdef _WIN32
				::HeapFree(heap, 0, p);
#else
				return ::free(p);
#endif
			}
		};
#ifdef _WIN32
		HANDLE SystemHeapAllocator::heap;
#endif

		using SystemLargeHeapAllocator = DKMemoryVirtualAllocator;

		// BackendAllocator : allocates all front-end allocators chunks.
		struct BackendAllocator
		{
			enum { UnitSize = (1 << 16) };
			enum { IndexTableAlignment = 64 };
			static_assert((IndexTableAlignment & (IndexTableAlignment-1)) == 0, "Alignment should be power of two.");
			using Allocator = DKFixedSizeAllocator<UnitSize, 1, 64, DKDummyLock, SystemHeapAllocator, SystemLargeHeapAllocator>;

			static BackendAllocator* Instance();	// init by main allocator. (AllocatorPool)

			using Index = short;
			enum { IndexNotFound = (Index)-1 };

#pragma pack(push, 2)
			struct IndexedAddress
			{
				uintptr_t address;
				Index index;
			};
#pragma pack(pop)

			void* AllocWithIndex(Index index)
			{
				ScopedLock guard(lock);
				void* p = allocator.Alloc(UnitSize);
				if (p)
				{
					if (indexAddrsCapacity < numIndexAddrs + 1)
					{
						size_t caps = (indexAddrsCapacity + IndexTableAlignment) & ~(IndexTableAlignment - 1);
						DKASSERT_MEM_DEBUG(caps >= (numIndexAddrs + 1));
						IndexedAddress* tmp = (IndexedAddress*)SystemHeapAllocator::Realloc(indexAddrs, sizeof(IndexedAddress) * caps);
						if (tmp)
						{
							indexAddrs = tmp;
							indexAddrsCapacity = caps;
						}
						else // out of memory!?
						{
							allocator.Dealloc(p);
							return NULL;
						}
					}

					uintptr_t baseAddr = reinterpret_cast<uintptr_t>(p);
					size_t addrIndex = 0;
					if (numIndexAddrs > 0)
					{
						addrIndex = FindAddress(baseAddr) + 1;
						if (addrIndex < numIndexAddrs)
						{
#if 1
							memmove(&indexAddrs[addrIndex + 1], &indexAddrs[addrIndex], sizeof(IndexedAddress) * (numIndexAddrs - addrIndex));
#else
							for (size_t i = numIndexAddrs; i > addrIndex; --i)
								indexAddrs[i] = indexAddrs[i - 1];
#endif
						}
					}
					indexAddrs[addrIndex].index = index;
					indexAddrs[addrIndex].address = baseAddr;
					numIndexAddrs++;
				}
				return p;
			}
			Index Dealloc(void* p)
			{
				ScopedLock guard(lock);
				DKASSERT_MEM_DEBUG(numIndexAddrs > 0);
				size_t addrIndex = FindAddress(reinterpret_cast<uintptr_t>(p));
				DKASSERT_MEM_DEBUG(addrIndex < numIndexAddrs);
				Index index = indexAddrs[addrIndex].index;
				numIndexAddrs--;
				if (numIndexAddrs > 0)
				{
#if 1
					if (addrIndex < numIndexAddrs)
						memmove(&indexAddrs[addrIndex], &indexAddrs[addrIndex + 1], sizeof(IndexedAddress) * (numIndexAddrs - addrIndex));
#else
					for (size_t i = addrIndex; i < numIndexAddrs; ++i)
						indexAddrs[i] = indexAddrs[i+1];
#endif
				}
				allocator.Dealloc(p);
				return index;
			}
			Index IndexForAddress(void* p)
			{
				uintptr_t addr = reinterpret_cast<uintptr_t>(p);
				ScopedLock guard(lock);
				size_t addrIndex = FindAddress(addr);
				if (addrIndex < numIndexAddrs && addr < indexAddrs[addrIndex].address + UnitSize)
					return indexAddrs[addrIndex].index;
				return IndexNotFound;
			}
			size_t PurgeThreshold(size_t threshold)
			{
				ScopedLock guard(lock);
				size_t r = allocator.ConditionalPurge(threshold);
				if (r > 0 || threshold == 0)
				{
					size_t alignedNumIndexAddrs = (numIndexAddrs + IndexTableAlignment - 1) & ~(IndexTableAlignment - 1);
					if (alignedNumIndexAddrs < indexAddrsCapacity)
					{
						if (numIndexAddrs > 0)
						{
							size_t caps = alignedNumIndexAddrs;
							DKASSERT_MEM_DEBUG((caps % IndexTableAlignment) == 0);
							IndexedAddress* tmp = (IndexedAddress*)SystemHeapAllocator::Realloc(indexAddrs, sizeof(IndexedAddress) * caps);
							if (tmp)
							{
								indexAddrs = tmp;
								indexAddrsCapacity = caps;
							}
							else
							{
								// out of memory! nothing changed.
							}
						}
						else
						{
							SystemHeapAllocator::Free(indexAddrs);
							indexAddrs = NULL;
							indexAddrsCapacity = 0;
						}
					}
				}
				return r;
			}
			size_t Size(void) const
			{
				ScopedLock guard(lock);
				return (sizeof(IndexedAddress) * indexAddrsCapacity) + allocator.Size();
			}
			BackendAllocator(void)
				: numIndexAddrs(0)
				, indexAddrsCapacity(0)
				, indexAddrs(NULL)
			{
			}
			~BackendAllocator(void)
			{
				if (indexAddrsCapacity > 0)
					SystemHeapAllocator::Free(indexAddrs);
			}
		private:
			FORCEINLINE size_t FindAddress(uintptr_t addr)
			{
				// lower-bound search.
				size_t start = 0;
				size_t count = numIndexAddrs;
				size_t med;
				while (count > 1)
				{
					med = count >> 1;
					if (addr < indexAddrs[start + med].address)
					{
						count = med;
					}
					else
					{
						start += med;
						count -= med;
					}
				}
				if (count > 0 && addr >= indexAddrs[start].address)
					return start;
				return (size_t)-1;
			}

			using Lock = DKSpinLock;
			using ScopedLock = DKCriticalSection<Lock>;
			Lock			lock;
			Allocator		allocator;
			IndexedAddress*	indexAddrs;
			size_t			indexAddrsCapacity;
			size_t			numIndexAddrs;
		};


		// ChunkAllocator : allocates chunks for front-end allocators.
		//   This is wrapper of BackendAllocator and keep track address that
		//   was allocated by BackendAllocator.
		template <int Index> struct ChunkAllocator
		{
			enum { UnitSize = BackendAllocator::UnitSize };

			FORCEINLINE static void* Alloc(size_t s)
			{
				DKASSERT_MEM_DEBUG(s <= UnitSize);
				return BackendAllocator::Instance()->AllocWithIndex(Index);
			}
			FORCEINLINE static void Free(void* p)
			{
				int index = BackendAllocator::Instance()->Dealloc(p);
				DKASSERT_MEM_DEBUG(index == Index);
			}
		};

		struct AllocatorInterface
		{
			virtual ~AllocatorInterface(void) noexcept(!DKGL_MEMORY_DEBUG) {}
			virtual void* Alloc(size_t) = 0;
			virtual void Dealloc(void*) = 0;
			virtual size_t Purge(void) = 0;
			virtual void Reserve(size_t) = 0;
			virtual void* AlignedChunkAddress(void*) const = 0;
			virtual size_t ChunkSize(void) const = 0;

			virtual bool ConditionalDealloc(void*) = 0;
			virtual size_t ConditionalPurge(size_t) = 0;
			virtual bool ConditionalDeallocAndPurge(void*, size_t, size_t*) = 0;

			virtual size_t NumberOfAllocatedUnits(void) const = 0;
		};

		struct AllocatorUnit
		{
			size_t unitSize;
			AllocatorInterface* allocator;
		};

		template <
			int Size,
			int SizeOffset,
			int Alignment,
			int Index,
			int Count
		>
		struct Initializer
		{
			using UnitAllocator = ChunkAllocator<Index>;

			enum { UnitSize = Size };
			enum { MaxChunkSize = UnitAllocator::UnitSize };
			enum { NumUnits = (MaxChunkSize - Alignment + 1) / UnitSize };
			static_assert((UnitSize % Alignment) == 0, "Wrong unit size!");
			static_assert((UnitSize % 16) == 0, "Invalid size, size must be aligned with 16bytes");

			struct Wrapper : public AllocatorInterface
			{
				void* Alloc(size_t s) override						{ return allocator.Alloc(s); }
				void Dealloc(void* p) override						{ return allocator.Dealloc(p); }
				size_t Purge(void) override							{ return allocator.Purge(); }
				void Reserve(size_t s) override						{ return allocator.Reserve(s); }
				void* AlignedChunkAddress(void* p) const override	{ return allocator.AlignedChunkAddress(p); }
				size_t ChunkSize(void) const override				{ return allocator.AlignedChunkSize; }

				bool ConditionalDealloc(void* p) override			{ return allocator.ConditionalDealloc(p); }
				size_t ConditionalPurge(size_t c) override			{ return allocator.ConditionalPurge(c); }
				bool ConditionalDeallocAndPurge(void* p, size_t s, size_t* bp) override
				{
					return allocator.ConditionalDeallocAndPurge(p, s, bp);
				}

				size_t NumberOfAllocatedUnits(void) const override	{ return allocator.NumberOfAllocatedUnits(); }

				using Allocator = DKFixedSizeAllocator<UnitSize, Alignment, NumUnits, DKSpinLock, SystemHeapAllocator, UnitAllocator>;
				Allocator allocator;
				static_assert(Allocator::AlignedChunkSize <= MaxChunkSize, "Wrong size!");
			};

			static int Init(AllocatorUnit* units)
			{
				DKLog("Allocator[%d]: (size:%d, alignment:%d, units:%d, chunkSize:%d/%d usage:%.2f%%)\n",
					  Index, UnitSize, Alignment, NumUnits, Wrapper::Allocator::AlignedChunkSize, MaxChunkSize,
					  ((double)Wrapper::Allocator::AlignedChunkSize / (double)MaxChunkSize) * 100.0);
				units[Index].unitSize = UnitSize;
				units[Index].allocator = ::new (SystemHeapAllocator::Alloc(sizeof(Wrapper))) Wrapper();
				return 1 + Initializer<UnitSize + SizeOffset, SizeOffset, Alignment, Index+1, Count-1>::Init(units);
			}
		};
		template <int Size, int SizeOffset, int Alignment, int Index>
		struct Initializer<Size, SizeOffset, Alignment, Index, 0> //sentinel
		{
			static int Init(AllocatorUnit*) { return 0; }
		};

		struct AllocatorPool : public DKAllocator
		{
			enum { NumAllocators = 136 };

			AllocatorPool(void) : backend(NULL)
			{
#ifdef _WIN32
				// reserve 16MB heap
				SystemHeapAllocator::heap = ::HeapCreate(0, (1<<24), 0);
#endif
				backend = ::new (SystemHeapAllocator::Alloc(sizeof(BackendAllocator))) BackendAllocator();

				// Initializer < Size, SizeOffset, Alignment, Index, Count>

				int count = 0;
				// 16 ~ 1024 (16 bytes offsets)
				count += Initializer< (1 << 4), 16, 1, 0, 64>::Init(allocators);
				// 1088 ~ 4096 (64 bytes offsets)
				count += Initializer< (1 << 10) + 64, 64, 1, 64, 48>::Init(allocators);
				// 4532 ~ 8192 (256 bytes offsets)
				count += Initializer< (1 << 12) + 256, 256, 1, 112, 16>::Init(allocators);
				// 9216 ~ 16384 (1024 bytes offset)
				count += Initializer< (1 << 13) + 1024, 1024, 1, 128, 8>::Init(allocators);

				DKASSERT_MEM_DEBUG(count == NumAllocators);

				size_t chunkSize = 0;
				for (int i = 0; i < count; ++i )
				{
					chunkSize += allocators[i].allocator->ChunkSize();
				}

				DKLog("AllocatorPool Initialized. (%lu - %lu, Units: %d, ChunkSize: %lu)\n",
					  allocators[0].unitSize,
					  allocators[NumAllocators-1].unitSize,
					  NumAllocators,
					  chunkSize);

				maxUnitSize = allocators[NumAllocators-1].unitSize;
			}

			~AllocatorPool(void)
			{
				bool cleanupHeap = true;
				for (int i = 0; i < NumAllocators; ++i)
				{
					size_t numAllocated = allocators[i].allocator->NumberOfAllocatedUnits();
					if ( numAllocated > 0)
					{
						DKLog("MEMORY LEAK WARNING: %lu objects (%d bytes unit) still occupied.\n",
							  numAllocated, (int)allocators[i].unitSize);
						cleanupHeap = false;
					}
					else
					{
						allocators[i].allocator->~AllocatorInterface();
					}

					SystemHeapAllocator::Free(allocators[i].allocator);
				}

				backend->PurgeThreshold(0);

				if (cleanupHeap)
				{
					backend->~BackendAllocator();
					SystemHeapAllocator::Free(backend);

#ifdef _WIN32
					::HeapDestroy(SystemHeapAllocator::heap);
#endif
				}
				else
				{
					DKLog("Warning: Cleaning Memory-Pool has been cancelled. (Leak detected)\n");
				}
			}

			void* Alloc(size_t s)
			{
				if (s > this->maxUnitSize)
					return SystemLargeHeapAllocator::Alloc(s);

				AllocatorUnit* unit = FindAllocatorForSize(s);
				DKASSERT_MEM_DEBUG(unit != NULL);
				DKASSERT_MEM_DEBUG(unit->unitSize >= s);
				return unit->allocator->Alloc(s);
			}

			void* Realloc(void* p, size_t s)
			{
				if (p)
				{
					AllocatorUnit* unit = FindAllocator(p);
					if (unit)
					{
						void* p2 = NULL;
						if (s > this->maxUnitSize)
						{
							p2 = SystemLargeHeapAllocator::Alloc(s);
						}
						else
						{
							AllocatorUnit* unit2 = FindAllocatorForSize(s);
							if (unit2 == unit)
								return p;
							p2 = unit2->allocator->Alloc(s);
						}
						if (p2)
						{
							size_t bytesToCopy = Min(s, unit->unitSize);
							memcpy(p2, p, bytesToCopy);
							if (!DeallocAndPurge(unit, p))
							{
								DKASSERT_MEM_DEBUG(0);
							}
						}
						return p2;
					}
					else // allocated from SystemLargeHeapAllocator.
					{
						if (s > this->maxUnitSize)
						{
							return SystemLargeHeapAllocator::Realloc(p, s);
						}
						else
						{
							unit = FindAllocatorForSize(s);
							DKASSERT_MEM_DEBUG(unit);
							void* p2 = unit->allocator->Alloc(s);
							if (p2)
							{
								memcpy(p2, p, s);
								SystemLargeHeapAllocator::Free(p);
							}
							return p2;
						}
					}
				}
				return NULL;
			}

			void Dealloc(void* p)
			{
				if (p)
				{
					AllocatorUnit* unit = FindAllocator(p);
					if (unit)
					{
						if (!DeallocAndPurge(unit, p))
						{
							DKASSERT_MEM_DEBUG(0);
						}
						return;
					}

					SystemLargeHeapAllocator::Free(p);
				}
			}

			size_t Purge(void)
			{
				size_t bytesPurged = 0;
				for (int i = 0; i < NumAllocators; ++i)
				{
					bytesPurged += allocators[i].allocator->ConditionalPurge(0);
				}
				if (bytesPurged > 0)
				{
					return backend->PurgeThreshold(0);
				}
				return 0;
			}

			FORCEINLINE size_t Size(void) const
			{
				return backend->Size();
			}

			FORCEINLINE DKMemoryLocation Location(void) const
			{
				return DKMemoryLocationPool;
			}

			FORCEINLINE BackendAllocator* Backend(void)
			{
				return backend;
			}

		private:
			FORCEINLINE bool DeallocAndPurge(AllocatorUnit* unit, void* p)
			{
				DKASSERT_MEM_DEBUG(unit);
				DKASSERT_MEM_DEBUG(p);

				//size_t threshold = BackendAllocator::UnitSize / (unit->unitSize);
				size_t threshold = 0;
				size_t purged = 0;
				if (unit->allocator->ConditionalDeallocAndPurge(p, threshold, &purged))
				{
					if (purged > 0)
						backend->PurgeThreshold(16);
					return true;
				}
				return false;
			}
			FORCEINLINE AllocatorUnit* FindAllocatorForSize(size_t size)
			{
				size_t count = NumAllocators;
				size_t start = 0;
				size_t med;
				while (count > 0)
				{
					med = count / 2;
					if (size > allocators[start + med].unitSize)
					{
						start += med + 1;
						count -= med + 1;
					}
					else
					{
						count = med;
					}
				}
				return &allocators[start];
			}
			FORCEINLINE AllocatorUnit* FindAllocator(void* p)
			{
				BackendAllocator::Index index = backend->IndexForAddress(p);
				if (index != BackendAllocator::IndexNotFound)
					return &allocators[index];
				return NULL;
			}

			BackendAllocator* backend;
			AllocatorUnit allocators[NumAllocators];
			size_t maxUnitSize;
		};

		AllocatorPool* GetAllocatorPool(void)
		{
			static DKAllocatorChain::Maintainer init;

			static DKSpinLock lock;
			static AllocatorPool* pool = NULL;
			if (pool == NULL)
			{
				lock.Lock();
				if (pool == NULL)
				{
					// It will be destroyed automatically.
					// see DKAllocatorChain.cpp
					pool = new AllocatorPool();
				}
				lock.Unlock();
			}
			return pool;
		}

		BackendAllocator* BackendAllocator::Instance()
		{
			return GetAllocatorPool()->Backend();
		}

		// VMSizeInfo : keep track VM-address, size pair.
		struct VMSizeInfo
		{
			enum { SizeOffset = 64 };
			using Index = size_t;
			enum : Index { IndexNotFound = (Index)-1 };

			FORCEINLINE static bool Set(void* p, size_t s)
			{
				DKASSERT_MEM_DEBUG(p != NULL);
				DKASSERT_MEM_DEBUG(s > 0);

				ScopedLock guard(Lock());
				VMTable& table = Table();
				Index index = 0;
				if (table.count > 0)
				{
					uintptr_t addr = reinterpret_cast<uintptr_t>(
																 std::upper_bound(&table.data[0],
																				  &table.data[table.count],
																				  reinterpret_cast<uintptr_t>(p),
																				  [](uintptr_t lhs, const Info& rhs)
																				  {
																					  return lhs < rhs.addr;
																				  }));
					index = (addr - reinterpret_cast<uintptr_t>(&table.data[0])) / sizeof(Info);
				}
				DKASSERT_MEM_DEBUG(index <= table.count);
#if DKGL_MEMORY_DEBUG
				if (index < table.count)
				{
					if (table.data[index].addr <= reinterpret_cast<uintptr_t>(p))
						return false;	// duplicated!
				}
#endif
				if (table.capacity <= table.count + 1)
				{
					size_t cap = table.capacity + SizeOffset;
					Info* p = (Info*)::realloc(table.data, cap * sizeof(Info));
					if (p == NULL)		// out of memory.
						return false;
					table.capacity = cap;
					table.data = p;
				}
				if (index < table.count)
				{
					memmove(&table.data[index+1], &table.data[index], sizeof(Info) * (table.count - index));
				}
				table.data[index].addr = reinterpret_cast<uintptr_t>(p);
				table.data[index].size = s;
				table.count++;
				return true;
			}
			FORCEINLINE static bool Update(void* p, size_t s, size_t* old)
			{
				ScopedLock guard(Lock());
				VMTable& table = Table();
				Index index = Find(&table, reinterpret_cast<uintptr_t>(p));
				if (index == IndexNotFound)
					return false;
				DKASSERT_MEM_DEBUG(table.data[index].addr == reinterpret_cast<uintptr_t>(p));
				size_t size = table.data[index].size;
				table.data[index].size = s;
				if (old)
					*old = size;
				return true;
			}
			FORCEINLINE static bool Unset(void* p)
			{
				ScopedLock guard(Lock());
				VMTable& table = Table();
				Index index = Find(&table, reinterpret_cast<uintptr_t>(p));
				if (index == IndexNotFound)
					return false;
				DKASSERT_MEM_DEBUG(table.data[index].addr == reinterpret_cast<uintptr_t>(p));
				table.count--;
				if (table.count > 0)
				{
					if (index < table.count)
						memmove(&table.data[index], &table.data[index+1], sizeof(Info) * (table.count - index));
				}
				else
				{
					::free(table.data);
					table.data = NULL;
					table.capacity = 0;
				}
				return true;
			}
			FORCEINLINE static size_t Size(void* p)
			{
				ScopedLock guard(Lock());
				VMTable& table = Table();
				Index index = Find(&table, reinterpret_cast<uintptr_t>(p));
				if (index != IndexNotFound)
					return table.data[index].size;
				return 0;
			}
		private:
			struct Info
			{
				uintptr_t addr;
				size_t size;
			};
			struct VMTable
			{
				Info* data;
				size_t count;
				size_t capacity;
			};
			static VMTable& Table(void)
			{
				static VMTable table = {0, 0, 0};
				return table;
			};
			static DKSpinLock& Lock(void)
			{
				static DKSpinLock lock;
				return lock;
			}
			FORCEINLINE static Index Find(VMTable* table, uintptr_t addr)
			{
				Index start = 0;
				size_t count = table->count;
				size_t mid;
				while (count > 0)
				{
					mid = count / 2;
					Info& info = table->data[start+mid];
					if (addr > info.addr)
					{
						start += mid+1;
						count -= mid+1;
					}
					else if (addr < info.addr)
						count = mid;
					else
						return (start + mid);
				}
				return IndexNotFound;
			}

			using ScopedLock = DKCriticalSection<DKSpinLock>;
		};

#ifdef _WIN32
		static std::wstring Win32GetErrorString(DWORD dwError)
		{
			std::wstring ret = L"";
			// error!
			LPVOID lpMsgBuf;
			::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
							 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);

			ret = (const wchar_t*)lpMsgBuf;
			::LocalFree(lpMsgBuf);
			return ret;
		}
#endif
	}


	using namespace Private;


	DKGL_API void* DKMemoryHeapAlloc(size_t s)
	{
#ifdef _WIN32
		return ::HeapAlloc(GetProcessHeap(), 0, s);
#else
		return ::malloc(s);
#endif
	}

	DKGL_API void* DKMemoryHeapRealloc(void* p, size_t s)
	{
#ifdef _WIN32
		if (p == NULL)
			return DKMemoryHeapAlloc(s);
		if (s == 0)
		{
			DKMemoryHeapFree(p);
			return NULL;
		}
		return ::HeapReAlloc(GetProcessHeap(), 0, p, s);
#else
		return ::realloc(p, s);
#endif
	}

	DKGL_API void  DKMemoryHeapFree(void* p)
	{
#ifdef _WIN32
		::HeapFree(GetProcessHeap(), 0, p);
#else
		return ::free(p);
#endif
	}

	// virtual-address, can commit, decommit.
	// data will be erased when decommit.
	DKGL_API void* DKMemoryVirtualAlloc(size_t s)
	{
		void* p = NULL;

		size_t pageSize = DKMemoryPageSize();
		DKASSERT_MEM_DEBUG(pageSize != 0);

		if (s == 0)
			s = pageSize;
		else if (s % pageSize)
			s += pageSize - (s % pageSize);

		DKASSERT_MEM_DEBUG((s % pageSize) == 0);

#ifdef _WIN32
		p = ::VirtualAlloc(0, s, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (p)
		{
			DKASSERT_MEM_DEBUG(VMSizeInfo::Set(p, s));
		}
#else
		p = ::mmap(0, s, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		if (p == MAP_FAILED)
		{
			DKLog("mmap failed: %s\n", strerror(errno));
			p = NULL;
		}
		else
		{
			if (!VMSizeInfo::Set(p, s))
			{
				//DKASSERT_MEM_DESC_DEBUG(0, "VMSizeInfo::Set failed.");
				DKLog("VMSizeInfo::Set failed.\n");
				// unmap.
				if (::munmap(p, s) != 0)
				{
					DKASSERT_MEM_DESC_DEBUG(0, "munmap failed");
					DKLog("munmap failed: %s\n", strerror(errno));
				}
				p = NULL;
			}
		}
#endif
		return p;
	}

	DKGL_API void* DKMemoryVirtualRealloc(void* p, size_t s)
	{
		if (p && s)
		{
			size_t pageSize = DKMemoryPageSize();
			DKASSERT_MEM_DEBUG(pageSize != 0);
			size_t alignedSize = s;
			if (s % pageSize)
				alignedSize += pageSize - (s % pageSize);

#ifdef _WIN32
			MEMORY_BASIC_INFORMATION memInfo;
			if (VirtualQuery(p, &memInfo, sizeof(memInfo)))
			{
				size_t pageSize = DKMemoryPageSize();
				DKASSERT_MEM_DEBUG(pageSize != 0);

				if (alignedSize != memInfo.RegionSize)
				{
					void* p2 = ::VirtualAlloc(0, s, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
					if (p2)
					{
						memcpy(p2, p, (alignedSize > memInfo.RegionSize ? memInfo.RegionSize : s));
						::VirtualFree(p, 0, MEM_RELEASE);
						DKASSERT_MEM_DEBUG(VMSizeInfo::Unset(p));
						DKASSERT_MEM_DEBUG(VMSizeInfo::Set(p2, alignedSize));
					}
					p = p2;
				}
			}
			else
			{
				DKASSERT_MEM_DESC_DEBUG(0, "VirtualQuery failed.");
				DKLog("VirtualQuery failed:%ls\n", Win32GetErrorString(::GetLastError()).c_str());
				p = NULL;
			}
#else
			size_t sizeOrig = VMSizeInfo::Size(p);
			DKASSERT_MEM_DESC_DEBUG(sizeOrig > 0, "Invalid address.");
			if (sizeOrig == alignedSize)
			{
				return p; // no change.
			}
			else if (sizeOrig > alignedSize)			// shrink
			{
				uintptr_t p2 = reinterpret_cast<uintptr_t>(p) + alignedSize;
				size_t len = sizeOrig - alignedSize;
				DKLog("munmap(%lu, %lu)\n", p2, len);
				if (::munmap(reinterpret_cast<void*>(p2), len) != 0)
				{
					DKLog("munmap failed: %s\n", strerror(errno));
				}
				else
				{
					VMSizeInfo::Update(p, alignedSize, NULL);
					return p;
				}
			}
			else	// expand
			{
				uintptr_t p2 = reinterpret_cast<uintptr_t>(p) + sizeOrig;
				size_t len = alignedSize - sizeOrig;
				// mmap on p2 with len length.
				// recall mmap for entire region and copy if failed.
				void* p3 = ::mmap(reinterpret_cast<void*>(p2), len, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
				if (p3 == MAP_FAILED)
				{
					DKLog("mmap failed: %s\n", strerror(errno));
					// failed, mmap for entire region and copy!
				}
				else
				{
					if (p2 == reinterpret_cast<uintptr_t>(p3))
					{
						// mmap returned good position!
						VMSizeInfo::Update(p, alignedSize, NULL);
						return p;
					}
					else
					{
						DKLog("mmap returns unwanted location(%lu != %p). unmap and realloc\n", p2, p3);
						if (::munmap(p3, len) != 0)
						{
							DKLog("munmap failed: %s\n", strerror(errno));
						}
					}
				}
			}
			// mmap with specified range and copy.
			void* p2 = ::mmap(0, alignedSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
			if (p2 == MAP_FAILED)
			{
				DKLog("mmap failed: %s\n", strerror(errno));
				p = NULL;
			}
			else
			{
				VMSizeInfo::Set(p2, alignedSize);

				size_t bytesCopy = Min(sizeOrig, alignedSize);
				::memcpy(p2, p, bytesCopy);
				p = p2;

				if (::munmap(p, sizeOrig) != 0)
				{
					DKLog("munmap failed: %s\n", strerror(errno));
				}
				VMSizeInfo::Unset(p);
				p = p2;
			}
#endif
			return p;
		}
		else if (p == NULL)
		{
			return DKMemoryVirtualAlloc(s);
		}
		else if (s == 0)
		{
			DKMemoryVirtualFree(p);
			return NULL;
		}
		return NULL;
	}

	DKGL_API void  DKMemoryVirtualFree(void* p)
	{
		if (p)
		{
#ifdef _WIN32
			if (::VirtualFree(p, 0, MEM_RELEASE))
			{
				DKASSERT_MEM_DEBUG(VMSizeInfo::Unset(p));
			}
			else
			{
				DKASSERT_MEM_DESC_DEBUG(0, "VirtualFree failed");
				DKLog("VirtualFree failed:%ls\n", Win32GetErrorString(::GetLastError()).c_str());
			}
#else
			size_t s = VMSizeInfo::Size(p);
			DKASSERT_MEM_DESC_DEBUG(s > 0, "Unallocated address.");
			if (s > 0)
			{
				DKASSERT_MEM_DEBUG((s % getpagesize()) == 0);
				if (::munmap(p, s) != 0)
				{
					DKLog("munmap failed: %s\n", strerror(errno));
				}
				VMSizeInfo::Unset(p);
			}
#endif
		}
	}

	DKGL_API size_t  DKMemoryVirtualSize(void* p)
	{
		if (p)
		{
#ifdef _WIN32
			MEMORY_BASIC_INFORMATION memInfo;
			if (VirtualQuery(p, &memInfo, sizeof(memInfo)))
			{
				DKASSERT_MEM_DEBUG(VMSizeInfo::Size(p) == memInfo.RegionSize);
				return memInfo.RegionSize;
			}
			else
			{
				DKASSERT_MEM_DESC_DEBUG(0, "VirtualQuery failed.");
				DKLog("VirtualQuery failed:%ls\n", Win32GetErrorString(::GetLastError()).c_str());
			}
#else
			return VMSizeInfo::Size(p);
#endif
		}
		return 0;
	}

	// system-paing functions.
	DKGL_API size_t DKMemoryPageSize(void)
	{
		static size_t pageSize = [](){
#ifdef _WIN32
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			return sysInfo.dwPageSize;
#else
			return getpagesize();
#endif
		}();
		return pageSize;
	}

	DKGL_API void* DKMemoryPageReserve(void* p, size_t s)
	{
		void* ptr = NULL;
		size_t pageSize = DKMemoryPageSize();
		DKASSERT_MEM_DEBUG(pageSize != 0);

		if (s == 0)
			s = pageSize;
		else if (s % pageSize)
			s += pageSize - (s % pageSize);

		DKASSERT_MEM_DEBUG((s % pageSize) == 0);
#ifdef _WIN32
		ptr = ::VirtualAlloc(p, s, MEM_RESERVE, PAGE_READWRITE);
		if (ptr == NULL)
			ptr = ::VirtualAlloc(0, s, MEM_RESERVE, PAGE_READWRITE);

		if (ptr)
		{
			DKASSERT_MEM_DEBUG(VMSizeInfo::Set(ptr, s));
		}

#else
		// mmap
		ptr = ::mmap(0, s, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (ptr == MAP_FAILED)
		{
			DKLog("mmap failed: %s\n", strerror(errno));
			ptr = NULL;
		}
		if (ptr)
		{
			if (!VMSizeInfo::Set(ptr, s))
			{
				DKASSERT_MEM_DESC_DEBUG(0, "VMSizeInfo::Set failed!");
			}
		}
#endif
		return ptr;
	}

	DKGL_API void DKMemoryPageRelease(void* p)
	{
		if (p)
		{
#ifdef _WIN32
#if DKGL_MEMORY_DEBUG
			MEMORY_BASIC_INFORMATION mbi;
			if (::VirtualQuery(p, &mbi, sizeof(mbi)))
			{
				DKASSERT_MEM_DEBUG(mbi.RegionSize == VMSizeInfo::Size(p));
			}
			else
			{
				DKASSERT_MEM_DESC_DEBUG(0, "VirtualQuery failed.");
				DKLog("VirtualQuery failed:%ls\n", Win32GetErrorString(::GetLastError()).c_str());
			}
#endif
			if (::VirtualFree(p, 0, MEM_RELEASE))
			{
				DKASSERT_MEM_DEBUG(VMSizeInfo::Unset(p));
			}
			else
			{
				DKASSERT_MEM_DESC_DEBUG(0, "VirtualFree failed");
				DKLog("VirtualFree failed:%ls\n", Win32GetErrorString(::GetLastError()).c_str());
			}
#else
			size_t s = VMSizeInfo::Size(p);
			if (s > 0)
			{
				//mumap..
				if (::munmap(p, s) != 0)
				{
					DKASSERT_MEM_DESC_DEBUG(0, "munmap failed");
					DKLog("munmap failed:%s\n", strerror(errno));
				}
				VMSizeInfo::Unset(p);
			}
			else
			{
				DKASSERT_MEM_DESC_DEBUG(0, "Unallocated address");
			}
#endif
		}
	}

	DKGL_API void DKMemoryPageCommit(void* p, size_t s)
	{
		if (p && s > 0)
		{
			size_t pageSize = DKMemoryPageSize();
			DKASSERT_MEM_DEBUG(pageSize != 0);

			s = Max(s, pageSize);

			if (s % pageSize)
				s += pageSize - (s % pageSize);

			DKASSERT_MEM_DEBUG((s % pageSize) == 0);

#ifdef _WIN32
#if DKGL_MEMORY_DEBUG
			MEMORY_BASIC_INFORMATION mbi;
			if (::VirtualQuery(p, &mbi, sizeof(mbi)))
			{
				size_t beginAddr = reinterpret_cast<size_t>(mbi.BaseAddress);
				size_t endAddr = reinterpret_cast<size_t>(mbi.BaseAddress) + mbi.RegionSize;

				DKASSERT_MEM_DEBUG(reinterpret_cast<size_t>(p) >= beginAddr);
				DKASSERT_MEM_DEBUG(reinterpret_cast<size_t>(p) <= endAddr);
				DKASSERT_MEM_DEBUG(reinterpret_cast<size_t>(p) + s >= beginAddr);
				DKASSERT_MEM_DEBUG(reinterpret_cast<size_t>(p) + s <= endAddr);
			}
#endif
			void* baseAddr = ::VirtualAlloc(p, s, MEM_COMMIT, PAGE_READWRITE);
			if (baseAddr == NULL)
			{
				DKERROR_THROW_DEBUG("VirtualAlloc failed");
				DKLog("VirtualAlloc failed:%ls\n", Win32GetErrorString(::GetLastError()).c_str());
			}
#else
			while (mprotect(p, s, PROT_READ|PROT_WRITE) == -1)
			{
				DKLog("madvice failed:%s\n", strerror(errno));
				if (errno != EAGAIN)
					break;
			}
			while (madvise(p, s, MADV_WILLNEED) == -1)
			{
				DKLog("madvice failed:%s\n", strerror(errno));
				if (errno != EAGAIN)
					break;
			}
#endif
		}
	}

	DKGL_API void DKMemoryPageDecommit(void* p, size_t s)
	{
		if (p && s > 0)
		{
			size_t pageSize = DKMemoryPageSize();
			DKASSERT_MEM_DEBUG(pageSize != 0);

			s = Max(s, pageSize);

			if (s % pageSize)
				s += pageSize - (s % pageSize);

			DKASSERT_MEM_DEBUG((s % pageSize) == 0);

#ifdef _WIN32
			if (::VirtualFree(p, s, MEM_DECOMMIT) == 0)
			{
				DKERROR_THROW_DEBUG("VirtualFree failed");
				DKLog("VirtualFree failed:%ls\n", Win32GetErrorString(::GetLastError()).c_str());
			}
#else
			while (madvise(p, s, MADV_DONTNEED) == -1)
			{
				DKLog("madvice failed:%s\n", strerror(errno));
				if (errno != EAGAIN)
					break;
			}
			while (mprotect(p, s, PROT_NONE) == -1)
			{
				DKLog("mprotect failed:%s\n", strerror(errno));
				if (errno != EAGAIN)
					break;
			}
#endif
		}
	}

	DKGL_API void* DKMemoryPoolAlloc(size_t s)
	{
		return GetAllocatorPool()->Alloc(s);
	}

	DKGL_API void* DKMemoryPoolRealloc(void* p, size_t s)
	{
		if (p && s)
		{
			return GetAllocatorPool()->Realloc(p, s);
		}
		else if (p == NULL)
		{
			return GetAllocatorPool()->Alloc(s);
		}
		else if (s == 0)
		{
			GetAllocatorPool()->Dealloc(p);
		}
		return NULL;
	}
	
	DKGL_API void DKMemoryPoolFree(void* p)
	{
		GetAllocatorPool()->Dealloc(p);
	}
	
	DKGL_API size_t DKMemoryPoolPurge(void)
	{
		return GetAllocatorPool()->Purge();
	}
	
	DKGL_API size_t DKMemoryPoolSize(void)
	{
		return GetAllocatorPool()->Size();
	}
}
