//
//  File: DKAllocatorChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// @brief
	///  An abstract class, a memory allocator chain class.
	///  implemented as linked-list.
	///  subclass will be added to chain automatically when they are instantiated.
	///
	/// @note
	///  If you need to use DKAllocator with static (global) object initialization,
	///  You have to hold Maintainer instance as static storage
	/// @note
	///  The Maintainer will postpone destruction of internal allocators and
	///  allocation pools until all Maintainer instances are destroyed.
	///  It is necessary to internal allocator become persistent even when main()
	///  function has been finished. (even after atexit() called)
	///
	///  Maintainer Usage:
	///    Just declare static instance before using any allocators in global scope.
	///    It is not necessary for function scope inside of main() routine.
	class DKGL_API DKAllocatorChain
	{
	public:
		DKAllocatorChain();
		virtual ~DKAllocatorChain() noexcept(!DKGL_MEMORY_DEBUG);

		virtual void* Alloc(size_t) = 0;
		virtual void* Realloc(void*, size_t) = 0;
		virtual void Dealloc(void*) = 0;

		virtual size_t Purge() { return 0; }
		virtual void Description() {}

		static size_t Cleanup();
		static DKAllocatorChain* FirstAllocator();
		DKAllocatorChain* NextAllocator();

		/// @brief Prevent destruction of allocator-chain of located in static
		///  storage at application is being terminated
		///  To extend static-object life cycle, a static-object which own
		///  DKAllocator, it should have Maintainer instance with static storage.
		struct DKGL_API Maintainer
		{
			Maintainer();
			~Maintainer() noexcept(!DKGL_MEMORY_DEBUG);
		};

		void* operator new (size_t);
		void operator delete (void*) noexcept;
	private:
		DKAllocatorChain* next;
	};
}
