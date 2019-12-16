//
//  File: RenderCommandEncoder.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKRenderCommandEncoder.h"
#include "CommandBuffer.h"
#include "RenderPipelineState.h"
#include "Buffer.h"
#include "Event.h"
#include "Semaphore.h"

namespace DKFramework::Private::Metal
{
	class RenderCommandEncoder : public DKRenderCommandEncoder
	{
	public:
		RenderCommandEncoder(MTLRenderPassDescriptor*, class CommandBuffer*);
		~RenderCommandEncoder();

		// DKCommandEncoder overrides
		void EndEncoding() override;
		bool IsCompleted() const override { return encoder == nullptr; }
		DKCommandBuffer* CommandBuffer() override { return commandBuffer; }

		// DKRenderCommandEncoder overrides
        void WaitEvent(DKGpuEvent*) override;
        void SignalEvent(DKGpuEvent*) override;
        void WaitSemaphoreValue(DKGpuSemaphore*, uint64_t) override;
        void SignalSemaphoreValue(DKGpuSemaphore*, uint64_t) override;

        void SetResources(uint32_t set, DKShaderBindingSet*) override;
		void SetViewport(const DKViewport&) override;
		void SetRenderPipelineState(DKRenderPipelineState*) override;
		void SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index) override;
		void SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count) override;
		void SetIndexBuffer(DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type) override;

		void Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance) override;
		void DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance) override;

	private:
		struct EncodingState
		{
			DKObject<RenderPipelineState> pipelineState;
			DKObject<Buffer> indexBuffer;
			size_t indexBufferOffset;
			MTLIndexType indexBufferType;
		};
		using EncoderCommand = DKFunctionSignature<void(id<MTLRenderCommandEncoder>, EncodingState&)>;
		class Encoder : public CommandEncoder
		{
        public:
            Encoder(MTLRenderPassDescriptor*);
            ~Encoder();
            bool Encode(id<MTLCommandBuffer> buffer) override;

            DKArray<DKObject<EncoderCommand>> commands;
			MTLRenderPassDescriptor* renderPassDescriptor;

            DKArray<DKObject<DKGpuEvent>> events;
            DKArray<DKObject<DKGpuSemaphore>> semaphores;

            DKSet<Event*> waitEvents;
            DKSet<Event*> signalEvents;
            DKMap<Semaphore*, uint64_t> waitSemaphores;
            DKMap<Semaphore*, uint64_t> signalSemaphores;
		};

		DKObject<Encoder> encoder;
		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_METAL
