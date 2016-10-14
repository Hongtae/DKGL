//
//  File: DKAllocatorChain.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include <new>
#include "DKAllocatorChain.h"
#include "DKSpinLock.h"
#include "DKCriticalSection.h"

namespace DKGL
{
	namespace Private
	{
		void CreateAllocationTable(void);
		void DestroyAllocationTable(void);

		using ScopedSpinLock = DKCriticalSection<DKSpinLock>;
		struct Chain
		{
			using RefCount = unsigned int;

			DKAllocatorChain* first;
			DKSpinLock lock;
			RefCount refCount;
			static Chain* instance;

			Chain(void)
			{
				CreateAllocationTable();
				lock.Lock();
				first = NULL;
				instance = this;
				refCount = 0;
				lock.Unlock();
			}
			~Chain(void)
			{
				DestroyAllocationTable();
				while (true)
				{
					lock.Lock();
					DKAllocatorChain* p = first;
					lock.Unlock();
					if (p)
						delete p;
					else
						break;
				}

				lock.Lock();
				instance = NULL;
				first = NULL;
				lock.Unlock();
			}
			static Chain* Instance(void)
			{
				static Chain* p = new Chain();
				return p->instance;
			}
			RefCount IncrementRef(void)
			{
				ScopedSpinLock guard(lock);
				this->refCount++;
				return this->refCount;
			}
			RefCount DecrementRef(void)
			{
				ScopedSpinLock guard(lock);
				this->refCount--;
				return this->refCount;
			}
		};
		Chain* Chain::instance;

		// default Chain-Holder
		static DKAllocatorChain::Maintainer init;
	}
}

using namespace DKGL;
using namespace DKGL::Private;


DKAllocatorChain::DKAllocatorChain(void)
: next(NULL)
{
	Chain* c = Chain::Instance();
	ScopedSpinLock guard(c->lock);
	if (c->first)
	{
		DKAllocatorChain* last = c->first;
		while (last->next)
			last = last->next;
		last->next = this;
	}
	else
	{
		c->first = this;
	}
}

DKAllocatorChain::~DKAllocatorChain(void) noexcept(!DKGL_MEMORY_DEBUG)
{
	Chain* c = Chain::Instance();
	ScopedSpinLock guard(c->lock);
	if (c->first == this)
		c->first = this->next;
	else
	{
		for (DKAllocatorChain* chain = c->first; chain; chain = chain->next)
		{
			if (chain->next == this)
			{
				chain->next = this->next;
				break;
			}
		}
	}
}

size_t DKAllocatorChain::Cleanup(void)
{
	size_t purged = 0;
	Chain* c = Chain::Instance();
	ScopedSpinLock guard(c->lock);
	for (DKAllocatorChain* chain = c->first; chain; chain = chain->next)
	{
		purged += chain->Purge();
	}
	return purged;
}

DKAllocatorChain* DKAllocatorChain::FirstAllocator(void)
{
	Chain* c = Chain::Instance();
	ScopedSpinLock guard(c->lock);
	return c->first;
}

DKAllocatorChain* DKAllocatorChain::NextAllocator(void)
{
	return next;
}

DKAllocatorChain::Maintainer::Maintainer(void)
{
	Chain* c = Chain::Instance();
	DKASSERT_STD_DEBUG( c != NULL );
	Chain::RefCount ref = c->IncrementRef();
	DKASSERT_STD_DEBUG( ref >= 0);
}

DKAllocatorChain::Maintainer::~Maintainer(void) noexcept(!DKGL_MEMORY_DEBUG)
{
	Chain* c = Chain::Instance();
	DKASSERT_STD_DEBUG( c != NULL );
	Chain::RefCount ref = c->DecrementRef();
	DKASSERT_STD_DEBUG( ref >= 0);
	if (ref == 0)
		delete c;
}

void* DKAllocatorChain::operator new (size_t s)
{
	return ::malloc(s);
}

void DKAllocatorChain::operator delete (void* p) noexcept
{
	::free(p);
}

