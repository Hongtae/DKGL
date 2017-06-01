//
//  File: CommandQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKCommandQueue.h"
#include "../../DKGraphicsDevice.h"

#include "QueueFamily.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class CommandQueue : public DKCommandQueue
			{
			public:
				CommandQueue(DKGraphicsDevice*, QueueFamily*, VkQueue);
				~CommandQueue(void);

				DKObject<DKCommandBuffer> CreateCommandBuffer(void) override;
				DKObject<DKSwapChain> CreateSwapChain(DKWindow*) override;

				bool Submit(const VkSubmitInfo* submits, uint32_t submitCount, DKOperation* callback);

				bool WaitIdle(void);

				DKGraphicsDevice* Device(void) override { return device; }

				QueueFamily* family;
				VkQueue queue;

				DKObject<DKGraphicsDevice> device;
			};
		}
	}
}
#endif //#if DKGL_ENABLE_VULKAN
