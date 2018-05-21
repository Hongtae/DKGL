//
//  File: RenderCommandEncoder.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKRenderCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class RenderCommandEncoder : public DKRenderCommandEncoder, public ReusableCommandEncoder
			{
			public:
				RenderCommandEncoder(MTLRenderPassDescriptor*, CommandBuffer*);
				~RenderCommandEncoder(void);

				// DKCommandEncoder overrides
				void EndEncoding(void) override;
				bool IsCompleted(void) const override { return buffer == nullptr; }
				DKCommandBuffer* Buffer(void) override { return buffer; }

				// DKRenderCommandEncoder overrides
				void SetRenderPipelineState(DKRenderPipelineState*) override;

				// ReusableCommandEncoder overrides
				bool EncodeBuffer(id<MTLCommandBuffer>) override;
				void CompleteBuffer(void) override { buffer = nullptr; }

			private:
				DKObject<CommandBuffer> buffer;
				MTLRenderPassDescriptor* renderPassDescriptor;

				using EncoderCommand = DKFunctionSignature<void (id<MTLRenderCommandEncoder>)>;
				DKArray<DKObject<EncoderCommand>> encoderCommands;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
