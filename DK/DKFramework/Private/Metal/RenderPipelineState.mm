//
//  File: RenderPipelineState.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include <TargetConditionals.h>

#include "RenderPipelineState.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

RenderPipelineState::RenderPipelineState(DKGraphicsDevice* d, id<MTLRenderPipelineState> st)
: device(d)
{
	pipelineState = [st retain];
}

RenderPipelineState::~RenderPipelineState()
{
	[pipelineState autorelease];
}

#endif //#if DKGL_ENABLE_METAL
