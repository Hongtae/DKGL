//
//  File: DKSingleton.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKSpinLock.h"

namespace DKFoundation
{
	/// @brief singleton class.
	/// Only one instance can be created and exists at run time.
	///
	/// @note
	///  You can define your class with 'typedef' or sublcass from DKSingleton.
	///  subclassing is recommended.
	/// @see DKSharedInstance
	template <class TYPE> class DKSingleton
	{
	public:
		static TYPE& Instance(void)
		{
			static TYPE* instancePtr = NULL;
			if (instancePtr == NULL)
			{
				static DKSpinLock lock;
				lock.Lock();
				if (instancePtr == NULL)
				{
					static TYPE obj;
					instancePtr = &obj;
				}
				lock.Unlock();
			}
			return *instancePtr;
		}
	protected:
		DKSingleton(void) {}
		virtual ~DKSingleton(void) {}
	private:
		DKSingleton(const DKSingleton&) = delete;
		DKSingleton& operator = (const DKSingleton&) = delete;
	};
}
