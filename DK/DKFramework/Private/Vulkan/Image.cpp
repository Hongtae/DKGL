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
    , layoutInfo{ ci.initialLayout }
{
    imageType = ci.imageType;
    format = ci.format;
    extent = ci.extent;
    mipLevels = ci.mipLevels;
    arrayLayers = ci.arrayLayers;
    usage = ci.usage;

    layoutInfo.accessMask = 0;
    layoutInfo.stageMaskBegin = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    layoutInfo.stageMaskEnd = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

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
    , layoutInfo{ VK_IMAGE_LAYOUT_UNDEFINED }
{
    layoutInfo.accessMask = 0;
    layoutInfo.stageMaskBegin = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    layoutInfo.stageMaskEnd = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
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

VkImageLayout Image::SetLayout(VkImageLayout layout,
                               VkAccessFlags accessMask,
                               VkPipelineStageFlags stageBegin,
                               VkPipelineStageFlags stageEnd,
                               LayoutPipelineBarrierProc* barrierProc) const
{
    DKASSERT_DEBUG(layout != VK_IMAGE_LAYOUT_UNDEFINED);
    DKASSERT_DEBUG(layout != VK_IMAGE_LAYOUT_PREINITIALIZED);

    DKCriticalSection<DKSpinLock> guard(layoutLock);
    if (barrierProc)
    {
        VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.srcAccessMask = layoutInfo.accessMask;
        barrier.dstAccessMask = accessMask;
        barrier.oldLayout = layoutInfo.layout;
        barrier.newLayout = layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;

        DKPixelFormat pixelFormat = PixelFormat();
        if (DKPixelFormatIsColorFormat(pixelFormat))
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        else
        {
            if (DKPixelFormatIsDepthFormat(pixelFormat))
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
            if (DKPixelFormatIsStencilFormat(pixelFormat))
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        barrierProc->Invoke(barrier, layoutInfo.stageMaskEnd);
    }
    VkImageLayout oldLayout = layoutInfo.layout;
    layoutInfo.layout = layout;
    layoutInfo.stageMaskBegin = stageBegin;
    layoutInfo.stageMaskEnd = stageEnd;
    layoutInfo.accessMask = accessMask;
    return oldLayout;
}

VkImageLayout Image::Layout() const
{
    DKCriticalSection<DKSpinLock> guard(layoutLock);
    return layoutInfo.layout;
}

#endif //#if DKGL_ENABLE_VULKAN
