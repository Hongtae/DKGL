//
//  File: RenderCommandEncoder.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_METAL

#include "ComputeCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ComputeCommandEncoder::ComputeCommandEncoder(CommandBuffer* b, id<MTLComputeCommandEncoder> e)
: buffer(b)
, encoder(nil)
{
	encoder = [e retain];
}

ComputeCommandEncoder::~ComputeCommandEncoder(void)
{
	if (encoder)
		[encoder release];
}

void ComputeCommandEncoder::EndEncoding(void)
{
	if (encoder)
	{
		[encoder endEncoding];
		[encoder release];
		encoder = nil;

		buffer->EndEncoder(this);
	}
}

DKCommandBuffer* ComputeCommandEncoder::Buffer(void)
{
	return buffer;
}

#endif //#if DKGL_USE_METAL
