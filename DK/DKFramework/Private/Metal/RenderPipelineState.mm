//
//  File: RenderPipelineState.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "RenderPipelineState.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

RenderPipelineState::RenderPipelineState(DKGraphicsDevice* d, id<MTLRenderPipelineState> pso, id<MTLDepthStencilState> dss)
: device(d)
{
	pipelineState = [pso retain];
    depthStencilState = [dss retain];
}

RenderPipelineState::~RenderPipelineState()
{
	[pipelineState autorelease];
    [depthStencilState autorelease];
}

#endif //#if DKGL_ENABLE_METAL
