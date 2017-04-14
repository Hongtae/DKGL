//
//  File: RenderCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "RenderCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

RenderCommandEncoder::RenderCommandEncoder(CommandBuffer* cb, const DKRenderPassDescriptor&)
	: commandBuffer(cb)
{
}

RenderCommandEncoder::~RenderCommandEncoder(void)
{
}

void RenderCommandEncoder::EndEncoding(void)
{
}

DKCommandBuffer* RenderCommandEncoder::Buffer(void)
{
	return commandBuffer;
}

#endif //#if DKGL_USE_VULKAN
