//
//  File: CommandBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKCommandBuffer.h"
#include "../../DKCommandQueue.h"
#include "QueueFamily.h"

namespace DKFramework::Private::Vulkan
{
    class CommandBuffer;
    class CommandEncoder
    {
    public:
        enum { InitialNumberOfCommands = 128 };
        virtual ~CommandEncoder() {}
        virtual bool Encode(VkCommandBuffer) = 0;
    };

	class CommandBuffer : public DKCommandBuffer
	{
	public:
		CommandBuffer(VkCommandPool, DKCommandQueue*);
		~CommandBuffer();

		DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(const DKRenderPassDescriptor&) override;
		DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder() override;
		DKObject<DKCopyCommandEncoder> CreateCopyCommandEncoder() override;

		bool Commit() override;

		DKCommandQueue* Queue() override { return queue; };

        QueueFamily* QueueFamily();
        void AddWaitSemaphore(VkSemaphore, VkPipelineStageFlags);
        void AddSignalSemaphore(VkSemaphore);

        void EndEncoder(DKCommandEncoder*, CommandEncoder*);

    private:
        VkCommandPool commandPool;
        DKObject<DKCommandQueue> queue;

        DKArray<DKObject<CommandEncoder>> encoders;

        DKMap<VkSemaphore, VkPipelineStageFlags> semaphorePipelineStageMasks;
        DKSet<VkSemaphore> signalSemaphores;

        VkCommandBuffer commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
