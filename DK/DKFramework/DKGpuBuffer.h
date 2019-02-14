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

		virtual ~DKGpuBuffer();

        virtual void* Contents() = 0;
        virtual void Flush(size_t offset = 0, size_t size = ~size_t(0)) = 0;
        virtual size_t Length() const = 0;

	protected:
		DKGpuBuffer();
	};
}
