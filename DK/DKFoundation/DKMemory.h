//
//  File: DKMemory.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKMemory
// a basic memory management functions are provided.
//
// DKMemoryHeap-Alloc/Realloc/Free functions are same as malloc, realloc, free currently.
//
// DKMemoryVirtual-Alloc/Realloc/Free functions are using Virtual Memory.
// they should be aligned with DKMemoryVMPageSize value.
//
// If you want allocate memory with backing-store with file, use DKFileMap.
//
// Note:
//   If you run out of memory, call DKAllocatorChain::Cleanup()
//   instead of calling DKMemoryPoolPurge(), which purges memory pool only.
////////////////////////////////////////////////////////////////////////////////


namespace DKFoundation
{
	// heap memory
	DKGL_API void* DKMemoryHeapAlloc(size_t);
	DKGL_API void* DKMemoryHeapRealloc(void*, size_t);
	DKGL_API void  DKMemoryHeapFree(void*);

	// virtual-address, can commit, decommit.
	// data will be erased when decommit.
	DKGL_API void* DKMemoryVirtualAlloc(size_t);
	DKGL_API void* DKMemoryVirtualRealloc(void*, size_t);
	DKGL_API void  DKMemoryVirtualFree(void*);
	DKGL_API size_t  DKMemoryVirtualSize(void*);

	// system-paing functions.
	DKGL_API size_t DKMemoryPageSize(void); // default allocation size
	DKGL_API void* DKMemoryPageReserve(void*, size_t);
	DKGL_API void DKMemoryPageRelease(void*);
	DKGL_API void DKMemoryPageCommit(void*, size_t);
	DKGL_API void DKMemoryPageDecommit(void*, size_t);

	// Pre-allocated pool.
	DKGL_API void* DKMemoryPoolAlloc(size_t);
	DKGL_API void* DKMemoryPoolRealloc(void*, size_t);
	DKGL_API void  DKMemoryPoolFree(void*);
	// Optional pool management functions.
	DKGL_API size_t DKMemoryPoolPurge(void);
	DKGL_API size_t DKMemoryPoolSize(void);


	enum DKMemoryLocation
	{
		DKMemoryLocationCustom = 0,
		DKMemoryLocationHeap,
		DKMemoryLocationVirtual,
		DKMemoryLocationPool,
	};

	// simple allocator types for template classes.
	// you can provide your own allocator.
	struct DKMemoryHeapAllocator
	{
		enum { Location = DKMemoryLocationHeap };
		static void* Alloc(size_t s)			{ return DKMemoryHeapAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryHeapRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryHeapFree(p); }
	};
	struct DKMemoryVirtualAllocator
	{
		enum { Location = DKMemoryLocationVirtual };
		static void* Alloc(size_t s)			{ return DKMemoryVirtualAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryVirtualRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryVirtualFree(p); }
	};
	struct DKMemoryPoolAllocator
	{
		enum { Location = DKMemoryLocationPool };
		static void* Alloc(size_t s)			{ return DKMemoryPoolAlloc(s); }
		static void* Realloc(void* p, size_t s)	{ return DKMemoryPoolRealloc(p, s); }
		static void Free(void* p)				{ DKMemoryPoolFree(p); }
	};

	using DKMemoryDefaultAllocator = DKMemoryHeapAllocator;
}
