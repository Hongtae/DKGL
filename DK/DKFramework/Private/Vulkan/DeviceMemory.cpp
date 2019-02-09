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
    , lockContext(nullptr)
{
    DKASSERT_DEBUG(memory);
    DKASSERT_DEBUG(length > 0);

    if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        lockContext = new HostVisibleContext();
        lockContext->locked = 0;
        lockContext->mapped = nullptr;
    }
}

DeviceMemory::~DeviceMemory()
{
    if (lockContext)
    {
        DKASSERT_DEBUG(lockContext->mapped == nullptr);
        DKASSERT_DEBUG(lockContext->locked == 0);

        delete lockContext;
    }

    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

    DKASSERT_DEBUG(memory != VK_NULL_HANDLE);
    vkFreeMemory(dev->device, memory, dev->allocationCallbacks);
}

void* DeviceMemory::Lock(size_t offset, size_t size)
{
    if (lockContext && offset < length)
    {
        DKASSERT_DEBUG(type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        if (size == ~size_t(0) || (offset + size) <= length)
        {
            DKCriticalSection<DKSpinLock> guard(lockContext->lock);
            if (lockContext->locked)
            {
                DKASSERT_DEBUG(lockContext->mapped);
                lockContext->locked++;
            }
            else
            {
                DKASSERT_DEBUG(lockContext->mapped == nullptr);

                if (size == ~size_t(0))
                    size = VK_WHOLE_SIZE;
                GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
                VkResult result = vkMapMemory(dev->device, memory, offset, size, 0, &lockContext->mapped);
                if (result == VK_SUCCESS)
                {
                    lockContext->locked++;
                }
                else
                {
                    DKLogE("ERROR: vkMapMemory failed: %s", VkResultCStr(result));
                }
            }
            return lockContext->mapped;
        }
    }
    return nullptr;
}

void DeviceMemory::Unlock()
{
    DKASSERT_DEBUG(lockContext);

    DKCriticalSection<DKSpinLock> guard(lockContext->lock);
    DKASSERT_DEBUG(lockContext->mapped);
    DKASSERT_DEBUG(lockContext->locked > 0);
    lockContext->locked--;
    if (lockContext->locked == 0)
    {
        GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
        vkUnmapMemory(dev->device, memory);
        lockContext->mapped = nullptr;
    }
}

#endif //#if DKGL_ENABLE_VULKAN
