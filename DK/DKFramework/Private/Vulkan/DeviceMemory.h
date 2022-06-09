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
    class DeviceMemory final
    {
    public:
        DeviceMemory(DKGraphicsDevice*, VkDeviceMemory, VkMemoryType, VkDeviceSize);
        ~DeviceMemory();

        VkDeviceMemory memory;
        VkMemoryType type;
        VkDeviceSize length;
        void* mapped;

        DKObject<DKGraphicsDevice> device;

        bool Invalidate(uint64_t offset, uint64_t size);
        bool Flush(uint64_t offset, uint64_t size);
    };
}

#endif //#if DKGL_ENABLE_VULKAN
