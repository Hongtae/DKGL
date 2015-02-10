//
//  File: DKCriticalSection.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKCriticalSection<T>
// synchronization utility. proved automatic locking with context scope based.
// use combination with DKLock, DKMutex, DKSpinLock, DKSharedLock, etc.
//
// this object ensures context section to be thread-safety.
// managing this object's life-cycles to control context locking range.
//
// lock with object creation and unlock when destroyed,
// to ensure mutually-exclusive section based on context scope.
//
// Example:
//  if ( .. )
//  {
//      DKCriticalSection<DKSpinLock> guard(mySpinLockObject);
//      // mutually-exclusive section from here.
//
//  }
//
// Note:
//  Do not confuse with Win32 CriticalSection object, this is unrelated to that.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename T> class DKCriticalSection
	{
	public:
		DKCriticalSection(const T& lockObject)
			: lock(lockObject)
		{
			lock.Lock();
		}
		~DKCriticalSection(void)
		{
			lock.Unlock();
		}
	private:
		DKCriticalSection(const DKCriticalSection&);
		DKCriticalSection& operator = (const DKCriticalSection&);
		const T& lock;
	};
}
