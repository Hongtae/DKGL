//
//  File: ComputeCommandEncoder.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKComputeCommandEncoder.h"
#include "CommandBuffer.h"
#include "ComputePipelineState.h"

namespace DKFramework::Private::Metal
{
	class ComputeCommandEncoder : public DKComputeCommandEncoder
	{
	public:
		ComputeCommandEncoder(class CommandBuffer*);
		~ComputeCommandEncoder();

		// DKCommandEncoder overrides
		void EndEncoding() override;
		bool IsCompleted() const override { return reusableEncoder == nullptr; }
		DKCommandBuffer* CommandBuffer() override { return commandBuffer; }

		// DKComputeCommandEncoder
        void SetResources(uint32_t set, DKShaderBindingSet*) override;
        void SetComputePipelineState(DKComputePipelineState*) override;

	private:
        struct Resources
        {
            DKObject<ComputePipelineState> pipelineState;
        };
		using EncoderCommand = DKFunctionSignature<void(id<MTLComputeCommandEncoder>, Resources&)>;
		struct ReusableEncoder : public ReusableCommandEncoder
		{
			bool EncodeBuffer(id<MTLCommandBuffer> buffer) override
			{
				id<MTLComputeCommandEncoder> encoder = [buffer computeCommandEncoder];
                Resources res = {};
				for (EncoderCommand* command : encoderCommands )
				{
					command->Invoke(encoder, res);
				}
				[encoder endEncoding];
				return true;
			}
			DKArray<DKObject<EncoderCommand>> encoderCommands;
		};
		DKObject<ReusableEncoder> reusableEncoder;
		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_METAL
