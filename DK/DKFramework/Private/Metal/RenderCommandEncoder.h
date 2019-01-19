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

namespace DKFramework::Private::Metal
{
	class RenderCommandEncoder : public DKRenderCommandEncoder
	{
	public:
		RenderCommandEncoder(MTLRenderPassDescriptor*, CommandBuffer*);
		~RenderCommandEncoder();

		// DKCommandEncoder overrides
		void EndEncoding() override;
		bool IsCompleted() const override { return reusableEncoder == nullptr; }
		DKCommandBuffer* Buffer() override { return buffer; }

		// DKRenderCommandEncoder overrides
        void SetResources(uint32_t set, DKShaderBindingSet*) override;
		void SetViewport(const DKViewport&) override;
		void SetRenderPipelineState(DKRenderPipelineState*) override;
		void SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index) override;
		void SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count) override;
		void SetIndexBuffer(DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type) override;

		void Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance) override;
		void DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance) override;

	private:
		struct Resources
		{
			DKObject<RenderPipelineState> pipelineState;
			DKObject<class Buffer> indexBuffer;
			size_t indexBufferOffset;
			MTLIndexType indexBufferType;
		};
		using EncoderCommand = DKFunctionSignature<void(id<MTLRenderCommandEncoder>, Resources&)>;

		struct ReusableEncoder : public ReusableCommandEncoder
		{
			~ReusableEncoder()
			{
				[renderPassDescriptor autorelease];
			}

			bool EncodeBuffer(id<MTLCommandBuffer> buffer) override
			{
				DKASSERT_DEBUG(renderPassDescriptor);
				if (renderPassDescriptor)
				{
					id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
					Resources res = {};
					for (EncoderCommand* command : encoderCommands )
					{
						command->Invoke(encoder, res);
					}
					[encoder endEncoding];
					return true;
				}
				return false;
			}

			DKArray<DKObject<EncoderCommand>> encoderCommands;
			MTLRenderPassDescriptor* renderPassDescriptor;
		};

		DKObject<ReusableEncoder> reusableEncoder;
		DKObject<CommandBuffer> buffer;
	};
}
#endif //#if DKGL_ENABLE_METAL
