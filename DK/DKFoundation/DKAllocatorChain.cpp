//
//  File: DKAllocatorChain.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#include "DKAllocatorChain.h"
#include "DKSpinLock.h"
#include "DKCriticalSection.h"

namespace DKFoundation
{
	namespace Private
	{
		static DKAllocatorChain* first = NULL;
		static DKSpinLock chainLock;
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKAllocatorChain::DKAllocatorChain(void)
: next(NULL)
{
	DKCriticalSection<DKSpinLock> guard(chainLock);
	if (first)
	{
		DKAllocatorChain* last = first;
		while (last->next)
			last = last->next;
		last->next = this;
	}
	else
	{
		first = this;
	}
}

DKAllocatorChain::~DKAllocatorChain(void)
{
	DKCriticalSection<DKSpinLock> guard(chainLock);
	if (first == this)
		first = this->next;
	else
	{
		for (DKAllocatorChain* chain = first; chain; chain = chain->next)
		{
			if (chain->next == this)
			{
				chain->next = this->next;
				break;
			}
		}
	}
}

void DKAllocatorChain::Cleanup(void)
{
	DKCriticalSection<DKSpinLock> guard(chainLock);
	for (DKAllocatorChain* chain = first; chain; chain = chain->next)
	{
		chain->Purge();
	}
}

DKAllocatorChain* DKAllocatorChain::FirstAllocator(void)
{
	DKCriticalSection<DKSpinLock> guard(chainLock);
	return first;
}

DKAllocatorChain* DKAllocatorChain::NextAllocator(void)
{
	return next;
}
