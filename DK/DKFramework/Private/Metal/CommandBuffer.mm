//
//  File: CommandBuffer.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_METAL

#include "CommandBuffer.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

CommandBuffer::~CommandBuffer(void)
{
	[buffer autorelease];
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

#endif //#if DKGL_USE_METAL
