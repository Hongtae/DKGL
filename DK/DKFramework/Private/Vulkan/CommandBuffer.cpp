//
//  File: CommandBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "CommandBuffer.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

CommandBuffer::~CommandBuffer(void)
{
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(DKRenderPassDescriptor*)
{
	return NULL;
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder(void)
{
	return NULL;
}

DKObject<DKBlitCommandEncoder> CommandBuffer::CreateBlitCommandEncoder(void)
{
	return NULL;
}

bool CommandBuffer::Commit(void)
{
	return false;
}

bool CommandBuffer::WaitUntilCompleted(void)
{
	return false;
}

#endif //#if DKGL_USE_VULKAN
