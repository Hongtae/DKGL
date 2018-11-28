//
//  File: ComputePipelineState.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2018 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "ComputePipelineState.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ComputePipelineState::ComputePipelineState(DKGraphicsDevice* d, id<MTLComputePipelineState> pso, MTLSize wgSize)
: device(d)
, workgroupSize(wgSize)
{
	pipelineState = [pso retain];
}

ComputePipelineState::~ComputePipelineState()
{
	[pipelineState autorelease];
}

#endif //#if DKGL_ENABLE_METAL
