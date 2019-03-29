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
		DKSharedLock();
		~DKSharedLock();
		void LockShared() const;
		bool TryLockShared() const;
		void UnlockShared() const;
		void Lock() const;		///< exclusive lock
		bool TryLock() const;	///< exclusive try-lock
		void Unlock() const;	///< exclusive unlock

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
		~DKSharedLockReadOnlySection() { lock.UnlockShared(); }

	private:
		DKSharedLockReadOnlySection(const DKSharedLockReadOnlySection&) = delete;
		DKSharedLockReadOnlySection& operator = (const DKSharedLockReadOnlySection&) = delete;
		const DKSharedLock& lock;
	};
}
