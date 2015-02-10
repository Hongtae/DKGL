//
//  File: DKSpinLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKAtomicNumber32.h"

////////////////////////////////////////////////////////////////////////////////
// DKSpinLock
// a busy-waiting locking class.
// atomic variable used internally.
// use this class for short period locking.
// (such as small computation, without I/O.)
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKSpinLock
	{
	public:
		DKSpinLock(void);
		~DKSpinLock(void);
		void Lock(void) const;
		bool TryLock(void) const;
		void Unlock(void) const;

	private:
		DKSpinLock(const DKSpinLock&);
		DKSpinLock& operator = (const DKSpinLock&);
		mutable DKAtomicNumber32 state;
	};
}
