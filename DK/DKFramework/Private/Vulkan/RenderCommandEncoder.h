//
//  File: RenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKRenderCommandEncoder.h"

#include "CommandBuffer.h"
#include "SwapChain.h"

namespace DKFramework::Private::Vulkan
{
	class RenderCommandEncoder : public DKRenderCommandEncoder
	{
	public:
		RenderCommandEncoder(VkCommandBuffer, CommandBuffer*, const DKRenderPassDescriptor&);
		~RenderCommandEncoder();

		void EndEncoding() override;
		bool IsCompleted() const override { return resources == nullptr; }
		DKCommandBuffer* Buffer() override { return commandBuffer; }

		void SetViewport(const DKViewport&) override;
		void SetRenderPipelineState(DKRenderPipelineState*) override;
		void SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index) override;
		void SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count) override;
		void SetIndexBuffer(DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type) override;

		void Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance) override;
		void DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance) override;

		struct Resources
		{
			VkFramebuffer		framebuffer;
			VkRenderPass		renderPass;

			DKArray<VkSemaphore>			waitSemaphores;
			DKArray<VkPipelineStageFlags>	waitStageMasks;
			DKArray<VkSemaphore>			signalSemaphores;

			CommandBuffer* cb;
			VkCommandBuffer commandBuffer;

			Resources(CommandBuffer*);
			~Resources();
		};

		void AddWaitSemaphore(VkSemaphore, VkPipelineStageFlags);
		void AddSignalSemaphore(VkSemaphore);

		DKMap<VkSemaphore, VkPipelineStageFlags> semaphorePipelineStageMasks;
		DKSet<VkSemaphore> signalSemaphores;

		DKObject<Resources> resources;
		DKObject<CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
