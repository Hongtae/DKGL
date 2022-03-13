//
//  File: DKCondition.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
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

#define WIN32_COND_SWRLOCK

namespace DKFoundation::Private
{
#ifdef _WIN32
    class ConditionImpl
    {
    public:
        ConditionImpl()
        {
#ifdef WIN32_COND_SWRLOCK
            InitializeSRWLock(&lock);
#ifdef DKGL_DEBUG_ENABLED
            owningThreadId = 0;
#endif
#else
            InitializeCriticalSectionAndSpinCount(&section, 1024);
#endif
            InitializeConditionVariable(&cond);
        }
        ~ConditionImpl()
        {
#ifdef WIN32_COND_SWRLOCK
            DKASSERT_DEBUG(owningThreadId == 0);
#else
            DeleteCriticalSection(&section);
#endif
        }
        void Wait() const
        {
#ifdef WIN32_COND_SWRLOCK
#ifdef DKGL_DEBUG_ENABLED
            DKASSERT_DESC_DEBUG(owningThreadId == GetCurrentThreadId(), "The current thread does not hold a lock.");
            auto tmp = owningThreadId;
            owningThreadId = 0;
#endif
            SleepConditionVariableSRW(&cond, &lock, INFINITE, 0);
#ifdef DKGL_DEBUG_ENABLED
            owningThreadId = tmp;
#endif
#else
            DKASSERT_DESC_DEBUG(section.OwningThread == (HANDLE)GetCurrentThreadId(), "The current thread does not hold a lock.");
            SleepConditionVariableCS(&cond, &section, INFINITE);
#endif
        }
        bool WaitTimeout(double t) const
        {
#ifdef WIN32_COND_SWRLOCK
#ifdef DKGL_DEBUG_ENABLED
            DKASSERT_DESC_DEBUG(owningThreadId == GetCurrentThreadId(), "The current thread does not hold a lock.");
            auto tmp = owningThreadId;
            owningThreadId = 0;
#endif
            bool r = SleepConditionVariableSRW(&cond, &lock, static_cast<DWORD>(t * 1000), 0) != 0;
#ifdef DKGL_DEBUG_ENABLED
            owningThreadId = tmp;
#endif
            return r;
#else
            DKASSERT_DESC_DEBUG(section.OwningThread == (HANDLE)GetCurrentThreadId(), "The current thread does not hold a lock.");
            return SleepConditionVariableCS(&cond, &section, static_cast<DWORD>(t * 1000)) != 0;
#endif
        }
        void Signal() const
        {
            WakeConditionVariable(&cond);
        }
        void Broadcast() const
        {
            WakeAllConditionVariable(&cond);
        }
        void Lock() const
        {
#ifdef WIN32_COND_SWRLOCK
#ifdef DKGL_DEBUG_ENABLED
            DKASSERT_DESC_DEBUG(owningThreadId != GetCurrentThreadId(), "The current thread already hold a lock.");
#endif
            AcquireSRWLockExclusive(&lock);
#ifdef DKGL_DEBUG_ENABLED
            DKASSERT_DESC_DEBUG(owningThreadId == 0, "The lock must not belong to another thread.");
            owningThreadId = GetCurrentThreadId();
#endif
#else
            EnterCriticalSection(&section);
            DKASSERT_DESC_DEBUG(section.RecursionCount == 1, "dead lock detected.");
#endif
        }
        bool TryLock() const
        {
#ifdef WIN32_COND_SWRLOCK
            if (TryAcquireSRWLockExclusive(&lock))
            {
#ifdef DKGL_DEBUG_ENABLED
                DKASSERT_DESC_DEBUG(owningThreadId == 0, "The lock mut not belong to anoother thread.");
                owningThreadId = GetCurrentThreadId();
#endif
                return true;
            }
            return false;
#else
            return TryEnterCriticalSection(&section) != 0;
#endif
        }
        void Unlock() const
        {
#ifdef WIN32_COND_SWRLOCK
#ifdef DKGL_DEBUG_ENABLED
            DKASSERT_DESC_DEBUG(owningThreadId == GetCurrentThreadId(), "The current thread does not hold a lock.");
            owningThreadId = 0;
#endif
            ReleaseSRWLockExclusive(&lock);
#else
            DKASSERT_DESC_DEBUG(section.OwningThread == (HANDLE)GetCurrentThreadId(), "The current thread does not hold a lock.");
            LeaveCriticalSection(&section);
#endif
        }
#ifdef WIN32_COND_SWRLOCK
        mutable SRWLOCK lock;
#ifdef DKGL_DEBUG_ENABLED
        mutable DWORD owningThreadId;
#endif
#else
        mutable CRITICAL_SECTION section;
#endif
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
