//
//  File: RenderCommandEncoder.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_METAL

#include "RenderCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

RenderCommandEncoder::RenderCommandEncoder(CommandBuffer* b, id<MTLRenderCommandEncoder> e)
: buffer(b)
, encoder(nil)
{
	encoder = [e retain];
}

RenderCommandEncoder::~RenderCommandEncoder(void)
{
	[encoder release];
}

void RenderCommandEncoder::EndEncoding(void)
{
	[encoder endEncoding];
}

DKCommandBuffer* RenderCommandEncoder::Buffer(void)
{
	return buffer;
}

#endif //#if DKGL_USE_METAL
