//
//  File: ImageView.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Image.h"
#include "ImageView.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ImageView::ImageView(Image* img, VkImageView view, const VkImageViewCreateInfo& ci)
    : image(img)
    , imageView(view)
    , signalSemaphore(VK_NULL_HANDLE)
    , waitSemaphore(VK_NULL_HANDLE)
{
}

ImageView::ImageView()
    : image(nullptr)
    , imageView(VK_NULL_HANDLE)
    , signalSemaphore(VK_NULL_HANDLE)
    , waitSemaphore(VK_NULL_HANDLE)
{
}

ImageView::~ImageView()
{
    GraphicsDevice* dev = nullptr;
    if (image)
        dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(image->deviceMemory->device);

    if (imageView)
    {
        DKASSERT_DEBUG(dev);
        vkDestroyImageView(dev->device, imageView, dev->allocationCallbacks);
    }
    if (signalSemaphore)
    {
        DKASSERT_DEBUG(dev);
        vkDestroySemaphore(dev->device, signalSemaphore, dev->allocationCallbacks);
    }
    if (waitSemaphore)
    {
        DKASSERT_DEBUG(dev);
        vkDestroySemaphore(dev->device, waitSemaphore, dev->allocationCallbacks);
    }
}

#endif //#if DKGL_ENABLE_VULKAN
