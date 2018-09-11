//
//  File: DKCondition.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#else // pthread
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#endif

#include "DKCondition.h"
#include "DKLog.h"

namespace DKFoundation
{
	namespace Private
	{
#ifdef _WIN32
		class ConditionImpl
		{
		public:
			ConditionImpl()
			{
				::InitializeCriticalSectionAndSpinCount(&section, 1024);
				::InitializeConditionVariable(&cond);
			}
			~ConditionImpl()
			{
				::DeleteCriticalSection(&section);
			}
			void Wait() const
			{
				DKASSERT_DESC_DEBUG(section.OwningThread == (HANDLE)::GetCurrentThreadId(), "The current thread does not hold a lock.");
				::SleepConditionVariableCS(&cond, &section, INFINITE);
			}
			bool WaitTimeout(double t) const
			{
				DKASSERT_DESC_DEBUG(section.OwningThread == (HANDLE)::GetCurrentThreadId(), "The current thread does not hold a lock.");
				return ::SleepConditionVariableCS(&cond, &section, static_cast<DWORD>(t * 1000)) != 0;
			}
			void Signal() const
			{
				::WakeConditionVariable(&cond);
			}
			void Broadcast() const
			{
				::WakeAllConditionVariable(&cond);
			}
			void Lock() const
			{
				::EnterCriticalSection(&section);
				DKASSERT_DESC_DEBUG(section.RecursionCount == 1, "dead lock detected.");
			}
			bool TryLock() const
			{
				return ::TryEnterCriticalSection(&section) != 0;
			}
			void Unlock() const
			{
				DKASSERT_DESC_DEBUG(section.OwningThread == (HANDLE)::GetCurrentThreadId(), "The current thread does not hold a lock.");
				::LeaveCriticalSection(&section);
			}
			mutable CRITICAL_SECTION section;
			mutable CONDITION_VARIABLE cond;
		};
#else
		class ConditionImpl
		{
		public:
			ConditionImpl()
			{
				pthread_mutexattr_init(&attr);
#ifdef DKGL_DEBUG_ENABLED
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
#else
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
#endif
				pthread_mutex_init(&mutex, &attr);
				pthread_cond_init(&cond, NULL);
			}
			~ConditionImpl()
			{
				pthread_mutex_destroy(&mutex);
				pthread_cond_destroy(&cond);
				pthread_mutexattr_destroy(&attr);
			}
			void Wait() const
			{
				pthread_cond_wait(&cond, &mutex);
			}
			bool WaitTimeout(double t) const
			{
				struct timeval tp;
				struct timespec ts;
				gettimeofday(&tp, NULL);
				
				long sec = static_cast<long>(t);
				tp.tv_sec += sec;
				tp.tv_usec += (t - sec) * 1000000;

				tp.tv_sec = tp.tv_sec + (tp.tv_usec / 1000000);
				tp.tv_usec = tp.tv_usec % 1000000;

				ts.tv_sec = tp.tv_sec;
				ts.tv_nsec = tp.tv_usec * 1000;
				
				return pthread_cond_timedwait(&cond, &mutex, &ts) == 0;
			}
			void Signal() const
			{
				pthread_cond_signal(&cond);
			}
			void Broadcast() const
			{
				pthread_cond_broadcast(&cond);
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
					// Dead lock.
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
			mutable pthread_cond_t cond;
		};
#endif
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKCondition::DKCondition()
{
	impl = reinterpret_cast<void*>(new ConditionImpl());
	DKASSERT_DEBUG(impl != NULL);
}

DKCondition::~DKCondition()
{
	DKASSERT_DEBUG(impl != NULL);
	delete reinterpret_cast<ConditionImpl*>(impl);
}

void DKCondition::Wait() const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<ConditionImpl*>(impl)->Wait();
}

bool DKCondition::WaitTimeout(double t) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<ConditionImpl*>(impl)->WaitTimeout(Max(t, 0.0));
}

void DKCondition::Signal() const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<ConditionImpl*>(impl)->Signal();
}

void DKCondition::Broadcast() const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<ConditionImpl*>(impl)->Broadcast();
}

void DKCondition::Lock() const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<ConditionImpl*>(impl)->Lock();
}

bool DKCondition::TryLock() const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<ConditionImpl*>(impl)->TryLock();
}

void DKCondition::Unlock() const
{
	DKASSERT_DEBUG(impl != NULL);
	reinterpret_cast<ConditionImpl*>(impl)->Unlock();
}
