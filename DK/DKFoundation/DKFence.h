//
//  File: DKFence.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

////////////////////////////////////////////////////////////////////////////////
// DKFence
// a simple locking object. can not be used with DKCriticalSection together.
//
// Usage:
//  if (...)
//  {
//       DKFence fence(this);  // locking with key(this)
//       .. mutually exclusive below scope ..
//       .. do something thread sensitive ..
//
//  } // unlock automatically while fence object being destructed.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKFence
	{
	public:
		// anything can be a key, but should be unique.
		DKFence(const void* key, bool exclusive = false);
		~DKFence(void);
		
	private:
		// copy constructor not allowed.
		DKFence(const DKFence&);
		DKFence& operator = (const DKFence&);
		const void* key;
	};
}
