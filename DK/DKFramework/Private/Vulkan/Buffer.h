//
//  File: Buffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKGpuBuffer.h"
#include "DeviceMemory.h"

namespace DKFramework::Private::Vulkan
{
    class Buffer
    {
    public:
        Buffer(DeviceMemory*, VkBuffer, const VkBufferCreateInfo&);
        Buffer(DKGraphicsDevice*, VkBuffer);
        ~Buffer();

        void* Contents();
        void Flush(size_t, size_t);
        size_t Length() const;

        DKObject<class BufferView> CreateBufferView(DKPixelFormat format, size_t offset, size_t range);

        VkBuffer buffer;
        VkBufferUsageFlags     usage;
        VkSharingMode          sharingMode;

        DKObject<DeviceMemory> deviceMemory;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
