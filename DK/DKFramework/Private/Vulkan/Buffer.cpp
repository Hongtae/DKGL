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
#include "BufferView.h"
#include "GraphicsDevice.h"
#include "Types.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

Buffer::Buffer(DeviceMemory* mem, VkBuffer b, const VkBufferCreateInfo& createInfo)
    : deviceMemory(mem)
    , device(mem->device)
	, buffer(b)
    , usage(createInfo.usage)
    , sharingMode(createInfo.sharingMode)
{
    DKASSERT_DEBUG(deviceMemory);
	DKASSERT_DEBUG(deviceMemory->length > 0);
}

Buffer::Buffer(DKGraphicsDevice* dev, VkBuffer b)
    : deviceMemory(nullptr)
    , device(dev)
    , buffer(b)
{
}

Buffer::~Buffer()
{
    if (buffer)
    {
        GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
        vkDestroyBuffer(dev->device, buffer, dev->allocationCallbacks);
    }
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
        if (size > 0)
            deviceMemory->Flush(offset, size);
    }
    deviceMemory->Invalidate(0, VK_WHOLE_SIZE);
}

size_t Buffer::Length() const
{
    return deviceMemory->length;
}

DKObject<BufferView> Buffer::CreateBufferView(DKPixelFormat pixelFormat, size_t offset, size_t range)
{
    if (usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT ||
        usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
    {
        VkFormat format = PixelFormat(pixelFormat);
        if (format != VK_FORMAT_UNDEFINED)
        {
            GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(deviceMemory->device);
            auto alignment = dev->properties.limits.minTexelBufferOffsetAlignment;
            DKASSERT_DEBUG(offset % alignment == 0);

            VkBufferViewCreateInfo bufferViewCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };
            bufferViewCreateInfo.buffer = buffer;
            bufferViewCreateInfo.format = format;
            bufferViewCreateInfo.offset = offset;
            bufferViewCreateInfo.range = range;

            VkBufferView view = VK_NULL_HANDLE;
            VkResult result = vkCreateBufferView(dev->device, &bufferViewCreateInfo, dev->allocationCallbacks, &view);
            if (result == VK_SUCCESS)
            {
                DKObject<BufferView> bufferView = DKOBJECT_NEW BufferView(this, view, bufferViewCreateInfo);
                return bufferView;
            }
            else
            {
                DKLogE("ERROR: vkCreateBufferView failed: %s", VkResultCStr(result));
            }
        }
        else
        {
            DKLogE("Buffer::CreateBufferView failed: Invalid pixel format!");
        }
    }
    else
    {
        DKLogE("Buffer::CreateBufferView failed: Invalid buffer object (Not intended for texel buffer creation)");
    }
    return NULL;
}

#endif //#if DKGL_ENABLE_VULKAN
