//
//  File: BufferView.cpp
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

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

BufferView::BufferView(Buffer* b, VkBufferView v, const VkBufferViewCreateInfo&)
    : buffer(b)
    , bufferView(v)
{
}

BufferView::~BufferView()
{
    if (bufferView)
    {
        GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(buffer->deviceMemory->device);
        vkDestroyBufferView(dev->device, bufferView, dev->allocationCallbacks);
    }
}

#endif //#if DKGL_ENABLE_VULKAN
