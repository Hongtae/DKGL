//
//  File: DKMemory.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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
////////////////////////////////////////////////////////////////////////////////


namespace DKFoundation
{
	// heap memory
	DKLIB_API void* DKMemoryHeapAlloc(size_t);
	DKLIB_API void* DKMemoryHeapRealloc(void*, size_t);
	DKLIB_API void  DKMemoryHeapFree(void*);

	// virtual-address, can commit, decommit.
	// data will be erased when decommit.
	DKLIB_API void* DKMemoryVirtualAlloc(size_t);
	DKLIB_API void* DKMemoryVirtualRealloc(void*, size_t);
	DKLIB_API void  DKMemoryVirtualFree(void*);
	DKLIB_API size_t  DKMemoryVirtualSize(void*);

	// system-paing functions.
	DKLIB_API size_t DKMemoryPageSize(void); // default allocation size
	DKLIB_API void* DKMemoryPageReserve(void*, size_t);
	DKLIB_API void DKMemoryPageRelease(void*);
	DKLIB_API void DKMemoryPageCommit(void*, size_t);
	DKLIB_API void DKMemoryPageDecommit(void*, size_t);

	// Pre-allocated pool.
	DKLIB_API void* DKMemoryPoolAlloc(size_t);
	DKLIB_API void* DKMemoryPoolRealloc(void*, size_t);
	DKLIB_API void  DKMemoryPoolFree(void*);
	// Optional pool management functions.
	DKLIB_API size_t DKMemoryPoolPurge(void);
	DKLIB_API size_t DKMemoryPoolSize(void);


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
