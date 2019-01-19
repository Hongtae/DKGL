//
//  File: ComputeCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "ComputeCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ComputeCommandEncoder::ComputeCommandEncoder(VkCommandBuffer vcb, CommandBuffer* cb)
	: encodingBuffer(vcb)
	, commandBuffer(cb)
{
}

ComputeCommandEncoder::~ComputeCommandEncoder()
{
}

void ComputeCommandEncoder::EndEncoding()
{
}

DKCommandBuffer* ComputeCommandEncoder::Buffer()
{
	return commandBuffer;
}

#endif //#if DKGL_ENABLE_VULKAN
