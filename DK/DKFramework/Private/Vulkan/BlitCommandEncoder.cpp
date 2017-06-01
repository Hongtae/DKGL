//
//  File: BlitCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN

#include "BlitCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

BlitCommandEncoder::BlitCommandEncoder(VkCommandBuffer vcb, CommandBuffer* cb)
	: encodingBuffer(vcb)
	, commandBuffer(cb)
{
}

BlitCommandEncoder::~BlitCommandEncoder(void)
{
}

void BlitCommandEncoder::EndEncoding(void)
{
}

DKCommandBuffer* BlitCommandEncoder::Buffer(void)
{
	return commandBuffer;
}

#endif //#if DKGL_ENABLE_VULKAN
