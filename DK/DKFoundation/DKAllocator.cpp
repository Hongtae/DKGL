//
//  File: DKAllocator.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <new>
#include <stdexcept>
#include "DKAllocator.h"
#include "DKObjectRefCounter.h"
#include "DKSpinLock.h"

using namespace DKFoundation;

DKAllocator::DKAllocator()
{
}

DKAllocator::~DKAllocator() noexcept(!DKGL_MEMORY_DEBUG)
{
}

DKAllocator& DKAllocator::DefaultAllocator(DKMemoryLocation loc)
{
	static Maintainer init;

	struct HeapAllocator : public DKAllocator
	{
		void* Alloc(size_t s) override					{ return DKMemoryHeapAlloc(s); }
		void* Realloc(void* p, size_t s) override		{ return DKMemoryHeapRealloc(p, s); }
		void Dealloc(void* p) override					{ DKMemoryHeapFree(p); }
		DKMemoryLocation Location() const override	{ return DKMemoryLocationHeap; }
	};
	struct VMemAllocator : public DKAllocator
	{
		void* Alloc(size_t s)override					{ return DKMemoryVirtualAlloc(s); }
		void* Realloc(void* p, size_t s) override		{ return DKMemoryVirtualRealloc(p, s); }
		void Dealloc(void* p)override					{ DKMemoryVirtualFree(p); }
		DKMemoryLocation Location() const override	{ return DKMemoryLocationVirtual; }
	};
	struct PoolAllocator : public DKAllocator
	{
		void* Alloc(size_t s) override					{ return DKMemoryPoolAlloc(s); }
		void* Realloc(void* p, size_t s) override		{ return DKMemoryPoolRealloc(p, s); }
		void Dealloc(void* p) override					{ DKMemoryPoolFree(p); }
		DKMemoryLocation Location() const override	{ return DKMemoryLocationPool; }
	};

	static bool initialized = false;
	static DKSpinLock lock;

	static HeapAllocator* hma = NULL;
	static VMemAllocator* vma = NULL;
	static PoolAllocator* pma = NULL;

	if (!initialized)
	{
		lock.Lock();
		if (!initialized)
		{
			hma = new HeapAllocator();
			vma = new VMemAllocator();
			pma = new PoolAllocator();
			initialized = true;
		}
		lock.Unlock();
	}

	switch (loc)
	{
		case DKMemoryLocationHeap:
			return *hma;
			break;
		case DKMemoryLocationVirtual:
			return *vma;
			break;
		case DKMemoryLocationPool:
			return *pma;
			break;
		default:		// custom?
			break;
	}
	return *hma;
}

DKGL_API void* operator new (size_t s, DKAllocator& a)
{
	void* p = a.Alloc(s);
	bool b = DKObjectRefCounter::SetRefCounter(p, &a, 0, NULL);
	DKASSERT_STD_DESC_DEBUG(b, "DKObjectRefCounter failed.");
	return p;
}

DKGL_API void operator delete (void* p, DKAllocator& a)
{
	DKAllocator* alloc = NULL;
	DKObjectRefCounter::UnsetRefCounter(p, 0, &alloc);
	DKASSERT_STD_DESC_DEBUG(alloc == &a, "Wrong allocator object.");
	a.Dealloc(p);
}
