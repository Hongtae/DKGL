//
//  File: DKAllocator.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKMemory.h"
#include "DKAllocator.h"

////////////////////////////////////////////////////////////////////////////////
// DKObjectRefCounter
// object ref-counter, weak-ref management.
// You can determine whether object is alive or not, by using pointer or ref-id.
//
// Note:
//  Using this class is optinal.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	struct DKLIB_API DKObjectRefCounter
	{
		typedef uintptr_t RefCountValue;
		typedef uint64_t RefIdValue;

		// pointer(void*) and RefIdValue must belongs to same object.
		static bool IncrementRefCount(void*, RefIdValue);
		// increase ref-count +1, return false if not object found.
		static bool IncrementRefCount(void*);
		// decrease ref-count -1, return false if not object found.
		static bool DecrementRefCount(void*);
		// decrease ref-count -1, and remove item that is equal to RefCountValue
		// return true if object has been removed.
		static bool DecrementRefCountAndUnsetIfEqual(void*, RefCountValue, DKAllocator**);
		// decrease ref-count -1, and remove item if item's count becomes zero.
		// return true if object has been removed.
		static bool DecrementRefCountAndUnsetIfZero(void*, DKAllocator**);

		// begin ref-count state for any pointer, allocator
		static bool SetRefCounter(void*, DKAllocator*, RefCountValue, RefIdValue*);
		// remove item state if ref-count is equal to specified value (RefCountValue)
		static bool UnsetRefCounterIfEqual(void*, RefCountValue, DKAllocator**);
		// remove item state if ref-count is equal to zero.
		static bool UnsetRefCounterIfZero(void*, DKAllocator**);
		// remove item state. You can retrieve item ref-counted state before being removed.
		static bool UnsetRefCounter(void*, RefCountValue*, DKAllocator**);

		// retrieve object ref-count state value.
		static bool RefCount(void*, RefCountValue*);
		// retrieve RefId
		static bool RefId(void*, RefIdValue*);
		static DKMemoryLocation Location(void*);
		// return allocator if object has one.
		static DKAllocator* Allocator(void*);

		// functions for debugging.
		static size_t TableSize(void);
		static void TableDump(size_t*);
	};
}
