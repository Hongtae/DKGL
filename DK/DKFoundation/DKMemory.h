//
//  File: DKMemory.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
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
// DKMemoryFile-Alloc/Realloc/Free functions using actual file as backing store.
// if system disk runs out of space, this function will fail.
//
// Note:
//  If you try allocate memory larger than system-available memory
//  by using DKMemoryFileAlloc, system freezing can be occurred.
//
//  DKMemoryReserved-Alloc/Realloc/Free functions is only for debugging. don't use.
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

	// using HDD file to manage memory.
	DKLIB_API void* DKMemoryFileAlloc(size_t);
	DKLIB_API void* DKMemoryFileRealloc(void*, size_t);
	DKLIB_API void  DKMemoryFileFree(void*);

	// system-paing functions.
	DKLIB_API size_t DKMemoryPageSize(void); // default allocation size
	DKLIB_API void* DKMemoryPageReserve(void*, size_t);
	DKLIB_API void DKMemoryPageRelease(void*);
	DKLIB_API void DKMemoryPageCommit(void*, size_t);
	DKLIB_API void DKMemoryPageDecommit(void*, size_t);

	// Don't use below DKMemoryReserved.. functions.
	DKLIB_API void* DKMemoryReservedAlloc(size_t);
	DKLIB_API void* DKMemoryReservedRealloc(void*, size_t);
	DKLIB_API void  DKMemoryReservedFree(void*);

	enum DKMemoryLocation
	{
		DKMemoryLocationCustom = 0,
		DKMemoryLocationHeap,
		DKMemoryLocationVirtual,
		DKMemoryLocationFile,
		DKMemoryLocationReserved,
	};

	// simple allocator types for template classes.
	// you can provide your own allocator.
	struct DKMemoryHMAllocator
	{
		enum {location = DKMemoryLocationHeap};
		static void* Alloc(size_t s)			{return DKMemoryHeapAlloc(s);}
		static void* Realloc(void* p, size_t s)	{return DKMemoryHeapRealloc(p, s);}
		static void Free(void* p)				{DKMemoryHeapFree(p);}
	};
	class DKMemoryVMAllocator
	{
		enum {location = DKMemoryLocationVirtual};
		static void* Alloc(size_t s)			{return DKMemoryVirtualAlloc(s);}
		static void* Realloc(void* p, size_t s)	{return DKMemoryVirtualRealloc(p, s);}
		static void Free(void* p)				{DKMemoryVirtualFree(p);}
	};
	class DKMemoryFMAllocator
	{
		enum {location = DKMemoryLocationFile};
		static void* Alloc(size_t s)			{return DKMemoryFileAlloc(s);}
		static void* Realloc(void* p, size_t s)	{return DKMemoryFileRealloc(p, s);}
		static void Free(void* p)				{DKMemoryFileFree(p);}
	};	
	class DKMemoryRVAllocator
	{
		enum {location = DKMemoryLocationReserved};
		static void* Alloc(size_t s)			{return DKMemoryReservedAlloc(s);}
		static void* Realloc(void* p, size_t s)	{return DKMemoryReservedRealloc(p, s);}
		static void Free(void* p)				{DKMemoryReservedFree(p);}
	};

	using DKMemoryDefaultAllocator = DKMemoryHMAllocator;
}
