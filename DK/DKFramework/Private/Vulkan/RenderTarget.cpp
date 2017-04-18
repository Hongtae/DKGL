//
//  File: RenderTarget.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include "Extensions.h"
#include "RenderTarget.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

RenderTarget::RenderTarget(DKGraphicsDevice* d, VkImageView v, VkImage i, const VkImageCreateInfo* ci)
	: TextureBaseT(i, ci)
	, device(d)
	, imageView(v)
	, swapchain(nullptr)
{
}

RenderTarget::~RenderTarget(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	if (imageView)
		vkDestroyImageView(dev->device, imageView, nullptr);
}

#endif //#if DKGL_USE_VULKAN
