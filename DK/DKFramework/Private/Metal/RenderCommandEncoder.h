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
#include "RenderPipelineState.h"
#include "Buffer.h"

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
				void SetViewport(const DKViewport&) override;
				void SetRenderPipelineState(DKRenderPipelineState*) override;
				void SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index) override;
				void SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count) override;
				void SetIndexBuffer(DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type) override;

				void Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance) override;
				void DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance) override;

				// ReusableCommandEncoder overrides
				bool EncodeBuffer(id<MTLCommandBuffer>) override;
				void CompleteBuffer(void) override { buffer = nullptr; }

			private:
				struct Resources
				{
					DKObject<RenderPipelineState> pipelineState;
					DKObject<class Buffer> indexBuffer;
					size_t indexBufferOffset;
					MTLIndexType indexBufferType;
				};

				DKObject<CommandBuffer> buffer;
				MTLRenderPassDescriptor* renderPassDescriptor;

				using EncoderCommand = DKFunctionSignature<void (id<MTLRenderCommandEncoder>, Resources&)>;
				DKArray<DKObject<EncoderCommand>> encoderCommands;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
