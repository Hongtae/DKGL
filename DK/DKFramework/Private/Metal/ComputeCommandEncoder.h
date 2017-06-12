//
//  File: ComputeCommandEncoder.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKComputeCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class ComputeCommandEncoder : public DKComputeCommandEncoder
			{
			public:
				ComputeCommandEncoder(id<MTLComputeCommandEncoder>, CommandBuffer*);
				~ComputeCommandEncoder(void);

				void EndEncoding(void) override;
				DKCommandBuffer* Buffer(void) override;

			private:
				DKObject<CommandBuffer> buffer;
				id<MTLComputeCommandEncoder> encoder;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
