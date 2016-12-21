//
//  File: DKSharedLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// @brief rw-lock implementation with common interface.
	///
	/// LockShared(), read-lock, can be locked by some threads concurrently.
	/// Lock (write-lock, exclusive), only one thread can have lock.
	///
	/// @note
	///  Only exclusive locking works with DKCriticalSecton.
	///  Use DKSharedLockReadOnlySecton instead for shared-locking with scoped context.
	class DKGL_API DKSharedLock
	{
	public:
		DKSharedLock(void);
		~DKSharedLock(void);
		void LockShared(void) const;
		bool TryLockShared(void) const;
		void UnlockShared(void) const;
		void Lock(void) const;		///< exclusive lock
		bool TryLock(void) const;	///< exclusive try-lock
		void Unlock(void) const;	///< exclusive unlock

	private:
		DKSharedLock(const DKSharedLock&) = delete;
		DKSharedLock& operator = (const DKSharedLock&) = delete;
		void* impl;
	};

	/// context scope based helper class for DKSharedLock
	/// use DKCriticalSecton for exclusive lock.
	class DKSharedLockReadOnlySection
	{
	public:
		DKSharedLockReadOnlySection(const DKSharedLock& sl) : lock(sl) { lock.LockShared(); }
		~DKSharedLockReadOnlySection(void) { lock.UnlockShared(); }

	private:
		DKSharedLockReadOnlySection(const DKSharedLockReadOnlySection&) = delete;
		DKSharedLockReadOnlySection& operator = (const DKSharedLockReadOnlySection&) = delete;
		const DKSharedLock& lock;
	};
}
