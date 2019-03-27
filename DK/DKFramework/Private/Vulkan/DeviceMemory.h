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
        DeviceMemory(DKGraphicsDevice*, VkDeviceMemory, VkMemoryType, size_t);
        ~DeviceMemory();

        VkDeviceMemory memory;
        VkMemoryType type;
        size_t length;
        void* mapped;

        DKObject<DKGraphicsDevice> device;

        bool Invalidate(size_t offset, size_t size);
        bool Flush(size_t offset, size_t size);
    };
}

#endif //#if DKGL_ENABLE_VULKAN
