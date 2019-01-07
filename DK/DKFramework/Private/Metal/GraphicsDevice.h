//
//  File: GraphicsDevice.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../Interface/DKGraphicsDeviceInterface.h"

namespace DKFramework::Private::Metal
{
	class GraphicsDevice : public DKGraphicsDeviceInterface
	{
	public:
		GraphicsDevice();
		~GraphicsDevice();

		DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*, uint32_t) override;
		DKObject<DKRenderPipelineState> CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKPipelineReflection*) override;
		DKObject<DKComputePipelineState> CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*) override;

        DKObject<DKShaderModule> CreateShaderModule(DKGraphicsDevice*, DKShader*) override;
        DKObject<DKShaderBindingSet> CreateShaderBindingSet(DKGraphicsDevice*, const DKShaderBindingSetLayout&) override;

		DKObject<DKGpuBuffer> CreateBuffer(DKGraphicsDevice*, size_t, DKGpuBuffer::StorageMode, DKCpuCacheMode) override;
		DKObject<DKTexture> CreateTexture(DKGraphicsDevice*, const DKTextureDescriptor&) override;
        DKObject<DKSamplerState> CreateSamplerState(DKGraphicsDevice*, const DKSamplerDescriptor&) override;

        DKString DeviceName() const override;

		id<MTLDevice> device;
	};
}
#endif //#if DKGL_ENABLE_METAL
