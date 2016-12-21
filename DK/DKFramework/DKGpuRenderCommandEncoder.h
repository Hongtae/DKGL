//
//  File: DKGpuRenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	/// @brief Command encoder for GPU render operation.
	class DKGpuRenderCommandEncoder
	{
	public:
		virtual ~DKGpuRenderCommandEncoder(void) {}
	};
	/// @brief Multi-threaded command encoder for GPU render operation.
	class DKParallelGpuRenderCommandEncoder
	{
	public:
		virtual ~DKParallelGpuRenderCommandEncoder(void) {}
		virtual DKObject<DKGpuRenderCommandEncoder> CreateEncoder(void) = 0;
	};
}
