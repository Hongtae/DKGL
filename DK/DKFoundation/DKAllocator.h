//
//  File: DKAllocator.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKMemory.h"
#include "DKAllocatorChain.h"

////////////////////////////////////////////////////////////////////////////////
// DKAllocator
// interface class for memory allocation, deallocation.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
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
		DKAllocator(const DKAllocator&);				/* = deleted */
		DKAllocator& operator = (const DKAllocator&);	/* = deleted */
	};
}

////////////////////////////////////////////////////////////////////////////////
// operator new, delete
//
// allocate memory and tracking reference-count by DKAllocator object.
// You need DKAllocator or inherited object.
////////////////////////////////////////////////////////////////////////////////

DKGL_API void* operator new (size_t, DKFoundation::DKAllocator&);	// to generation internal ref-count.
DKGL_API void operator delete (void*, DKFoundation::DKAllocator&);	// invoked when allocation failed.
