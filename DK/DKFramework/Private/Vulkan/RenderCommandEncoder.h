//
//  File: RenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKRenderCommandEncoder.h"
#include "CommandBuffer.h"
#include "SwapChain.h"
#include "RenderPipelineState.h"
#include "ShaderBindingSet.h"

namespace DKFramework::Private::Vulkan
{
	class RenderCommandEncoder : public DKRenderCommandEncoder
	{
        struct EncodingState
        {
            RenderPipelineState* pipelineState;
        };
        using EncoderCommand = DKFunctionSignature<void(VkCommandBuffer, EncodingState&)>;
        class Encoder : public CommandBufferEncoder
        {
        public:
            Encoder(class CommandBuffer*, const DKRenderPassDescriptor& desc);
            ~Encoder();

            bool Encode(VkCommandBuffer) override;

            // Retain ownership of all encoded objects
            DKArray<DKObject<RenderPipelineState>> pipelineStateObjects;
            DKArray<DKObject<ShaderBindingSet>> shaderBindingSets;
            DKArray<DKObject<DKGpuBuffer>> buffers;

            DKRenderPassDescriptor renderPassDescriptor;

            VkFramebuffer		framebuffer;
            VkRenderPass		renderPass;

            class CommandBuffer* commandBuffer;
            DKArray<DKObject<EncoderCommand>> commands;
        };
        DKObject<Encoder> encoder;

	public:
		RenderCommandEncoder(class CommandBuffer*, const DKRenderPassDescriptor&);

		void EndEncoding() override;
		bool IsCompleted() const override { return encoder == nullptr; }
		DKCommandBuffer* CommandBuffer() override { return commandBuffer; }

        void SetResources(uint32_t set, DKShaderBindingSet*) override;

		void SetViewport(const DKViewport&) override;
		void SetRenderPipelineState(DKRenderPipelineState*) override;
		void SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index) override;
		void SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count) override;
		void SetIndexBuffer(DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type) override;

		void Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance) override;
		void DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance) override;

		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
