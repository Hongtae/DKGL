//
//  File: Image.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Image.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

Image::Image(DeviceMemory* m, VkImage i, const VkImageCreateInfo& ci)
    : image(i)
    , deviceMemory(m)
    , device(m->device)
    , imageType(VK_IMAGE_TYPE_1D)
    , format(VK_FORMAT_UNDEFINED)
    , extent({ 0,0,0 })
    , mipLevels(1)
    , arrayLayers(1)
    , usage(0)
    , optimalLayout(VK_IMAGE_LAYOUT_GENERAL)
{
    imageType = ci.imageType;
    format = ci.format;
    extent = ci.extent;
    mipLevels = ci.mipLevels;
    arrayLayers = ci.arrayLayers;
    usage = ci.usage;
    currentLayout = ci.initialLayout;

    DKASSERT_DEBUG(deviceMemory);
    DKASSERT_DEBUG(extent.width > 0);
    DKASSERT_DEBUG(extent.height > 0);
    DKASSERT_DEBUG(extent.depth > 0);
    DKASSERT_DEBUG(mipLevels > 0);
    DKASSERT_DEBUG(arrayLayers > 0);
    DKASSERT_DEBUG(format != VK_FORMAT_UNDEFINED);
}

Image::Image(DKGraphicsDevice* dev, VkImage img)
    : image(img)
    , device(dev)
    , deviceMemory(nullptr)
    , imageType(VK_IMAGE_TYPE_1D)
    , format(VK_FORMAT_UNDEFINED)
    , extent({ 0,0,0 })
    , mipLevels(1)
    , arrayLayers(1)
    , usage(0)
    , currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
    , optimalLayout(VK_IMAGE_LAYOUT_GENERAL)
{
}

Image::~Image()
{
    if (image)
    {
        GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
        vkDestroyImage(dev->device, image, dev->allocationCallbacks);
    }
    deviceMemory = nullptr;
}

VkImageLayout Image::ResetLayout() const
{
    DKCriticalSection<DKSpinLock> guard(layoutLock);
    VkImageLayout oldLayout = currentLayout;
    currentLayout = optimalLayout;
    return oldLayout;
}

VkImageLayout Image::SetLayout(VkImageLayout layout) const
{
    DKASSERT_DEBUG(layout != VK_IMAGE_LAYOUT_UNDEFINED);
    DKASSERT_DEBUG(layout != VK_IMAGE_LAYOUT_PREINITIALIZED);

    DKCriticalSection<DKSpinLock> guard(layoutLock);
    VkImageLayout oldLayout = currentLayout;
    currentLayout = layout;
    return oldLayout;
}

VkImageLayout Image::SetOptimalLayout(VkImageLayout layout)
{
    DKASSERT_DEBUG(layout != VK_IMAGE_LAYOUT_UNDEFINED);
    DKASSERT_DEBUG(layout != VK_IMAGE_LAYOUT_PREINITIALIZED);

    DKCriticalSection<DKSpinLock> guard(layoutLock);
    VkImageLayout oldLayout = optimalLayout;
    optimalLayout = layout;
    return oldLayout;
}

VkImageLayout Image::Layout() const
{
    DKCriticalSection<DKSpinLock> guard(layoutLock);
    return currentLayout;
}

VkImageLayout Image::OptimalLayout() const
{
    DKCriticalSection<DKSpinLock> guard(layoutLock);
    return optimalLayout;
}


#endif //#if DKGL_ENABLE_VULKAN
