//
//  File: Buffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2018 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Buffer.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

Buffer::Buffer(DKGraphicsDevice* dev, VkBuffer b, VkBufferView v)
	: device(dev)
	, buffer(b)
	, view(v)
{

}

Buffer::~Buffer(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	if (view)
		vkDestroyBufferView(dev->device, view, nullptr);
	if (buffer)
		vkDestroyBuffer(dev->device, buffer, nullptr);
}

#endif //#if DKGL_ENABLE_VULKAN
