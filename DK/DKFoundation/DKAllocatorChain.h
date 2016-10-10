//
//  File: DKAllocatorChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKAllocatorChain
// an abstract class, a memory allocator chain class.
// implemented as linked-list.
// subclass will be added to chain automatically when they are instantiated.
//
// Warning:
//  If you need to use DKAllocator with static (global) object initialization,
//  You have to hold Maintainer instance as static storage.
//
//  The Maintainer will postpone destruction of internal allocators and
//  allocation pools until all Maintainer instances are destroyed.
//  It is necessary to internal allocator become persistent even when main()
//  function has been finished. (even after atexit() called)
//
//  Maintainer Usage:
//    Just declare static instance before using any allocators in global scope.
//    not necessary for function scope inside of main() routine.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKAllocatorChain
	{
	public:
		DKAllocatorChain(void);
		virtual ~DKAllocatorChain(void) noexcept(!DKGL_MEMORY_DEBUG);

		virtual void* Alloc(size_t) = 0;
		virtual void Dealloc(void*) = 0;

		virtual size_t Purge(void) { return 0; }
		virtual void Description(void) {}

		static size_t Cleanup(void);
		static DKAllocatorChain* FirstAllocator(void);
		DKAllocatorChain* NextAllocator(void);


		// To extend static-object life cycle, a static-object which own
		// DKAllocator, it should have Maintainer instance with static storage.
		struct DKGL_API Maintainer
		{
			Maintainer(void);
			~Maintainer(void) noexcept(!DKGL_MEMORY_DEBUG);
		};

		void* operator new (size_t);
		void operator delete (void*) noexcept;
	private:
		DKAllocatorChain* next;
	};
}
