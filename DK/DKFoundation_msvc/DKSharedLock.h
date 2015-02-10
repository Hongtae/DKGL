//
//  File: DKSharedLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKSharedLock
// rw-lock implementation with common interface.
//
// LockShared(), read-lock, can be locked by some threads concurrently.
// Lock (write-lock, exclusive), only one thread can have lock.
//
// Note:
//  Only exclusive locking works with DKCriticalSecton.
//  Use DKSharedLockReadOnlySecton instead for shared-locking with scoped context.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKSharedLock
	{
	public:
		DKSharedLock(void);
		~DKSharedLock(void);
		void LockShared(void) const;
		bool TryLockShared(void) const;
		void UnlockShared(void) const;
		void Lock(void) const;		// exclusive
		bool TryLock(void) const;	// exclusive
		void Unlock(void) const;	// exclusive

	private:
		DKSharedLock(const DKSharedLock&);
		DKSharedLock& operator = (const DKSharedLock&);
		void* impl;
	};

	// context scope based helper class.
	// use DKCriticalSecton for exclusive lock.
	class DKSharedLockReadOnlySection
	{
	public:
		DKSharedLockReadOnlySection(const DKSharedLock& sl) : lock(sl) { lock.LockShared(); }
		~DKSharedLockReadOnlySection(void) { lock.UnlockShared(); }

	private:
		DKSharedLockReadOnlySection(const DKSharedLockReadOnlySection&);
		DKSharedLockReadOnlySection& operator = (const DKSharedLockReadOnlySection&);
		const DKSharedLock& lock;
	};
}
