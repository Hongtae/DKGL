//
//  File: DKAllocator.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKAllocator.h"
#include "DKObjectRefCounter.h"

using namespace DKFoundation;

DKAllocator::DKAllocator(void)
{
}

DKAllocator::~DKAllocator(void)
{
}

DKAllocator& DKAllocator::DefaultAllocator(DKMemoryLocation loc)
{
	static class HMAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryHeapAlloc(s);}
		void Dealloc(void* p)					{DKMemoryHeapFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationHeap;}
	} hma;
	static class VMAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryVirtualAlloc(s);}
		void Dealloc(void* p)					{DKMemoryVirtualFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationVirtual;}
	} vma;
	static class FMAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryFileAlloc(s);}
		void Dealloc(void* p)					{DKMemoryFileFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationFile;}
	} fma;
	static class RVAllocator : public DKAllocator
	{
		void* Alloc(size_t s)					{return DKMemoryReservedAlloc(s);}
		void Dealloc(void* p)					{DKMemoryReservedFree(p);}
		DKMemoryLocation Location(void) const	{return DKMemoryLocationReserved;}
	} rva;

	switch (loc)
	{
		case DKMemoryLocationHeap:
			return hma;
			break;
		case DKMemoryLocationVirtual:
			return vma;
			break;
		case DKMemoryLocationFile:
			return fma;
			break;
		case DKMemoryLocationReserved:
			return rva;
			break;
		default:		// custom?
			break;
	}
	return hma;
}

DKLIB_API void* operator new (size_t s, DKFoundation::DKAllocator& a)
{
	void* p = a.Alloc(s);
	DKObjectRefCounter::SetRefCounter(p, &a, 0, NULL);
	return p;
}

DKLIB_API void operator delete (void* p, DKFoundation::DKAllocator& a)
{
	DKAllocator* alloc = NULL;
	DKObjectRefCounter::UnsetRefCounter(p, 0, &alloc);
	DKASSERT_DEBUG(alloc == &a);
	a.Dealloc(p);
}
