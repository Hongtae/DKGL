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
		bool IsCompleted() const override { return encoder == nullptr; }
		DKCommandBuffer* CommandBuffer() override { return commandBuffer; }

		// DKComputeCommandEncoder
        void WaitEvent(DKGpuEvent*) override;
        void SignalEvent(DKGpuEvent*) override;

        void SetResources(uint32_t set, DKShaderBindingSet*) override;
        void SetComputePipelineState(DKComputePipelineState*) override;
        void Dispatch(uint32_t, uint32_t, uint32_t) override;
        
	private:
        struct EncodingState
        {
            DKObject<ComputePipelineState> pipelineState;
        };
		using EncoderCommand = DKFunctionSignature<void(id<MTLComputeCommandEncoder>, EncodingState&)>;
		class Encoder : public CommandEncoder
		{
        public:
            bool Encode(id<MTLCommandBuffer> buffer) override;
			DKArray<DKObject<EncoderCommand>> commands;
            
            DKArray<DKObject<DKGpuEvent>> waitEvents;
            DKArray<DKObject<DKGpuEvent>> signalEvents;
		};
		DKObject<Encoder> encoder;
		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_METAL
