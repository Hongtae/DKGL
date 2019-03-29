//
//  File: DKSpinLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKAtomicNumber32.h"

namespace DKFoundation
{
	/// a busy-waiting locking class.
	/// atomic variable used internally.
	/// use this class for short period locking.
	/// (such as small computation, without I/O.)
	class DKGL_API DKSpinLock
	{
	public:
		DKSpinLock();
		~DKSpinLock();
		void Lock() const;
		bool TryLock() const;
		void Unlock() const;

	private:
		DKSpinLock(const DKSpinLock&) = delete;
		DKSpinLock& operator = (const DKSpinLock&) = delete;
		mutable DKAtomicNumber32 state;
	};
}
