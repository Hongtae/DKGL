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
    , device(img->device)
    , signalSemaphore(VK_NULL_HANDLE)
    , waitSemaphore(VK_NULL_HANDLE)
{
}

ImageView::ImageView(DKGraphicsDevice* dev, VkImageView view)
    : image(nullptr)
    , imageView(view)
    , device(dev)
    , signalSemaphore(VK_NULL_HANDLE)
    , waitSemaphore(VK_NULL_HANDLE)
{
}

ImageView::~ImageView()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

    if (imageView)
        vkDestroyImageView(dev->device, imageView, dev->allocationCallbacks);
    if (signalSemaphore)
        vkDestroySemaphore(dev->device, signalSemaphore, dev->allocationCallbacks);
    if (waitSemaphore)
        vkDestroySemaphore(dev->device, waitSemaphore, dev->allocationCallbacks);
}

#endif //#if DKGL_ENABLE_VULKAN
