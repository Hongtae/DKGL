//
//  File: DKGpuCommandBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

#include "DKGpuRenderCommandEncoder.h"
#include "DKGpuComputeCommandEncoder.h"
#include "DKGpuBlitCommandEncoder.h"

////////////////////////////////////////////////////////////////////////////////
// DKGpuCommandBuffer
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKRenderPassDescriptor;

	class DKGpuCommandBuffer
	{
	public:
		enum class Status
		{
			NotEnqueued = 0,
			Enqueued,
			Committed,
			Scheduled,
			Completed,
			Error,
		};

		virtual ~DKGpuCommandBuffer(void) {}

		virtual DKObject<DKGpuRenderCommandEncoder> CreateRenderCommandEncoder(DKRenderPassDescriptor*) = 0;
		virtual DKObject<DKParallelGpuRenderCommandEncoder> CreateParallelRenderCommandEncoder(DKRenderPassDescriptor*) = 0;

		virtual DKObject<DKGpuComputeCommandEncoder> CreateComputeCommandEncoder(void) = 0;
		virtual DKObject<DKGpuBlitCommandEncoder> CreateBlitCommandEncoder(void) = 0;
	};
}
