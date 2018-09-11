//
//  File: DKDummyLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// @brief this is dummy lock object. have no locking feature.
	/// use this object for single-threaded app or context not shared by threads.
	///
	/// @note
	///  You can create your own locking object which can be used in template container classes.
	///  to create your own locking class, you need provide Lock, TryLock, Unlock functions
	class DKDummyLock
	{
	public:
		DKDummyLock() {}
		~DKDummyLock() {}
		void Lock() const {}
		bool TryLock() const {return true;}
		void Unlock() const {}

	private:
		DKDummyLock(const DKDummyLock&) = delete;
		DKDummyLock& operator = (const DKDummyLock&) = delete;
	};
}
