//
//  File: CommandBuffer.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#import <Metal/Metal.h>

#include "../../DKCommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class CommandBuffer : public DKCommandBuffer
			{
			public:
				~CommandBuffer(void);

				DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(DKRenderPassDescriptor*) override;
				DKObject<DKParallelRenderCommandEncoder> CreateParallelRenderCommandEncoder(DKRenderPassDescriptor*) override;

				DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder(void) override;
				DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder(void) override;


				id<MTLCommandBuffer> buffer;
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
