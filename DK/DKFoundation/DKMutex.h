//
//  File: DKMutex.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// @brief a mutex object.
	/// recursive locking operation not supported.
	class DKGL_API DKMutex
	{
	public:
		DKMutex(void);
		~DKMutex(void);
		void Lock(void) const;
		bool TryLock(void) const;
		void Unlock(void) const;

	private:
		DKMutex(const DKMutex&) = delete;
		DKMutex& operator = (const DKMutex&) = delete;
		void* impl;
	};
}
