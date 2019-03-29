//
//  File: DKFence.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/**
	 @brief
	 A simple locking object. can not be used with DKCriticalSection together.

	 Usage:
	 @code
	  if (...)
	  {
		   DKFence fence(this);  // locking with key(this)
		   .. mutually exclusive below scope ..
		   .. do something thread sensitive ..

	  } // unlock automatically while fence object being destructed.
	 @endcode
	 */
	class DKGL_API DKFence
	{
	public:
		/// anything can be a key, but should be unique.
		DKFence(const void* key, bool exclusive = false);
		~DKFence();
		
	private:
		DKFence(const DKFence&) = delete;
		DKFence& operator = (const DKFence&) = delete;
		const void* key;
	};
}
