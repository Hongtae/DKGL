//
//  File: RenderCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "RenderCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

RenderCommandEncoder::RenderCommandEncoder(id<MTLRenderCommandEncoder> e, CommandBuffer* b)
: buffer(b)
, encoder(nil)
{
	encoder = [e retain];
}

RenderCommandEncoder::~RenderCommandEncoder(void)
{
    [encoder autorelease];
}

void RenderCommandEncoder::EndEncoding(void)
{
	if (encoder)
	{
		[encoder endEncoding];
		[encoder autorelease];
		encoder = nil;

		buffer->EndEncoder(this);
	}
}

DKCommandBuffer* RenderCommandEncoder::Buffer(void)
{
	return buffer;
}

#endif //#if DKGL_ENABLE_METAL
