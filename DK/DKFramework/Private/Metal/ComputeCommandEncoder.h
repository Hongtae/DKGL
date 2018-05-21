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
			class ComputeCommandEncoder : public DKComputeCommandEncoder, public ReusableCommandEncoder
			{
			public:
				ComputeCommandEncoder(CommandBuffer*);
				~ComputeCommandEncoder(void);

				// DKCommandEncoder overrides
				void EndEncoding(void) override;
				bool IsCompleted(void) const override { return buffer == nullptr; }
				DKCommandBuffer* Buffer(void) override { return buffer; }

				// DKComputeCommandEncoder

				// ReusableCommandEncoder overrides
				bool EncodeBuffer(id<MTLCommandBuffer>) override;
				void CompleteBuffer(void) override { buffer = nullptr; }

			private:
				DKObject<CommandBuffer> buffer;

				using EncoderCommand = DKFunctionSignature<void (id<MTLComputeCommandEncoder>)>;
				DKArray<DKObject<EncoderCommand>> encoderCommands;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
