//
//  File: DKMutex.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKMutex
// a mutex object.
// recursive locking operation not supported.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKMutex
	{
	public:
		DKMutex(void);
		~DKMutex(void);
		void Lock(void) const;
		bool TryLock(void) const;
		void Unlock(void) const;

	private:
		DKMutex(const DKMutex&);
		DKMutex& operator = (const DKMutex&);
		void* impl;
	};
}
