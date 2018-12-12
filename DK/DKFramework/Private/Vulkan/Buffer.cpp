//
//  File: Buffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2018 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Buffer.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

Buffer::Buffer(DKGraphicsDevice* dev, VkBuffer b, VkBufferView v, VkDeviceMemory mem, VkMemoryType memType, size_t len)
	: device(dev)
	, buffer(b)
	, view(v)
	, memory(mem)
	, memoryType(memType)
	, length(len)
	, mapped(nullptr)
{
	DKASSERT_DEBUG(length > 0);
}

Buffer::~Buffer()
{
	DKASSERT_DEBUG(mapped == nullptr);

	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	if (view)
		vkDestroyBufferView(dev->device, view, dev->allocationCallbacks);
	if (buffer)
		vkDestroyBuffer(dev->device, buffer, dev->allocationCallbacks);
	if (memory)
		vkFreeMemory(dev->device, memory, dev->allocationCallbacks);
}

void* Buffer::Lock(size_t offset, size_t size)
{
	if (offset < length &&
		memory &&
		memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		if (size == ~size_t(0) || (offset + size) <= length)
		{
			DKCriticalSection<DKSpinLock> guard(lock);
			DKASSERT_DEBUG(mapped == nullptr);
			if (!mapped)
			{
				if (size == ~size_t(0))
					size = VK_WHOLE_SIZE;
				GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
				VkResult result = vkMapMemory(dev->device, memory, offset, size, 0, &mapped);
				if (result == VK_SUCCESS)
				{
					return mapped;
				}
				else
				{
					DKLogE("ERROR: vkMapMemory failed: %s", VkResultCStr(result));
				}
			}
		}
	}
	return NULL;
}

void Buffer::Unlock()
{
	DKCriticalSection<DKSpinLock> guard(lock);
	if (mapped)
	{
		GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
		vkUnmapMemory(dev->device, memory);
		mapped = nullptr;
	}
}

#endif //#if DKGL_ENABLE_VULKAN
