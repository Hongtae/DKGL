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
    , commandBuffer(VK_NULL_HANDLE)
{
	DKASSERT_DEBUG(commandPool);
}

CommandBuffer::~CommandBuffer()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkDevice device = dev->device;

    if (commandBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        commandBuffer = VK_NULL_HANDLE;
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
	bool result = false;

    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
    VkDevice device = dev->device;

    if (commandBuffer == VK_NULL_HANDLE)
    {
        if (encoders.Count() > 0)
        {
            VkCommandBufferAllocateInfo  bufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            bufferInfo.commandPool = commandPool;
            bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            bufferInfo.commandBufferCount = 1;

            VkResult err = vkAllocateCommandBuffers(device, &bufferInfo, &commandBuffer);
            if (err != VK_SUCCESS)
            {
                DKLogE("ERROR: vkAllocateCommandBuffers failed: %s", VkResultCStr(err));
                return false;
            }

            // encode buffer!
            VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
            for (CommandEncoder* enc : encoders)
            {
                if (!enc->Encode(commandBuffer))
                {
                    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
                    commandBuffer = VK_NULL_HANDLE;
                    return false;
                }
            }
            vkEndCommandBuffer(commandBuffer);
        }
    }

    if (commandBuffer)
    {
        DKArray<VkSemaphore>			submitWaitSemaphores;
        DKArray<VkPipelineStageFlags>	submitWaitStageMasks;
        DKArray<VkSemaphore>			submitSignalSemaphores;

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        submitWaitSemaphores.Reserve(this->semaphorePipelineStageMasks.Count());
        submitWaitStageMasks.Reserve(this->semaphorePipelineStageMasks.Count());

        this->semaphorePipelineStageMasks.EnumerateForward([&](decltype(semaphorePipelineStageMasks)::Pair& pair)
        {
            submitWaitSemaphores.Add(pair.key);
            submitWaitStageMasks.Add(pair.value);
        });

        submitSignalSemaphores.Reserve(this->signalSemaphores.Count());
        this->signalSemaphores.EnumerateForward([&](VkSemaphore semaphore)
        {
            submitSignalSemaphores.Add(semaphore);
        });

        DKASSERT_DEBUG(submitWaitSemaphores.Count() == submitWaitStageMasks.Count());

        submitInfo.waitSemaphoreCount = submitWaitSemaphores.Count();
        submitInfo.pWaitSemaphores = submitWaitSemaphores;
        submitInfo.pWaitDstStageMask = submitWaitStageMasks;
        submitInfo.signalSemaphoreCount = submitSignalSemaphores.Count();
        submitInfo.pSignalSemaphores = submitSignalSemaphores;

        CommandQueue* commandQueue = this->queue.StaticCast<CommandQueue>();
        result = commandQueue->Submit(&submitInfo, 1,
                                      DKFunction([=](DKObject<CommandBuffer> cb) mutable
        {
            if (cb)
                cb = nullptr;
        })->Invocation(this));
    }
    return result;
}

void CommandBuffer::AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags flags)
{
    if (semaphore != VK_NULL_HANDLE)
    {
        if (!semaphorePipelineStageMasks.Insert(semaphore, flags))
            semaphorePipelineStageMasks.Value(semaphore) |= flags;
    }
}

void CommandBuffer::AddSignalSemaphore(VkSemaphore semaphore)
{
    if (semaphore != VK_NULL_HANDLE)
        signalSemaphores.Insert(semaphore);
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
