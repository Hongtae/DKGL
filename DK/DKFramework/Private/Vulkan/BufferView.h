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
        ~BufferView();

        void* Contents()
        {
            return buffer->Contents();
        }

        void Flush(size_t offset, size_t size)
        {
            buffer->Flush(offset, size);
        }

        size_t Length() const
        {
            return buffer->Length();
        }


        VkBufferView bufferView;
        DKObject<Buffer> buffer;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
