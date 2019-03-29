//
//  File: DKLock.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <errno.h>
#endif

#include "DKLock.h"
#include "DKLog.h"

namespace DKFoundation
{
	namespace Private
	{
#ifdef _WIN32
		class LockImpl
		{
		public:
			LockImpl()
			{
				::InitializeCriticalSectionAndSpinCount(&section, 1024);
			}
			~LockImpl()
			{
				::DeleteCriticalSection(&section);
			}
			void Lock() const
			{
				::EnterCriticalSection(&section);
			}
			bool TryLock() const
			{
				return ::TryEnterCriticalSection(&section) != 0;
			}
			void Unlock() const
			{
				DKASSERT_DESC_DEBUG(section.OwningThread == (HANDLE)::GetCurrentThreadId(), "The current thread does not hold a lock on mutex.");
				::LeaveCriticalSection(&section);
				//::Sleep(0);
			}
			mutable CRITICAL_SECTION section;
		};
#else
		class LockImpl
		{
		public:
			LockImpl()
			{
				pthread_mutexattr_init(&attr);
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
				pthread_mutex_init(&mutex, &attr);
			}
			~LockImpl()
			{
				pthread_mutex_destroy(&mutex);
				pthread_mutexattr_destroy(&attr);
			}
			void Lock() const
			{
				int ret = pthread_mutex_lock(&mutex);
				if (ret == EINVAL)
				{
					// Lock failed.
					DKERROR_THROW_DEBUG("resource is invalid.");
				}
			}
			bool TryLock() const
			{
				return pthread_mutex_trylock(&mutex) == 0;
			}
			void Unlock() const
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
#endif
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKLock::DKLock()
{
	impl = reinterpret_cast<void*>(new LockImpl());
	DKASSERT_DEBUG(impl != NULL);
}

DKLock::~DKLock()
{
	DKASSERT_DEBUG(impl != NULL);
	delete reinterpret_cast<LockImpl*>(impl);
}

void DKLock::Lock() const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<LockImpl*>(impl)->Lock();
}

bool DKLock::TryLock() const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<LockImpl*>(impl)->TryLock();
}

void DKLock::Unlock() const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<LockImpl*>(impl)->Unlock();
}

