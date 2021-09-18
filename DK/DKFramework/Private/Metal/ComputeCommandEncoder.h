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
#include "Event.h"
#include "Semaphore.h"

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
        void WaitEvent(const DKGpuEvent*) override;
        void SignalEvent(const DKGpuEvent*) override;
        void WaitSemaphoreValue(const DKGpuSemaphore*, uint64_t) override;
        void SignalSemaphoreValue(const DKGpuSemaphore*, uint64_t) override;

        void SetResources(uint32_t set, const DKShaderBindingSet*) override;
        void SetComputePipelineState(const DKComputePipelineState*) override;

        void PushConstant(uint32_t stages, uint32_t offset, uint32_t size, const void*) override;

        void Dispatch(uint32_t, uint32_t, uint32_t) override;
        
	private:
        class Encoder;
        struct EncodingState
        {
            Encoder* encoder;
            DKObject<ComputePipelineState> pipelineState;
        };
		using EncoderCommand = DKFunctionSignature<void(id<MTLComputeCommandEncoder>, EncodingState&)>;
		class Encoder : public CommandEncoder
		{
        public:
            bool Encode(id<MTLCommandBuffer> buffer) override;
			DKArray<DKObject<EncoderCommand>> commands;
            
            DKArray<DKObject<DKGpuEvent>> events;
            DKArray<DKObject<DKGpuSemaphore>> semaphores;

            DKSet<Event*> waitEvents;
            DKSet<Event*> signalEvents;
            DKMap<Semaphore*, uint64_t> waitSemaphores;
            DKMap<Semaphore*, uint64_t> signalSemaphores;

            DKArray<uint8_t> pushConstants;
		};
		DKObject<Encoder> encoder;
		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_METAL
