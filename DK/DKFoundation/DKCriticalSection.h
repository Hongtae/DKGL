//
//  File: DKCriticalSection.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/**
	 @brief Synchronization utility. proved automatic locking with context scope based.
	 Use combination with DKLock, DKMutex, DKSpinLock, DKSharedLock, etc.
	
	 this object ensures context section to be thread-safety.
	 managing this object's life-cycles to control context locking range.
	
	 lock with object creation and unlock when destroyed,
	 to ensure mutually-exclusive section based on context scope.
	
	 @code
	  if ( .. )
	  {
	      DKCriticalSection<DKSpinLock> guard(mySpinLockObject);
	      // mutually-exclusive section from here.
	
	  }
	 @endocde
	
	 @note
	  Do not confuse with Win32 CriticalSection object, this is unrelated to that.
	 */
	template <typename T> class DKCriticalSection
	{
	public:
		DKCriticalSection(const T& lockObject)
			: lock(lockObject)
		{
			lock.Lock();
		}
		~DKCriticalSection()
		{
			lock.Unlock();
		}
	private:
		DKCriticalSection(const DKCriticalSection&) = delete;
		DKCriticalSection& operator = (const DKCriticalSection&) = delete;
		const T& lock;
	};

	template <typename T> class DKScopedLock
	{
	public:
		DKScopedLock(const T& lockObject)
			: lock(&lockObject)
		{
			lock->Lock();
		}
		~DKScopedLock()
		{
			if (lock)
				lock->Unlock();
		}
		void Unlock()
		{
			if (lock)
				lock->Unlock();
			lock = nullptr;
		}
	private:
		const T* lock;
	};
}
