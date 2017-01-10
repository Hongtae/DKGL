//
//  File: CommandBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "CommandBuffer.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

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

void CommandBuffer::Commit(void)
{
}

#endif //#if DKGL_USE_DIRECT3D
