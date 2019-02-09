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

void* Buffer::Lock(size_t offset, size_t size)
{
    return deviceMemory->Lock(offset, size);
}

void Buffer::Unlock()
{
    deviceMemory->Unlock();
}

#endif //#if DKGL_ENABLE_VULKAN
