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

        struct WaitTimelineSemaphoreStageValue
        {
            VkPipelineStageFlags stages;
            uint64_t value; // 0 for non-timeline semaphore (binary semaphore)
        };
        DKMap<VkSemaphore, WaitTimelineSemaphoreStageValue> waitSemaphores;
        DKMap<VkSemaphore, uint64_t> signalSemaphores;

        virtual ~CommandEncoder() {}
        virtual bool Encode(VkCommandBuffer) = 0;

        void AddWaitSemaphore(VkSemaphore semaphore, uint64_t value, VkPipelineStageFlags flags)
        {
            if (semaphore != VK_NULL_HANDLE)
            {
                if (!waitSemaphores.Insert(semaphore, { flags, value }))
                {
                    auto p = waitSemaphores.Find(semaphore);
                    {
                        if (value > p->value.value)
                            p->value.value = value;
                        p->value.stages |= flags;
                    }
                }
            }
        }
        void AddSignalSemaphore(VkSemaphore semaphore, uint64_t value)
        {
            if (semaphore != VK_NULL_HANDLE)
            {
                if (!signalSemaphores.Insert(semaphore, value))
                {
                    auto p = signalSemaphores.Find(semaphore);
                    DKASSERT_DEBUG(p);
                    if (value > p->value)
                        p->value = value;
                }
            }
        }
    };

	class CommandBuffer : public DKCommandBuffer
	{
	public:
		CommandBuffer(VkCommandPool, DKCommandQueue*);
		~CommandBuffer();

		DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(const DKRenderPassDescriptor&) override;
		DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder() override;
		DKObject<DKCopyCommandEncoder> CreateCopyCommandEncoder() override;

        void AddCompletedHandler(DKOperation*) override;
		bool Commit() override;

		DKCommandQueue* Queue() override { return queue; };

        QueueFamily* QueueFamily();

        void EndEncoder(DKCommandEncoder*, CommandEncoder*);

    private:
        VkCommandPool commandPool;
        DKObject<DKCommandQueue> queue;
        DKArray<DKObject<CommandEncoder>> encoders;

        DKArray<VkSubmitInfo>           submitInfos;
        DKArray<VkCommandBuffer>        submitCommandBuffers;
        DKArray<VkSemaphore>	            submitWaitSemaphores;
        DKArray<VkPipelineStageFlags>	submitWaitStageMasks;
        DKArray<VkSemaphore>            submitSignalSemaphores;

        DKArray<uint64_t>               submitWaitTimelineSemaphoreValues;
        DKArray<uint64_t>               submitSignalTimelineSemaphoreValues;
        DKArray<VkTimelineSemaphoreSubmitInfoKHR> submitTimelineSemaphoreInfos;

        DKArray<DKObject<DKOperation>> completedHandlers;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
