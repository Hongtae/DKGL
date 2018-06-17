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
		GraphicsDevice(void);
		~GraphicsDevice(void);

		DKString DeviceName(void) const override;
		DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*, uint32_t) override;
		DKObject<DKShaderModule> CreateShaderModule(DKGraphicsDevice*, DKShader*) override;
		DKObject<DKRenderPipelineState> CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKRenderPipelineReflection*) override;
		DKObject<DKComputePipelineState> CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKComputePipelineReflection*) override;
		DKObject<DKGpuBuffer> CreateBuffer(DKGraphicsDevice*, size_t, DKGpuBuffer::StorageMode, DKCpuCacheMode) override;
		DKObject<DKTexture> CreateTexture(DKGraphicsDevice*, const DKTextureDescriptor&) override;

		NSUInteger VertexAttributeIndexForDevice(NSUInteger index)
		{
			/* 31 = Maximum number of vertex attributes, per vertex descriptor */
			const NSUInteger maxNumberOfVertexAttributes = 31;
			return maxNumberOfVertexAttributes - index - 1;
		}
		id<MTLDevice> device;
	};
}
#endif //#if DKGL_ENABLE_METAL
