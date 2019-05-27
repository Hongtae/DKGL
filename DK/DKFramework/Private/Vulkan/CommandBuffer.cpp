//
//  File: CommandBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "CommandBuffer.h"
#include "RenderCommandEncoder.h"
#include "ComputeCommandEncoder.h"
#include "CopyCommandEncoder.h"
#include "GraphicsDevice.h"
#include "CommandQueue.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

CommandBuffer::CommandBuffer(VkCommandPool p, DKCommandQueue* q)
	: commandPool(p)
	, queue(q)
{
	DKASSERT_DEBUG(commandPool);
}

CommandBuffer::~CommandBuffer()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkDevice device = dev->device;

    if (submitCommandBuffers.Count() > 0)
    {
        vkFreeCommandBuffers(device, commandPool, submitCommandBuffers.Count(), (const VkCommandBuffer*)submitCommandBuffers);
    }
	vkDestroyCommandPool(device, commandPool, dev->allocationCallbacks);
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(const DKRenderPassDescriptor& rp)
{
    DKASSERT_DEBUG(queue.SafeCast<CommandQueue>());
    CommandQueue* commandQueue = queue.StaticCast<CommandQueue>();
    if (commandQueue->family->properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
        DKObject<RenderCommandEncoder> encoder = DKOBJECT_NEW RenderCommandEncoder(this, rp);
        return encoder.SafeCast<DKRenderCommandEncoder>();
    }
    return nullptr;
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder()
{
    DKASSERT_DEBUG(queue.SafeCast<CommandQueue>());
    CommandQueue* commandQueue = queue.StaticCast<CommandQueue>();
    if (commandQueue->family->properties.queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        DKObject<ComputeCommandEncoder> encoder = DKOBJECT_NEW ComputeCommandEncoder(this);
        return encoder.SafeCast<DKComputeCommandEncoder>();
    }
    return nullptr;
}

DKObject<DKCopyCommandEncoder> CommandBuffer::CreateCopyCommandEncoder()
{
    DKObject<CopyCommandEncoder> encoder = DKOBJECT_NEW CopyCommandEncoder(this);
    return encoder.SafeCast<DKCopyCommandEncoder>();
}

bool CommandBuffer::Commit()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
    VkDevice device = dev->device;

    auto cleanup = [this, device](bool result)
    {
        if (submitCommandBuffers.Count() > 0)
            vkFreeCommandBuffers(device, commandPool, submitCommandBuffers.Count(), (const VkCommandBuffer*)submitCommandBuffers);

        submitInfos.Clear();
        submitCommandBuffers.Clear();
        submitWaitSemaphores.Clear();
        submitWaitStageMasks.Clear();
        submitSignalSemaphores.Clear();
        return result;
    };

    if (submitInfos.Count() != encoders.Count())
    {
        cleanup(false);

        // reserve storage for semaphores.
        size_t numWaitStageSemaphores = 0;
        size_t numSignalSemaphores = 0;
        for (CommandEncoder* encoder : encoders)
        {
            numWaitStageSemaphores += encoder->waitStageSemaphores.Count();
            numSignalSemaphores += encoder->signalSemaphores.Count();
        }
        submitWaitSemaphores.Reserve(numWaitStageSemaphores);
        submitWaitStageMasks.Reserve(numWaitStageSemaphores);
        submitSignalSemaphores.Reserve(numSignalSemaphores);

        submitCommandBuffers.Reserve(encoders.Count());
        submitInfos.Reserve(encoders.Count());

        for (CommandEncoder* encoder : encoders)
        {
            size_t commandBuffersOffset = submitCommandBuffers.Count();
            size_t waitSemaphoresOffset = submitWaitSemaphores.Count();
            size_t signalSemaphoresOffset = submitSignalSemaphores.Count();
            DKASSERT_DEBUG(submitWaitStageMasks.Count() == waitSemaphoresOffset);

            VkCommandBufferAllocateInfo  bufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            bufferInfo.commandPool = commandPool;
            bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            bufferInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
            VkResult err = vkAllocateCommandBuffers(device, &bufferInfo, &commandBuffer);
            if (err != VK_SUCCESS)
            {
                DKLogE("ERROR: vkAllocateCommandBuffers failed: %s", VkResultCStr(err));
                return cleanup(false);
            }
            submitCommandBuffers.Add(commandBuffer);

            encoder->waitStageSemaphores.EnumerateForward([&](decltype(encoder->waitStageSemaphores)::Pair& pair)
            {
                VkSemaphore semaphore = pair.key;
                VkPipelineStageFlags stages = pair.value;

                DKASSERT_DEBUG(semaphore);
                DKASSERT_DEBUG((stages & VK_PIPELINE_STAGE_HOST_BIT) == 0);

                submitWaitSemaphores.Add(semaphore);
                submitWaitStageMasks.Add(stages);
            });
            encoder->signalSemaphores.EnumerateForward([&](VkSemaphore semaphore)
            {
                DKASSERT_DEBUG(semaphore);
                submitSignalSemaphores.Add(semaphore);
            });

            VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
            bool result = encoder->Encode(commandBuffer);
            vkEndCommandBuffer(commandBuffer);

            if (!result)
            {
                return cleanup(false);
            }

            DKASSERT_DEBUG(submitWaitSemaphores.Count() == submitWaitStageMasks.Count());

            VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
            if (submitCommandBuffers.Count() > commandBuffersOffset)
            {
                uint32_t count = submitCommandBuffers.Count() - commandBuffersOffset;
                VkCommandBuffer* commandBuffers = submitCommandBuffers;
                submitInfo.commandBufferCount = count;
                submitInfo.pCommandBuffers = &commandBuffers[commandBuffersOffset];
            }
            if (submitWaitSemaphores.Count() > waitSemaphoresOffset)
            {
                uint32_t count = submitWaitSemaphores.Count() - waitSemaphoresOffset;
                VkSemaphore* semaphores = submitWaitSemaphores;
                VkPipelineStageFlags* stages = submitWaitStageMasks;
                submitInfo.waitSemaphoreCount = count;
                submitInfo.pWaitSemaphores = &semaphores[waitSemaphoresOffset];
                submitInfo.pWaitDstStageMask = &stages[waitSemaphoresOffset];
            }
            if (submitSignalSemaphores.Count() > signalSemaphoresOffset)
            {
                uint32_t count = submitSignalSemaphores.Count() - signalSemaphoresOffset;
                VkSemaphore* semaphores = submitSignalSemaphores;
                submitInfo.signalSemaphoreCount = count;
                submitInfo.pSignalSemaphores = &semaphores[signalSemaphoresOffset];
            }
            submitInfos.Add(submitInfo);
        }
    }

    if (submitInfos.Count() > 0)
    {
        CommandQueue* commandQueue = this->queue.StaticCast<CommandQueue>();
        bool result = commandQueue->Submit(submitInfos, submitInfos.Count(),
                                           DKFunction([](DKObject<CommandBuffer> cb) mutable
        {
            DKASSERT_DEBUG(cb);
            cb = nullptr;
        })->Invocation(this));
        return result;
    }
    return true;
}

void CommandBuffer::EndEncoder(DKCommandEncoder*, CommandEncoder* encoder)
{
    this->encoders.Add(encoder);
}

QueueFamily* CommandBuffer::QueueFamily()
{
    DKASSERT_DEBUG(queue.SafeCast<CommandQueue>() != nullptr);
    CommandQueue* cq = queue.StaticCast<CommandQueue>();
    return cq->family;
}

#endif //#if DKGL_ENABLE_VULKAN
