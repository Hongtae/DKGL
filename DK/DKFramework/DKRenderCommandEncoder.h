//
//  File: DKRenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	/// @brief Command encoder for GPU render operation.
	class DKRenderCommandEncoder
	{
	public:
		virtual ~DKRenderCommandEncoder(void) {}
	};
	/// @brief Multi-threaded command encoder for GPU render operation.
	class DKParallelRenderCommandEncoder
	{
	public:
		virtual ~DKParallelRenderCommandEncoder(void) {}
		virtual DKObject<DKRenderCommandEncoder> CreateEncoder(void) = 0;
	};
}
