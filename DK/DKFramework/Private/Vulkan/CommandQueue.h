//
//  File: CommandQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
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
				DKObject<DKDrawable> CreateDrawable(DKWindow*) override;

				DKGraphicsDevice* Device(void) override { return device; }

				QueueFamily* family;
				VkQueue queue;

				DKObject<DKGraphicsDevice> device;
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
