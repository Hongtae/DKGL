//
//  File: RenderPipelineState.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKRenderPipeline.h"
#include "../../DKGraphicsDevice.h"
#include "ShaderModule.h"

namespace DKFramework::Private::Metal
{
	class RenderPipelineState : public DKRenderPipelineState
	{
	public:
		RenderPipelineState(DKGraphicsDevice*, id<MTLRenderPipelineState>, id<MTLDepthStencilState>);
		~RenderPipelineState();

		DKGraphicsDevice* Device() override { return device; }

        id<MTLRenderPipelineState> pipelineState;
        id<MTLDepthStencilState> depthStencilState;

		MTLPrimitiveType primitiveType;
		MTLDepthClipMode depthClipMode;
		MTLTriangleFillMode triangleFillMode;
		MTLWinding frontFacingWinding;
		MTLCullMode cullMode;

		DKObject<DKGraphicsDevice> device;

        StageResourceBindingMap vertexBindings;
        StageResourceBindingMap fragmentBindings;
	};
}
#endif //#if DKGL_ENABLE_METAL
