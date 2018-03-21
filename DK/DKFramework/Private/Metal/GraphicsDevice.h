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

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class GraphicsDevice : public DKGraphicsDeviceInterface
			{
			public:
				GraphicsDevice(void);
				~GraphicsDevice(void);

				DKString DeviceName(void) const override;
				DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*, uint32_t) override;
                DKObject<DKShaderModule> CreateShaderModule(DKGraphicsDevice*, DKShader*) override;
                DKObject<DKRenderPipelineState> CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKPipelineReflection*) override;
                DKObject<DKComputePipelineState> CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*) override;
				DKObject<DKGpuBuffer> CreateBuffer(size_t, DKGpuStorageMode, DKCpuCacheMode);
				DKObject<DKTexture> CreateTexture(const DKTextureDescriptor&);

			private:
				id<MTLDevice> device;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
