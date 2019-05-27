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
#include "DescriptorSet.h"

namespace DKFramework::Private::Vulkan
{
	class RenderCommandEncoder : public DKRenderCommandEncoder
	{
        class Encoder;
        struct EncodingState
        {
            Encoder* encoder;
            RenderPipelineState* pipelineState;
            ShaderBindingSet::ImageLayoutMap imageLayoutMap;
            ShaderBindingSet::ImageViewLayoutMap imageViewLayoutMap;
            DKMap<ShaderBindingSet*, DescriptorSet*> bindingSetMap;
        };
        using EncoderCommand = DKFunctionSignature<void(VkCommandBuffer, EncodingState&)>;
        class Encoder : public CommandEncoder
        {
        public:
            Encoder(class CommandBuffer*, const DKRenderPassDescriptor& desc);
            ~Encoder();

            bool Encode(VkCommandBuffer) override;

            // Retain ownership of all encoded objects
            DKArray<DKObject<RenderPipelineState>> pipelineStateObjects;
            DKArray<DKObject<ShaderBindingSet>> shaderBindingSets;
            DKArray<DKObject<DescriptorSet>> descriptorSets;
            DKArray<DKObject<DKGpuBuffer>> buffers;
            DKArray<DKObject<DKGpuEvent>> events;

            DKRenderPassDescriptor renderPassDescriptor;

            VkFramebuffer		framebuffer;
            VkRenderPass		renderPass;

            class CommandBuffer* commandBuffer;
            DKArray<DKObject<EncoderCommand>> commands;
            DKArray<DKObject<EncoderCommand>> setupCommands;    // before renderPass
            DKArray<DKObject<EncoderCommand>> cleanupCommands;  // after renderPass
        };
        DKObject<Encoder> encoder;

	public:
		RenderCommandEncoder(class CommandBuffer*, const DKRenderPassDescriptor&);

		void EndEncoding() override;
		bool IsCompleted() const override { return encoder == nullptr; }
		DKCommandBuffer* CommandBuffer() override { return commandBuffer; }

        void WaitEvent(DKGpuEvent*, DKRenderStages) override;
        void SignalEvent(DKGpuEvent*) override;

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
