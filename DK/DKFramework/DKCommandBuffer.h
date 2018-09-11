//
//  File: DKCommandBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKRenderCommandEncoder.h"
#include "DKComputeCommandEncoder.h"
#include "DKBlitCommandEncoder.h"

namespace DKFramework
{
	class DKGraphicsDevice;
	class DKCommandQueue;
	/// @brief GPU command buffer
	class DKCommandBuffer
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

		virtual ~DKCommandBuffer() {}

		virtual DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(const DKRenderPassDescriptor&) = 0;
		virtual DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder() = 0;
		virtual DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder() = 0;

		virtual bool Commit() = 0;

		virtual DKCommandQueue* Queue() = 0;

		DKGraphicsDevice* Device();
	};
}
