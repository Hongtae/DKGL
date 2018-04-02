//
//  File: DKSharedLock.cpp
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

#include "DKSharedLock.h"
#include "DKThread.h"
#include "DKMap.h"
#include "DKSpinLock.h"

namespace DKFoundation
{
	namespace Private
	{
#ifdef _WIN32
		class SharedLockImpl
		{
		private:
#ifdef DKGL_DEBUG_ENABLED
			DKSpinLock spinLock;
			typedef DKMap<DWORD,size_t> ThreadCountMap;
			mutable ThreadCountMap readerThreads;
			mutable DWORD writerThread;

			size_t SharedCount(void) const
			{
				size_t num = 0;
				readerThreads.EnumerateForward([&num](const ThreadCountMap::Pair& p){ num += p.value; });
				return num;
			}
			size_t SharedCountForCurrentThread(void) const
			{
				size_t num = 0;
				DWORD tid = ::GetCurrentThreadId();
				readerThreads.EnumerateForward([&num, tid](const ThreadCountMap::Pair& p){ if (p.key == tid) num += p.value; });
				return num;
			}
			void IncrementShareCount(void) const
			{
				DWORD tid = ::GetCurrentThreadId();
				ThreadCountMap::Pair* p = readerThreads.Find(tid);
				if (p)
				{
					p->value++;
				}
				else
				{
					readerThreads.Insert(tid, 1);
				}
			}
			bool DecrementShareCount(void) const
			{
				DWORD tid = ::GetCurrentThreadId();
				ThreadCountMap::Pair* p = readerThreads.Find(tid);
				if (p && p->value > 0)
				{					
					p->value--;
					return true;
				}
				return false;
			}
#endif
		public:
			SharedLockImpl(void)
			{
				::InitializeSRWLock(&lock);
#ifdef DKGL_DEBUG_ENABLED
				this->readerThreads.Clear();
				this->writerThread = 0;
#endif
			}
			~SharedLockImpl(void)
			{
			}
			void Lock(void) const
			{
				::AcquireSRWLockExclusive(&lock);
#ifdef DKGL_DEBUG_ENABLED
				DKCriticalSection<DKSpinLock> guard(this->spinLock);
				DKASSERT(this->SharedCount() == 0);
				DKASSERT(this->writerThread == NULL);
				this->writerThread = ::GetCurrentThreadId();
#endif
			}
			bool TryLock(void) const
			{
				if (::TryAcquireSRWLockExclusive(&lock))
				{
#ifdef DKGL_DEBUG_ENABLED
					DKCriticalSection<DKSpinLock> guard(this->spinLock);
					DKASSERT(this->SharedCount() == 0);
					DKASSERT(this->writerThread == NULL);
					this->writerThread = ::GetCurrentThreadId();
#endif
					return true;
				}
				return false;
			}
			void Unlock(void) const
			{
#ifdef DKGL_DEBUG_ENABLED
				DKCriticalSection<DKSpinLock> guard(this->spinLock);
				DKASSERT(this->writerThread == ::GetCurrentThreadId());
				DKASSERT(this->SharedCount() == 0);
				this->writerThread = NULL;
#endif
				::ReleaseSRWLockExclusive(&lock);
			}
			void LockShared(void) const
			{
				::AcquireSRWLockShared(&lock);
#ifdef DKGL_DEBUG_ENABLED
				DKCriticalSection<DKSpinLock> guard(this->spinLock);
				DKASSERT(this->writerThread == NULL);
				this->IncrementShareCount();
#endif
			}
			bool TryLockShared(void) const
			{
				if (::TryAcquireSRWLockShared(&lock))
				{
#ifdef DKGL_DEBUG_ENABLED
					DKCriticalSection<DKSpinLock> guard(this->spinLock);
					DKASSERT(this->writerThread == NULL);
					this->IncrementShareCount();
#endif
					return true;
				}
				return false;
			}
			void UnlockShared(void) const
			{
#ifdef DKGL_DEBUG_ENABLED
				DKCriticalSection<DKSpinLock> guard(this->spinLock);
				DKASSERT(this->DecrementShareCount());
				DKASSERT(this->writerThread == NULL);
#endif
				::ReleaseSRWLockShared(&lock);
			}
			mutable SRWLOCK lock;
		};
#else
		class SharedLockImpl
		{
		public:
			SharedLockImpl(void)
			{
				pthread_rwlockattr_init(&attr);
				pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
				pthread_rwlock_init(&rwlock, &attr);
			}
			~SharedLockImpl(void)
			{
				pthread_rwlock_destroy(&rwlock);
				pthread_rwlockattr_destroy(&attr);
			}
			void Lock(void) const
			{
				int ret = pthread_rwlock_wrlock(&rwlock);

				DKASSERT_DESC_DEBUG(ret != EDEADLK, "The calling thread already owns the read/write lock.");
				DKASSERT_DESC_DEBUG(ret != EINVAL, "The value specified by rwlock is invalid.");
				DKASSERT_DESC_DEBUG(ret != ENOMEM, "Insufficient memory?");
			}
			bool TryLock(void) const
			{
				int ret = pthread_rwlock_trywrlock(&rwlock);

				DKASSERT_DESC_DEBUG(ret != EDEADLK, "The calling thread already owns the read/write lock.");
				DKASSERT_DESC_DEBUG(ret != EINVAL, "The value specified by rwlock is invalid.");
				DKASSERT_DESC_DEBUG(ret != ENOMEM, "Insufficient memory?");
				return ret == 0;
			}
			void Unlock(void) const
			{
				int ret = pthread_rwlock_unlock(&rwlock);
				DKASSERT_DESC_DEBUG(ret != EINVAL, "The value specified by rwlock is invalid.");
				DKASSERT_DESC_DEBUG(ret != EPERM, "The current thread does not own the lock.");
			}
			void LockShared(void) const
			{
				int ret = pthread_rwlock_rdlock(&rwlock);
				while (ret == EAGAIN) // waiting queue is full.
				{
					DKThread::Yield();
					ret = pthread_rwlock_rdlock(&rwlock);
				}

				DKASSERT_DESC_DEBUG(ret != EDEADLK, "The current thread already owns rwlock for writing.");
				DKASSERT_DESC_DEBUG(ret != EINVAL, "The value specified by rwlock is invalid.");
				DKASSERT_DESC_DEBUG(ret != ENOMEM, "Insufficient memory?");
			}
			bool TryLockShared(void) const
			{
				int ret = pthread_rwlock_tryrdlock(&rwlock);

				DKASSERT_DESC_DEBUG(ret != EDEADLK, "The current thread already owns rwlock for writing.");
				DKASSERT_DESC_DEBUG(ret != EINVAL, "The value specified by rwlock is invalid.");
				DKASSERT_DESC_DEBUG(ret != ENOMEM, "Insufficient memory?");
				return ret == 0;
			}
			void UnlockShared(void) const
			{
				int ret = pthread_rwlock_unlock(&rwlock);
				DKASSERT_DESC_DEBUG(ret != EINVAL, "The value specified by rwlock is invalid.");
				DKASSERT_DESC_DEBUG(ret != EPERM, "The current thread does not own the lock.");
			}
			mutable pthread_rwlock_t rwlock;
			pthread_rwlockattr_t attr;
		};
#endif
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKSharedLock::DKSharedLock(void)
{
	impl = reinterpret_cast<void*>(new SharedLockImpl());
	DKASSERT_DEBUG(impl != NULL);
}

DKSharedLock::~DKSharedLock(void)
{
	DKASSERT_DEBUG(impl != NULL);
	delete reinterpret_cast<SharedLockImpl*>(impl);
}

void DKSharedLock::LockShared(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<SharedLockImpl*>(impl)->LockShared();
}

bool DKSharedLock::TryLockShared(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<SharedLockImpl*>(impl)->TryLockShared();
}

void DKSharedLock::UnlockShared(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<SharedLockImpl*>(impl)->UnlockShared();
}

void DKSharedLock::Lock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<SharedLockImpl*>(impl)->Lock();
}

bool DKSharedLock::TryLock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<SharedLockImpl*>(impl)->TryLock();
}

void DKSharedLock::Unlock(void) const
{
	DKASSERT_DEBUG(impl != NULL);
	return reinterpret_cast<SharedLockImpl*>(impl)->Unlock();
}
