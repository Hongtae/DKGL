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
#include "RenderPipelineState.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

RenderCommandEncoder::RenderCommandEncoder(MTLRenderPassDescriptor* rpDesc, CommandBuffer* b)
: buffer(b)
, renderPassDescriptor(nil)
{
	renderPassDescriptor = [rpDesc retain];
	encoderCommands.Reserve(InitialNumberOfCommands);
}

RenderCommandEncoder::~RenderCommandEncoder(void)
{
	if (renderPassDescriptor)
	{
		[renderPassDescriptor release];
	}
}

void RenderCommandEncoder::EndEncoding(void)
{
	DKASSERT_DEBUG(!IsCompleted());
	encoderCommands.ShrinkToFit();
	buffer->EndEncoder(this);
}

void RenderCommandEncoder::SetRenderPipelineState(DKRenderPipelineState* ps)
{
	DKASSERT_DEBUG(!IsCompleted());
	DKASSERT_DEBUG(dynamic_cast<RenderPipelineState*>(ps));
	DKObject<RenderPipelineState> pipeline = static_cast<RenderPipelineState*>(ps);

	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder)
	{
		id<MTLRenderPipelineState> pipelineState = pipeline->pipelineState;
		[encoder setRenderPipelineState:pipelineState];
	});
	encoderCommands.Add(command);
}

bool RenderCommandEncoder::EncodeBuffer(id<MTLCommandBuffer> buffer)
{
	DKASSERT_DEBUG(IsCompleted());
	DKASSERT_DEBUG(renderPassDescriptor);

	if (renderPassDescriptor)
	{
		id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		for (EncoderCommand* command : encoderCommands )
		{
			command->Invoke(encoder);
		}
		[encoder endEncoding];
		return true;
	}

	return false;
}
#endif //#if DKGL_ENABLE_METAL
