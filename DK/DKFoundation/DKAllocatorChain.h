//
//  File: DKAllocatorChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKAllocatorChain
// an abstract class, a memory allocator chain class.
// implemented as linked-list.
// subclass will be added to chain automatically when they are instantiated.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKAllocatorChain
	{
	public:
		DKAllocatorChain(void);
		virtual ~DKAllocatorChain(void);

		virtual void* Alloc(size_t) = 0;
		virtual void Dealloc(void*) = 0;

		virtual void Purge(void) {}
		virtual void Description(void) {}

		static void Cleanup(void);
		static DKAllocatorChain* FirstAllocator(void);
		DKAllocatorChain* NextAllocator(void);

	private:
		DKAllocatorChain* next;
	};
}
