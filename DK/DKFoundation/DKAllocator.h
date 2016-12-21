//
//  File: DKAllocator.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKMemory.h"
#include "DKAllocatorChain.h"

namespace DKFoundation
{
	/// Interface class for memory allocation, deallocation.
	class DKGL_API DKAllocator : public DKAllocatorChain
	{
	public:
		virtual ~DKAllocator(void) noexcept(!DKGL_MEMORY_DEBUG);
		virtual void* Alloc(size_t) = 0;
		virtual void Dealloc(void*) = 0;
		virtual DKMemoryLocation Location(void) const = 0;

		static DKAllocator& DefaultAllocator(DKMemoryLocation loc = DKMemoryLocationHeap);

	protected:
		DKAllocator(void);

	private:
		DKAllocator(const DKAllocator&) = delete;
		DKAllocator& operator = (const DKAllocator&) = delete;
	};
}

/// allocate memory and tracking reference count by DKAllocator object.
/// You have to provide your allocator (DKAllocator subclass) object.
DKGL_API void* operator new (size_t, DKFoundation::DKAllocator&);
/// Invoked when allocation failed.
DKGL_API void operator delete (void*, DKFoundation::DKAllocator&);
