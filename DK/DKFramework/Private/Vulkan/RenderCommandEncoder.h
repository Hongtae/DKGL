//
//  File: RenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKRenderCommandEncoder.h"

#include "CommandBuffer.h"
#include "SwapChain.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class RenderCommandEncoder : public DKRenderCommandEncoder
			{
			public:
				RenderCommandEncoder(VkCommandBuffer, CommandBuffer*, const DKRenderPassDescriptor&);
				~RenderCommandEncoder(void);

				void EndEncoding(void) override;
				DKCommandBuffer* Buffer(void) override;

				struct Resources
				{
					VkFramebuffer		framebuffer;
					VkRenderPass		renderPass;

					DKArray<VkSemaphore>			waitSemaphores;
					DKArray<VkPipelineStageFlags>	waitStageMasks;
					DKArray<VkSemaphore>			signalSemaphores;

					CommandBuffer* cb;
					VkCommandBuffer commandBuffer;

					Resources(CommandBuffer*);
					~Resources(void);
				};

				void AddWaitSemaphore(VkSemaphore, VkPipelineStageFlags);
				void AddSignalSemaphore(VkSemaphore);

			private:

				DKMap<VkSemaphore, VkPipelineStageFlags> semaphorePipelineStageMasks;
				DKSet<VkSemaphore> signalSemaphores;

				DKObject<Resources> resources;
				DKObject<CommandBuffer> commandBuffer;
			};
		}
	}
}
#endif //#if DKGL_ENABLE_VULKAN
