//
//  File: DKLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKLock
// provide recursive-lock feature.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKLock
	{
	public:
		DKLock(void);
		~DKLock(void);
		void Lock(void) const;
		bool TryLock(void) const;
		void Unlock(void) const;

	private:
		DKLock(const DKLock&);
		DKLock& operator = (const DKLock&);
		void* impl;
	};
}
