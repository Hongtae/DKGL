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
	reusableEncoder = DKOBJECT_NEW ReusableEncoder();
	reusableEncoder->encoderCommands.Reserve(ReusableCommandEncoder::InitialNumberOfCommands);
}

ComputeCommandEncoder::~ComputeCommandEncoder()
{
}

void ComputeCommandEncoder::EndEncoding()
{
	DKASSERT_DEBUG(!IsCompleted());
	reusableEncoder->encoderCommands.ShrinkToFit();
	buffer->EndEncoder(this, reusableEncoder);
	reusableEncoder = NULL;
}
#endif //#if DKGL_ENABLE_METAL
