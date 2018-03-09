//
//  File: Texture.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2018 Hongtae Kim. All rights reserved.
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
	, mipLevels(0)
	, arrayLayers(0)
	, usage(0)
	, waitSemaphore(VK_NULL_HANDLE)
	, signalSemaphore(VK_NULL_HANDLE)
	, device(d)
{
	if (ci)
	{
		imageType = ci->imageType;
		format = ci->format;
		extent = ci->extent;
		mipLevels = ci->mipLevels;
		arrayLayers = ci->arrayLayers;
		usage = ci->usage;
	}
}

Texture::~Texture(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	if (imageView)
		vkDestroyImageView(dev->device, imageView, nullptr);
	if (signalSemaphore)
		vkDestroySemaphore(dev->device, signalSemaphore, nullptr);
	if (waitSemaphore)
		vkDestroySemaphore(dev->device, waitSemaphore, nullptr);
}

#endif //#if DKGL_ENABLE_VULKAN
