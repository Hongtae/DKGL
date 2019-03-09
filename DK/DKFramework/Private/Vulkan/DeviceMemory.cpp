//
//  File: DeviceMemory.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "DeviceMemory.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

DeviceMemory::DeviceMemory(DKGraphicsDevice* dev, VkDeviceMemory mem, VkMemoryType t, size_t s)
    : device(dev)
    , memory(mem)
    , type(t)
    , length(s)
    , mapped(nullptr)
{
    DKASSERT_DEBUG(memory);
    DKASSERT_DEBUG(length > 0);

    if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        size_t offset = 0;
        size_t size = VK_WHOLE_SIZE;

        GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
        VkResult result = vkMapMemory(dev->device, memory, offset, size, 0, &mapped);
        if (result != VK_SUCCESS)
        {
            DKLogE("ERROR: vkMapMemory failed: %s", VkResultCStr(result));
        }
    }
}

DeviceMemory::~DeviceMemory()
{
    DKASSERT_DEBUG(memory != VK_NULL_HANDLE);
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

    if (mapped)
        vkUnmapMemory(dev->device, memory);

    vkFreeMemory(dev->device, memory, dev->allocationCallbacks);
}

void DeviceMemory::Invalidate(size_t offset, size_t size)
{
    DKASSERT_DEBUG(memory != VK_NULL_HANDLE);

    if (mapped && (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
        if (offset < length)
        {
            GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

            VkMappedMemoryRange range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
            range.memory = memory;
            range.offset = offset;
            if (size == VK_WHOLE_SIZE)
                range.size = size;
            else
                range.size = Min(size, length - offset);
            VkResult result = vkInvalidateMappedMemoryRanges(dev->device, 1, &range);
            if (result != VK_SUCCESS)
            {
                DKLogE("ERROR: vkInvalidateMappedMemoryRanges failed: %s", VkResultCStr(result));
            }
        }
        else
        {
            DKLogE("ERROR: DeviceMemory::Invalidate() failed: Out of range");
        }
    }
}

void DeviceMemory::Flush(size_t offset, size_t size)
{
    DKASSERT_DEBUG(memory != VK_NULL_HANDLE);

    if (mapped && (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
        if (offset < length)
        {
            GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

            VkMappedMemoryRange range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
            range.memory = memory;
            range.offset = offset;
            if (size == VK_WHOLE_SIZE)
                range.size = size;
            else
                range.size = Min(size, length - offset);            
            VkResult result = vkFlushMappedMemoryRanges(dev->device, 1, &range);
            if (result != VK_SUCCESS)
            {
                DKLogE("ERROR: vkFlushMappedMemoryRanges failed: %s", VkResultCStr(result));
            }
        }
        else
        {
            DKLogE("ERROR: DeviceMemory::Flush() failed: Out of range");
        }
    }
}

#endif //#if DKGL_ENABLE_VULKAN
