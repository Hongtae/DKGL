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
    , imageType(VK_IMAGE_TYPE_1D)
    , format(VK_FORMAT_UNDEFINED)
    , extent({ 0,0,0 })
    , mipLevels(1)
    , arrayLayers(1)
    , usage(0)
    , deviceMemory(m)
    , device(m->device)
{
    imageType = ci.imageType;
    format = ci.format;
    extent = ci.extent;
    mipLevels = ci.mipLevels;
    arrayLayers = ci.arrayLayers;
    usage = ci.usage;
    VkImageLayout initialLayout = ci.initialLayout;

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

#endif //#if DKGL_ENABLE_VULKAN
