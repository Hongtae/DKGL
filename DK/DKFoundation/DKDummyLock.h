//
//  File: DKDummyLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKDummyLock
// this is dummy lock object. have no locking feature.
// use this object for single-threaded app or context not shared by threads.
//
// Note:
//  You can create your own locking object which can be used in template container classes.
//  to create your own locking class, you need provide Lock, TryLock, Unlock functions
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKDummyLock
	{
	public:
		DKDummyLock(void) {}
		~DKDummyLock(void) {}
		void Lock(void) const {}
		bool TryLock(void) const {return true;}
		void Unlock(void) const {}

	private:
		DKDummyLock(const DKDummyLock&);
		DKDummyLock& operator = (const DKDummyLock&);
	};
}
