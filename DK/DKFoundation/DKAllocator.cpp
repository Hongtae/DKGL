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

	struct HMAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryHeapAlloc(s);}
		void Dealloc(void* p)					{DKMemoryHeapFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationHeap;}
	};
	struct VMAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryVirtualAlloc(s);}
		void Dealloc(void* p)					{DKMemoryVirtualFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationVirtual;}
	};
	struct FMAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryFileAlloc(s);}
		void Dealloc(void* p)					{DKMemoryFileFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationFile;}
	};
	struct RVAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryReservedAlloc(s);}
		void Dealloc(void* p)					{DKMemoryReservedFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationReserved;}
	};

	static bool initialized = false;
	static DKSpinLock lock;
	static HMAllocator* hma = NULL;
	static VMAllocator* vma = NULL;
	static FMAllocator* fma = NULL;
	static RVAllocator* rva = NULL;

	if (!initialized)
	{
		lock.Lock();
		if (!initialized)
		{
			hma = new HMAllocator();
			vma = new VMAllocator();
			fma = new FMAllocator();
			rva = new RVAllocator();
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
		case DKMemoryLocationReserved:
			return *rva;
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
#ifdef DKLIB_DEBUG_ENABLED
	if (!b)
	{
		throw std::runtime_error("DKObjectRefCounter failed");
	}
#endif
	return p;
}

DKLIB_API void operator delete (void* p, DKFoundation::DKAllocator& a)
{
	DKAllocator* alloc = NULL;
	DKObjectRefCounter::UnsetRefCounter(p, 0, &alloc);
#ifdef DKLIB_DEBUG_ENABLED
	if (alloc != &a)
	{
		throw std::runtime_error("Wrong allocator object.");
	}
#endif
	a.Dealloc(p);
}
