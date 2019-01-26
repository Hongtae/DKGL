//
//  File: CopyCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "CopyCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

CopyCommandEncoder::CopyCommandEncoder(VkCommandBuffer vcb, class CommandBuffer* cb)
	: encodingBuffer(vcb)
	, commandBuffer(cb)
{
}

CopyCommandEncoder::~CopyCommandEncoder()
{
}

void CopyCommandEncoder::EndEncoding()
{
}

DKCommandBuffer* CopyCommandEncoder::CommandBuffer()
{
	return commandBuffer;
}

#endif //#if DKGL_ENABLE_VULKAN
