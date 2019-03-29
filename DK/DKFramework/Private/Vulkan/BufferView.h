//
//  File: BufferView.h
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
#include "Buffer.h"

namespace DKFramework::Private::Vulkan
{
    class BufferView : public DKGpuBuffer
    {
    public:
        BufferView(Buffer*, VkBufferView, const VkBufferViewCreateInfo&);
        BufferView(DKGraphicsDevice*, VkBufferView);
        ~BufferView();

        void* Contents() override
        {
            return buffer->Contents();
        }
        void Flush() override
        {
            buffer->Flush(0, VK_WHOLE_SIZE);
        }
        size_t Length() const override
        {
            return buffer->Length();
        }

        VkBufferView bufferView;
        DKObject<Buffer> buffer;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
