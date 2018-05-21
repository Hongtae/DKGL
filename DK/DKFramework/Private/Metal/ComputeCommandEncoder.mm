//
//  File: RenderCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "ComputeCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ComputeCommandEncoder::ComputeCommandEncoder(CommandBuffer* b)
: buffer(b)
{
	encoderCommands.Reserve(InitialNumberOfCommands);
}

ComputeCommandEncoder::~ComputeCommandEncoder(void)
{
}

void ComputeCommandEncoder::EndEncoding(void)
{
	DKASSERT_DEBUG(!IsCompleted());
	encoderCommands.ShrinkToFit();
	buffer->EndEncoder(this);
}

bool ComputeCommandEncoder::EncodeBuffer(id<MTLCommandBuffer> buffer)
{
	DKASSERT_DEBUG(IsCompleted());

	id<MTLComputeCommandEncoder> encoder = [buffer computeCommandEncoder];
	for (EncoderCommand* command : encoderCommands )
	{
		command->Invoke(encoder);
	}
	[encoder endEncoding];
	return true;
}
#endif //#if DKGL_ENABLE_METAL
