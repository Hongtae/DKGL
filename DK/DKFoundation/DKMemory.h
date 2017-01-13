//
//  File: DKMemory.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// allocate memory from system heap.
	DKGL_API void* DKMemoryHeapAlloc(size_t);
	/// resize memory allocated by DKMemoryHeapAlloc
	DKGL_API void* DKMemoryHeapRealloc(void*, size_t);
	/// release memory allocated by DKMemoryHeapAlloc
	DKGL_API void  DKMemoryHeapFree(void*);

	/// allocate memory from system VM.
	DKGL_API void* DKMemoryVirtualAlloc(size_t);
	/// resize memory allocated by DKMemoryVirtualAlloc
	DKGL_API void* DKMemoryVirtualRealloc(void*, size_t);
	/// release memory allocated by DKMemoryVirtualAlloc
	DKGL_API void  DKMemoryVirtualFree(void*);
	/// query allocation size of memory allocated by DKMemoryVirtualAlloc
	DKGL_API size_t  DKMemoryVirtualSize(void*);

	/// query system page size (default allocation size)
	DKGL_API size_t DKMemoryPageSize(void);
	/// reserve page memory with given location if available.
	/// address should be aligned with value returned by DKMemoryPageSize or NULL
	DKGL_API void* DKMemoryPageReserve(void*, size_t);
	/// release memory reserved by DKMemoryPageReserve
	/// or decommitted by DKMemoryPageDecommit
	DKGL_API void DKMemoryPageRelease(void*);
	/// commit virtual memory reserved by DKMemoryPageReserve
	DKGL_API void DKMemoryPageCommit(void*, size_t);
	/// decommit virtual memory committed by DKMemoryPageCommit
	DKGL_API void DKMemoryPageDecommit(void*, size_t);

	/// allocate memory from memory pool
	/// If the request exceeds 32KB, the system allocates using VM.
	DKGL_API void* DKMemoryPoolAlloc(size_t);
	/// resize memory allocated by DKMemoryPoolAlloc
	DKGL_API void* DKMemoryPoolRealloc(void*, size_t);
	/// release memory allocated by DKMemoryPoolAlloc
	DKGL_API void  DKMemoryPoolFree(void*);
	/// purge unused memory pool chunks
	/// @note
	///   If you run out of memory, call DKAllocatorChain::Cleanup
	///   instead of calling DKMemoryPoolPurge, which purges memory pool only.
	DKGL_API size_t DKMemoryPoolPurge(void);
	/// query memory pool size
	DKGL_API size_t DKMemoryPoolSize(void);

	/**
	 @brief
		Memory pool allocation status info (for statistics)
		There are many sub-allocators in the memory pool.
		This is a single sub-allocator state for the pool.			
	 @code
		// getting allocation status!
		size_t numBuckets = DKMemoryPoolNumberOfBuckets();
		DKMemoryPoolBucketStatus* buckets = new DKMemoryPoolBucketStatus[numBuckets];
		DKMemoryPoolQueryAllocationStatus(buckets, numBuckets);
		for (int i = 0; i < numBuckets; ++i)
			printf("unit-size:%lu, allocated:%lu, reserved:%lu (usage:%.1f%%)\n",
				buckets[i].chunkSize,
				buckets[i].chunkSize * buckets[i].usedChunks, 
				buckets[i].chunkSize * (buckets[i].totalChunks - buckets[i].usedChunks),
				double(buckets[i].usedChunks) / double(buckets[i].totalChunks) * 100.0);
		printf("MemoryPool Usage: %.1fMB / %.1fMB\n", double(usedBytes) / (1024 * 1024), double(DKMemoryPoolSize()) / (1024 * 1024));
		delete[] buckets;
	 @endcode
	 @note
		This structure may contain a memory pool allocation range (that is, less than 32KB).
	 */
	struct DKMemoryPoolBucketStatus
	{
		size_t chunkSize;		///< allocation unit size of the allocator
		size_t totalChunks;		///< total chunks in the allocator
		size_t usedChunks;		///< allocated units
	};
	/// Get number of buckets, a bucket is a unit of sub-allocator in memory pool.
	/// this value does not change during run-time.
	DKGL_API size_t DKMemoryPoolNumberOfBuckets(void);
	/// Query allocation status of Memory-Pool.
	/// Do not call this function at exiting.
	/// @see DKMemoryPoolBucketStatus
	DKGL_API void DKMemoryPoolQueryAllocationStatus(DKMemoryPoolBucketStatus* status, size_t numBuckets);

	/// @brief track allocator location for debugging purpose
	/// you can provide your own allocator.
	/// @note
	/// you can use DKFileMap for file backed allocation
	/// @see DKFileMap
	enum DKMemoryLocation
	{
		DKMemoryLocationCustom = 0,	///< custom allocator, You can use DKFileMap as allocator
		DKMemoryLocationHeap,		///< allocated by DKMemoryHeapAlloc
		DKMemoryLocationVirtual,	///< allocated by DKMemoryVirtualAlloc
		DKMemoryLocationPool,		///< allocated by DKMemoryPoolAlloc
	};

	/// simple heap allocator type for template classes.
	struct DKMemoryHeapAllocator
	{
		enum { Location = DKMemoryLocationHeap };
		static void* Alloc(size_t s)			{ return DKMemoryHeapAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryHeapRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryHeapFree(p); }
	};
	/// simple vm allocator type for template classes.
	struct DKMemoryVirtualAllocator
	{
		enum { Location = DKMemoryLocationVirtual };
		static void* Alloc(size_t s)			{ return DKMemoryVirtualAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryVirtualRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryVirtualFree(p); }
	};
	/// simple memory pool allocator type for template class
	struct DKMemoryPoolAllocator
	{
		enum { Location = DKMemoryLocationPool };
		static void* Alloc(size_t s)			{ return DKMemoryPoolAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryPoolRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryPoolFree(p); }
	};

#ifdef DKGL_HEAP_ALLOCATOR_IS_DEFAULT	// define if you don't want to use memory-pool
	using DKMemoryDefaultAllocator = DKMemoryHeapAllocator;
#else
	using DKMemoryDefaultAllocator = DKMemoryPoolAllocator;
#endif
}
