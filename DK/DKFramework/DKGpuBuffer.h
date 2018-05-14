//
//  File: DKGpuBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//


#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	class DKGpuBuffer
	{
	public:
		enum StorageMode
		{
			StorageModeShared = 0, // accessible to both the CPU and the GPU
			StorageModePrivate, // only accessible to the GPU
		};

		virtual ~DKGpuBuffer(void);

		virtual void* Lock(void) = 0;
		virtual void Unlock(void) = 0;

	protected:
		DKGpuBuffer(void);
	};
}
