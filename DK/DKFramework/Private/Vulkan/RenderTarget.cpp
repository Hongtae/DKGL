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

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

RenderTarget::RenderTarget(VkDevice d, VkImageView v)
	: device(d)
	, imageView(v)
{
}

RenderTarget::~RenderTarget(void)
{
	if (imageView)
		vkDestroyImageView(device, imageView, nullptr);
}

#endif //#if DKGL_USE_VULKAN
