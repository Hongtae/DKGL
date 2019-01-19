//
//  File: BlitCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "BlitCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

BlitCommandEncoder::BlitCommandEncoder(class CommandBuffer* b)
: commandBuffer(b)
{
	reusableEncoder = DKOBJECT_NEW ReusableEncoder();
	reusableEncoder->encoderCommands.Reserve(ReusableCommandEncoder::InitialNumberOfCommands);
}

BlitCommandEncoder::~BlitCommandEncoder()
{
}

void BlitCommandEncoder::EndEncoding()
{
	DKASSERT_DEBUG(!IsCompleted());
	reusableEncoder->encoderCommands.ShrinkToFit();
	commandBuffer->EndEncoder(this, reusableEncoder);
	reusableEncoder = NULL;
}
#endif //#if DKGL_ENABLE_METAL
