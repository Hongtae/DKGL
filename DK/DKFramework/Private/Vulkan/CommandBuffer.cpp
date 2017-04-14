//
//  File: CommandBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "CommandBuffer.h"
#include "RenderCommandEncoder.h"
#include "ComputeCommandEncoder.h"
#include "BlitCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

CommandBuffer::~CommandBuffer(void)
{
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(const DKRenderPassDescriptor& rp)
{
	DKObject<RenderCommandEncoder> encoder = DKOBJECT_NEW RenderCommandEncoder(this, rp);
	return encoder.SafeCast<DKRenderCommandEncoder>();
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder(void)
{
	DKObject<ComputeCommandEncoder> encoder = DKOBJECT_NEW ComputeCommandEncoder(this);
	return encoder.SafeCast<DKComputeCommandEncoder>();
}

DKObject<DKBlitCommandEncoder> CommandBuffer::CreateBlitCommandEncoder(void)
{
	DKObject<BlitCommandEncoder> encoder = DKOBJECT_NEW BlitCommandEncoder(this);
	return encoder.SafeCast<DKBlitCommandEncoder>();
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
