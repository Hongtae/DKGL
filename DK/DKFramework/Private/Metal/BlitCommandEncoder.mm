//
//  File: BlitCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "BlitCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

BlitCommandEncoder::BlitCommandEncoder(CommandBuffer* b)
: buffer(b)
{
	encoderCommands.Reserve(InitialNumberOfCommands);
}

BlitCommandEncoder::~BlitCommandEncoder(void)
{
}

void BlitCommandEncoder::EndEncoding(void)
{
	DKASSERT_DEBUG(!IsCompleted());
	encoderCommands.ShrinkToFit();
	buffer->EndEncoder(this);
}

bool BlitCommandEncoder::EncodeBuffer(id<MTLCommandBuffer> buffer)
{
	DKASSERT_DEBUG(IsCompleted());

	id<MTLBlitCommandEncoder> encoder = [buffer blitCommandEncoder];
	for (EncoderCommand* command : encoderCommands )
	{
		command->Invoke(encoder);
	}
	[encoder endEncoding];
	return true;
}
#endif //#if DKGL_ENABLE_METAL
