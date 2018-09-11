//
//  File: DKMutex.cpp
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

#include "DKMutex.h"
#include "DKLog.h"

namespace DKFoundation
{
	namespace Private
	{
#ifdef _WIN32
		class MutexImpl
		{
		public:
			MutexImpl()
				: mutex(::CreateMutex(NULL, FALSE, NULL))
#ifdef DKGL_DEBUG_ENABLED
				, ownerId(0)
#endif
			{
				DKASSERT_DEBUG(mutex != NULL);
			}
			~MutexImpl()
			{
				::CloseHandle(mutex);
			}
			void Lock() const
			{
				::WaitForSingleObject(mutex, INFINITE);
#ifdef DKGL_DEBUG_ENABLED
				DKASSERT_DEBUG(ownerId == 0);
				ownerId = ::GetCurrentThreadId();
#endif
			}
			bool TryLock() const
			{
				if (::WaitForSingleObject(mutex, 0) == WAIT_OBJECT_0)
				{
#ifdef DKGL_DEBUG_ENABLED
					DKASSERT_DEBUG(ownerId == 0);
					ownerId = ::GetCurrentThreadId();
#endif
					return true;
				}
				return false;
			}
			void Unlock() const
			{
#ifdef DKGL_DEBUG_ENABLED
				DKASSERT_DESC_DEBUG(ownerId == ::GetCurrentThreadId(), "The current thread does not own the mutex!");
				ownerId = 0;
#endif
				BOOL b = ::ReleaseMutex(mutex);
				DKASSERT_DESC_DEBUG(b, "ReleaseMutex FAILED");
			}
			HANDLE mutex;
#ifdef DKGL_DEBUG_ENABLED
			mutable DWORD ownerId;
#endif
		};
#else
		class MutexImpl
		{
		public:
			MutexImpl()
			{
				pthread_mutexattr_init(&attr);
#ifdef DKGL_DEBUG_ENABLED
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
#else
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
#endif
				pthread_mutex_init(&mutex, &attr);
			}
			~MutexImpl()
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
				else if (ret == EDEADLK)
				{
					// Dead lock!
					DKERROR_THROW_DEBUG("dead lock detected.");
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
			pthread_mutexattr_t attr;
			mutable pthread_mutex_t mutex;
		};
#endif
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKMutex::DKMutex()
{
	impl = reinterpret_cast<void*>(new MutexImpl());
	DKASSERT_DEBUG(impl != NULL);
}

DKMutex::~DKMutex()
{
	DKASSERT_DEBUG(impl != NULL);
	delete reinterpret_cast<MutexImpl*>(impl);
}

void DKMutex::Lock() const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<MutexImpl*>(impl)->Lock();
}

bool DKMutex::TryLock() const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<MutexImpl*>(impl)->TryLock();
}

void DKMutex::Unlock() const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<MutexImpl*>(impl)->Unlock();
}
