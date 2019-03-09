//
//  File: Texture.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Texture.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

Texture::Texture(DKGraphicsDevice* d, VkImage i, VkImageView v, const VkImageCreateInfo* ci)
	: image(i)
	, imageView(v)
	, imageType(VK_IMAGE_TYPE_1D)
	, format(VK_FORMAT_UNDEFINED)
	, extent({ 0,0,0 })
	, mipLevels(1)
	, arrayLayers(1)
	, usage(0)
	, waitSemaphore(VK_NULL_HANDLE)
	, signalSemaphore(VK_NULL_HANDLE)
	, device(d)
{
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (ci)
	{
		imageType = ci->imageType;
		format = ci->format;
		extent = ci->extent;
		mipLevels = ci->mipLevels;
		arrayLayers = ci->arrayLayers;
		usage = ci->usage;
        initialLayout = ci->initialLayout;
	}
    DKASSERT_DEBUG(arrayLayers > 0);

    layerLayouts = new VkImageLayout[arrayLayers];
    for (int layer = 0; layer < arrayLayers; ++layer)
    {
        layerLayouts[layer] = initialLayout;
    }
}

Texture::~Texture()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    if (image)
        vkDestroyImage(dev->device, image, dev->allocationCallbacks);
	if (imageView)
		vkDestroyImageView(dev->device, imageView, dev->allocationCallbacks);
	if (signalSemaphore)
		vkDestroySemaphore(dev->device, signalSemaphore, dev->allocationCallbacks);
	if (waitSemaphore)
		vkDestroySemaphore(dev->device, waitSemaphore, dev->allocationCallbacks);

    deviceMemory = nullptr;
    delete[] layerLayouts;
}

VkImageLayout Texture::ChangeLayerLayout(uint32_t layer,
                                         VkImageLayout newLayout,
                                         VkCommandBuffer commandBuffer,
                                         VkPipelineStageFlags srcStageMasks,
                                         VkPipelineStageFlags dstStageMasks) const
{
    DKASSERT_DEBUG(newLayout != VK_IMAGE_LAYOUT_UNDEFINED);
    DKASSERT_DEBUG(newLayout != VK_IMAGE_LAYOUT_PREINITIALIZED);
    DKASSERT_DEBUG(layer < arrayLayers);

    VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (layer < arrayLayers)
    {
        if (true)
        {
            DKCriticalSection<DKSpinLock> guard(layoutTransitionLock);
            oldLayout = layerLayouts[layer];

            layerLayouts[layer] = newLayout;
        }
        if (commandBuffer != VK_NULL_HANDLE && oldLayout != newLayout)
        {
            DKPixelFormat pixelFormat = PixelFormat();

            VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = 0;
            if (DKPixelFormatIsColorFormat(pixelFormat))
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
            if (DKPixelFormatIsDepthFormat(pixelFormat))
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
            if (DKPixelFormatIsStencilFormat(pixelFormat))
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            barrier.subresourceRange.baseArrayLayer = layer;
            barrier.subresourceRange.layerCount = 1;

            auto getLayoutAccessMasks = [](VkImageLayout layout)->VkAccessFlags
            {
                VkAccessFlags accessMask = 0;
                switch (layout)
                {
                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                    accessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                    break;
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                    accessMask = VK_ACCESS_SHADER_READ_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                    accessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                    accessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_PREINITIALIZED:
                    accessMask = VK_ACCESS_HOST_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                    accessMask = (VK_ACCESS_SHADER_READ_BIT |
                                  VK_ACCESS_SHADER_WRITE_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
                    break;
                case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                    accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    break;
                }
                return accessMask;
            };
            barrier.srcAccessMask = getLayoutAccessMasks(oldLayout);
            barrier.dstAccessMask = getLayoutAccessMasks(newLayout);

            vkCmdPipelineBarrier(
                commandBuffer,
                srcStageMasks,
                dstStageMasks,
                0,          //dependencyFlags
                0, nullptr, //pMemoryBarriers
                0, nullptr, //pBufferMemoryBarriers
                1, &barrier);
        }
    }
    return oldLayout;
}

#endif //#if DKGL_ENABLE_VULKAN
