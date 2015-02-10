//
//  File: DKSingleton.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKSpinLock.h"

////////////////////////////////////////////////////////////////////////////////
// DKSingleton
// singleton class.
// Only one instance can be created and exists at run time.
//
// Note:
//  You can define your class with 'typedef' or sublcass from DKSingleton.
//  subclassing is recommended.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
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
	private:
		DKSingleton(const DKSingleton&);
		DKSingleton& operator = (const DKSingleton&);
	protected:
		DKSingleton(void)					{}
		virtual ~DKSingleton(void)			{}
	};
}
