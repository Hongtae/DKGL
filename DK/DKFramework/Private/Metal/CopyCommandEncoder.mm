//
//  File: CopyCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "CopyCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

CopyCommandEncoder::CopyCommandEncoder(class CommandBuffer* b)
: commandBuffer(b)
{
	reusableEncoder = DKOBJECT_NEW ReusableEncoder();
	reusableEncoder->encoderCommands.Reserve(ReusableCommandEncoder::InitialNumberOfCommands);
}

CopyCommandEncoder::~CopyCommandEncoder()
{
}

void CopyCommandEncoder::EndEncoding()
{
	DKASSERT_DEBUG(!IsCompleted());
	reusableEncoder->encoderCommands.ShrinkToFit();
	commandBuffer->EndEncoder(this, reusableEncoder);
	reusableEncoder = NULL;
}
#endif //#if DKGL_ENABLE_METAL
