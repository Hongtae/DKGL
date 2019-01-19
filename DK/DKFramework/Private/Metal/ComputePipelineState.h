//
//  File: ComputePipelineState.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKComputePipeline.h"
#include "../../DKGraphicsDevice.h"
#include "ShaderModule.h"

namespace DKFramework::Private::Metal
{
	class ComputePipelineState : public DKComputePipelineState
	{
	public:
		ComputePipelineState(DKGraphicsDevice*, id<MTLComputePipelineState>, MTLSize);
		~ComputePipelineState();

		DKGraphicsDevice* Device() override { return device; }

		DKObject<DKGraphicsDevice> device;
		id<MTLComputePipelineState> pipelineState;

        StageResourceBindingMap bindings;
        MTLSize workgroupSize;
	};
}
#endif //#if DKGL_ENABLE_METAL
