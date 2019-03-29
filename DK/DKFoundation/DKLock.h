//
//  File: DKLock.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// @brief recursive-lock.
	class DKGL_API DKLock
	{
	public:
		DKLock();
		~DKLock();
		void Lock() const;
		bool TryLock() const;
		void Unlock() const;

	private:
		DKLock(const DKLock&) = delete;
		DKLock& operator = (const DKLock&) = delete;
		void* impl;
	};
}	
