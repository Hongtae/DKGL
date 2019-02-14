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
    class Buffer : public DKGpuBuffer
    {
    public:
        Buffer(DKGraphicsDevice*, VkBuffer, VkBufferView, DeviceMemory*);
        ~Buffer();

        void* Contents() override;
        void Flush(size_t, size_t) override;
        size_t Length() const override;

        VkBuffer buffer;
        VkBufferView bufferView;
        DKObject<DeviceMemory> deviceMemory;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
