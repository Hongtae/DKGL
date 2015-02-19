//
//  File: DKMutex.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKMutex.h"
#include "DKLog.h"

#ifdef _WIN32
#include <windows.h>

namespace DKFoundation
{
	namespace Private
	{
		class MutexImpl
		{
		public:
			MutexImpl(void)
				: mutex(::CreateMutex(NULL, FALSE, NULL))
#ifdef DKLIB_DEBUG_ENABLED
				, ownerId(0)
#endif
			{
				DKASSERT_DEBUG(mutex != NULL);
			}
			~MutexImpl(void)
			{
				::CloseHandle(mutex);
			}
			void Lock(void) const
			{
				::WaitForSingleObject(mutex, INFINITE);
#ifdef DKLIB_DEBUG_ENABLED
				DKASSERT_DEBUG(ownerId == 0);
				ownerId = ::GetCurrentThreadId();
#endif
			}
			bool TryLock(void) const
			{
				if (::WaitForSingleObject(mutex, 0) == WAIT_OBJECT_0)
				{
#ifdef DKLIB_DEBUG_ENABLED
					DKASSERT_DEBUG(ownerId == 0);
					ownerId = ::GetCurrentThreadId();
#endif
					return true;
				}
				return false;
			}
			void Unlock(void) const
			{
#ifdef DKLIB_DEBUG_ENABLED
				DKASSERT_DESC_DEBUG(ownerId == ::GetCurrentThreadId(), "The current thread does not own the mutex!");
				ownerId = 0;
#endif
				BOOL b = ::ReleaseMutex(mutex);
				DKASSERT_DESC_DEBUG(b, "ReleaseMutex FAILED");
			}
			HANDLE mutex;
#ifdef DKLIB_DEBUG_ENABLED
			mutable DWORD ownerId;
#endif
		};
	}
}
#else			// pthread
#include <pthread.h>
#include <errno.h>

namespace DKFoundation
{
	namespace Private
	{
		class MutexImpl
		{
		public:
			MutexImpl(void)
			{
				pthread_mutexattr_init(&attr);
#ifdef DKLIB_DEBUG_ENABLED
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
#else
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
#endif
				pthread_mutex_init(&mutex, &attr);
			}
			~MutexImpl(void)
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
				else if (ret == EDEADLK)
				{
					// Dead lock!
					DKERROR_THROW_DEBUG("dead lock detected.");
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
			pthread_mutexattr_t attr;
			mutable pthread_mutex_t mutex;
		};
	}
}
#endif

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKMutex::DKMutex(void)
{
	impl = reinterpret_cast<void*>(new MutexImpl);
	DKASSERT_DEBUG(impl != NULL);
}

DKMutex::~DKMutex(void)
{
	DKASSERT_DEBUG(impl != NULL);
	delete reinterpret_cast<MutexImpl*>(impl);
}

void DKMutex::Lock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<MutexImpl*>(impl)->Lock();
}

bool DKMutex::TryLock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<MutexImpl*>(impl)->TryLock();
}

void DKMutex::Unlock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<MutexImpl*>(impl)->Unlock();
}