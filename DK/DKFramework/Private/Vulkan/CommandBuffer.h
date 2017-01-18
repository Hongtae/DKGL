//
//  File: CommandBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKCommandBuffer.h"
#include "../../DKCommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class CommandBuffer : public DKCommandBuffer
			{
			public:
				~CommandBuffer(void);

				DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(DKRenderPassDescriptor*) override;
				DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder(void) override;
				DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder(void) override;

				bool Commit(void) override;
				bool WaitUntilCompleted(void) override;

				DKCommandQueue* Queue(void) override { return queue; };

				DKObject<DKCommandQueue> queue;
			};
		}
	}
}

#endif //#if DKGL_USE_VULKAN
