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
#include "BlitCommandEncoder.h"
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
    submitCallbacks.Clear();
    completedCallbacks.Clear();

	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkDevice device = dev->device;

	vkDestroyCommandPool(device, commandPool, dev->allocationCallbacks);
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(const DKRenderPassDescriptor& rp)
{
	VkCommandBuffer buffer = GetEncodingBuffer();
	if (buffer)
	{
		DKObject<RenderCommandEncoder> encoder = DKOBJECT_NEW RenderCommandEncoder(buffer, this, rp);
		return encoder.SafeCast<DKRenderCommandEncoder>();
	}
	return nullptr;
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder()
{
	VkCommandBuffer buffer = GetEncodingBuffer();
	if (buffer)
	{
		DKObject<ComputeCommandEncoder> encoder = DKOBJECT_NEW ComputeCommandEncoder(buffer, this);
		return encoder.SafeCast<DKComputeCommandEncoder>();
	}
	return nullptr;
}

DKObject<DKBlitCommandEncoder> CommandBuffer::CreateBlitCommandEncoder()
{
	VkCommandBuffer buffer = GetEncodingBuffer();
	if (buffer)
	{
		DKObject<BlitCommandEncoder> encoder = DKOBJECT_NEW BlitCommandEncoder(buffer, this);
		return encoder.SafeCast<DKBlitCommandEncoder>();
	}
	return nullptr;
}

bool CommandBuffer::Commit()
{
	bool result = false;
	if (submitInfos.Count() > 0)
	{
		CommandQueue* commandQueue = this->queue.StaticCast<CommandQueue>();

        for (DKOperation* op : submitCallbacks)
            op->Perform();

		DKArray<DKObject<DKOperation>> callbacksCopy = (completedCallbacks);

		GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
		VkDevice device = dev->device;

		result = commandQueue->Submit(submitInfos, submitInfos.Count(),
							 DKFunction([=](DKObject<CommandBuffer> cb) mutable
		{
			for (DKOperation* op : callbacksCopy)
				op->Perform();
			callbacksCopy.Clear();

		})->Invocation(this));
	}
	return result;
}

VkCommandBuffer CommandBuffer::GetEncodingBuffer()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkDevice device = dev->device;

	VkCommandBufferAllocateInfo  bufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	bufferInfo.commandPool = commandPool;
	bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferInfo.commandBufferCount = 1;

	VkCommandBuffer buffer = nullptr;
	VkResult err = vkAllocateCommandBuffers(device, &bufferInfo, &buffer);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkAllocateCommandBuffers failed: %s", VkResultCStr(err));
		return NULL;
	}
	return buffer;
}

void CommandBuffer::ReleaseEncodingBuffer(VkCommandBuffer cb)
{
	if (cb)
	{
		GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
		VkDevice device = dev->device;
		vkFreeCommandBuffers(device, commandPool, 1, &cb);
	}
}

void CommandBuffer::Submit(const VkSubmitInfo& info, DKOperation* cb1, DKOperation* cb2)
{
	submitInfos.Add(info);
    if (cb1)
        submitCallbacks.Add(cb1);
    if (cb2)
        completedCallbacks.Add(cb2);
}

#endif //#if DKGL_ENABLE_VULKAN
