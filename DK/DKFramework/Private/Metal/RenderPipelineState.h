//
//  File: RenderPipelineState.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKRenderPipeline.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class RenderPipelineState : public DKRenderPipelineState
			{
			public:
				RenderPipelineState(DKGraphicsDevice*, id<MTLRenderPipelineState>);
				~RenderPipelineState(void);

				DKGraphicsDevice* Device(void) override { return device;}

				DKObject<DKGraphicsDevice> device;
				id<MTLRenderPipelineState> pipelineState;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
