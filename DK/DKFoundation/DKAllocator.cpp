//
//  File: DKAllocator.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include <new>
#include <stdexcept>
#include "DKAllocator.h"
#include "DKObjectRefCounter.h"
#include "DKSpinLock.h"

using namespace DKFoundation;

DKAllocator::DKAllocator(void)
{
}

DKAllocator::~DKAllocator(void)
{
}

DKAllocator& DKAllocator::DefaultAllocator(DKMemoryLocation loc)
{
	static StaticInitializer init;

	struct HeapAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryHeapAlloc(s);}
		void Dealloc(void* p)					{DKMemoryHeapFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationHeap;}
	};
	struct VMemAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryVirtualAlloc(s);}
		void Dealloc(void* p)					{DKMemoryVirtualFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationVirtual;}
	};
	struct FMemAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryFileAlloc(s);}
		void Dealloc(void* p)					{DKMemoryFileFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationFile;}
	};
	struct PoolAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryPoolAlloc(s);}
		void Dealloc(void* p)					{DKMemoryPoolFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationPool;}
	};

	static bool initialized = false;
	static DKSpinLock lock;

	static HeapAllocator* hma = NULL;
	static VMemAllocator* vma = NULL;
	static FMemAllocator* fma = NULL;
	static PoolAllocator* pma = NULL;

	if (!initialized)
	{
		lock.Lock();
		if (!initialized)
		{
			hma = new HeapAllocator();
			vma = new VMemAllocator();
			fma = new FMemAllocator();
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
		case DKMemoryLocationFile:
			return *fma;
			break;
		case DKMemoryLocationPool:
			return *pma;
			break;
		default:		// custom?
			break;
	}
	return *hma;
}

DKLIB_API void* operator new (size_t s, DKFoundation::DKAllocator& a)
{
	void* p = a.Alloc(s);
	bool b = DKObjectRefCounter::SetRefCounter(p, &a, 0, NULL);
	DKASSERT_STD_DESC_DEBUG(b, "DKObjectRefCounter failed.");
	return p;
}

DKLIB_API void operator delete (void* p, DKFoundation::DKAllocator& a)
{
	DKAllocator* alloc = NULL;
	DKObjectRefCounter::UnsetRefCounter(p, 0, &alloc);
	DKASSERT_STD_DESC_DEBUG(alloc == &a, "Wrong allocator object.");
	a.Dealloc(p);
}
