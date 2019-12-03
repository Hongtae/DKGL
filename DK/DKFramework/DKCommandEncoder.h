//
//  File: DKCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
    class DKGpuEvent;
    class DKGpuSemaphore;
	class DKCommandBuffer;
	/// @brief Command encoder for GPU compute operation
	class DKCommandEncoder
	{
	public:
		virtual ~DKCommandEncoder() {}

		virtual void EndEncoding() = 0;
		virtual bool IsCompleted() const = 0;
		virtual DKCommandBuffer* CommandBuffer() = 0;

        virtual void WaitEvent(DKGpuEvent*) = 0;
        virtual void SignalEvent(DKGpuEvent*) = 0;

        virtual void WaitSemaphoreValue(DKGpuSemaphore*, uint64_t) = 0;
        virtual void SignalSemaphoreValue(DKGpuSemaphore*, uint64_t) = 0;
    };
}
