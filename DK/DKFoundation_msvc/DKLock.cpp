//
//  File: DKLock.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKLock.h"
#include "DKLog.h"

#ifdef _WIN32
#include <windows.h>

namespace DKFoundation
{
	namespace Private
	{
		class LockImpl
		{
		public:
			LockImpl(void)
			{
				::InitializeCriticalSectionAndSpinCount(&section, 1024);
			}
			~LockImpl(void)
			{
				::DeleteCriticalSection(&section);
			}
			void Lock(void) const
			{
				::EnterCriticalSection(&section);
			}
			bool TryLock(void) const
			{
				return ::TryEnterCriticalSection(&section) != 0;
			}
			void Unlock(void) const
			{
				DKASSERT_DESC_DEBUG(section.OwningThread == (HANDLE)::GetCurrentThreadId(), "The current thread does not hold a lock on mutex.");
				::LeaveCriticalSection(&section);
				//::Sleep(0);
			}
			mutable CRITICAL_SECTION section;
		};
	}
}
#else		// pthread library
#include <pthread.h>
#include <errno.h>

namespace DKFoundation
{
	namespace Private
	{
		class LockImpl
		{
		public:
			LockImpl(void)
			{
				pthread_mutexattr_init(&attr);
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
				pthread_mutex_init(&mutex, &attr);
			}
			~LockImpl(void)
			{
				pthread_mutex_destroy(&mutex);
				pthread_mutexattr_destroy(&attr);
			}
			void Lock(void) const
			{
				int ret = pthread_mutex_lock(&mutex);
				if (ret == EINVAL)
				{
					// Lock failed.
					DKERROR_THROW_DEBUG("resource is invalid.");
				}
			}
			bool TryLock(void) const
			{
				return pthread_mutex_trylock(&mutex) == 0;
			}
			void Unlock(void) const
			{
				int ret = pthread_mutex_unlock(&mutex);
				if (ret == EPERM)
				{
					DKERROR_THROW_DEBUG("The current thread does not hold a lock on mutex.");
				}
				else if (ret == EINVAL)
				{
					DKERROR_THROW_DEBUG("resource is invalid.");
				}
			}
			mutable pthread_mutex_t mutex;
			pthread_mutexattr_t attr;
		};
	}
}
#endif

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKLock::DKLock(void)
{
	impl = reinterpret_cast<void*>(new LockImpl);
	DKASSERT_DEBUG(impl != NULL);
}

DKLock::~DKLock(void)
{
	DKASSERT_DEBUG(impl != NULL);
	delete reinterpret_cast<LockImpl*>(impl);
}

void DKLock::Lock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<LockImpl*>(impl)->Lock();
}

bool DKLock::TryLock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<LockImpl*>(impl)->TryLock();
}

void DKLock::Unlock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<LockImpl*>(impl)->Unlock();
}

