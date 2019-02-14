//
//  File: Buffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Buffer.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

Buffer::Buffer(DKGraphicsDevice* dev, VkBuffer b, VkBufferView v, DeviceMemory* mem)
	: device(dev)
	, buffer(b)
	, bufferView(v)
    , deviceMemory(mem)
{
    DKASSERT_DEBUG(deviceMemory)
	DKASSERT_DEBUG(deviceMemory->length > 0);
}

Buffer::~Buffer()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	if (bufferView)
		vkDestroyBufferView(dev->device, bufferView, dev->allocationCallbacks);
	if (buffer)
		vkDestroyBuffer(dev->device, buffer, dev->allocationCallbacks);
    deviceMemory = nullptr;
}

void* Buffer::Contents()
{
    return deviceMemory->mapped;
}

void Buffer::Flush(size_t offset, size_t size)
{
    size_t length = deviceMemory->length;
    if (offset < length)
    {
        if (size != VK_WHOLE_SIZE)
        {
            if (offset + size > length)
                size = length - offset;
        }
        if (size > 0 || size == VK_WHOLE_SIZE)
            deviceMemory->Flush(offset, size);
    }
    deviceMemory->Invalidate(0, VK_WHOLE_SIZE);
}

size_t Buffer::Length() const
{
    return deviceMemory->length;
}

#endif //#if DKGL_ENABLE_VULKAN
