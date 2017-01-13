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
	class DKGL_API DKCommandBuffer
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

		virtual ~DKCommandBuffer(void);

		virtual DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(DKRenderPassDescriptor*) = 0;
		virtual DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder(void) = 0;
		virtual DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder(void) = 0;

		virtual bool Commit(void) = 0;
		virtual void WaitUntilCompleted(void) = 0;
		virtual bool WaitUntilCompleted(double timeout) = 0;

		virtual DKCommandQueue* Queue(void) = 0;

		DKGraphicsDevice* Device(void);
	};
}
