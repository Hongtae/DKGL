//
//  File: DeviceMemory.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Vulkan
{
    class DeviceMemory
    {
        struct HostVisibleContext
        {
            DKSpinLock lock;
            uint64_t locked;
            void* mapped;
        };

    public:
        DeviceMemory(DKGraphicsDevice*, VkDeviceMemory, VkMemoryType, size_t);
        virtual ~DeviceMemory();

        VkDeviceMemory memory;
        VkMemoryType type;
        size_t length;

        void* Lock(size_t offset, size_t size);
        void Unlock();

        bool IsLockable() const { return lockContext != nullptr; }

        DKObject<DKGraphicsDevice> device;

    private:
        HostVisibleContext* lockContext;
    };
}

#endif //#if DKGL_ENABLE_VULKAN
