//
//  File: RenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKRenderCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class RenderCommandEncoder : public DKRenderCommandEncoder
			{
			public:
				RenderCommandEncoder(ID3D12GraphicsCommandList*, class CommandBuffer*, const DKRenderPassDescriptor&);
				~RenderCommandEncoder();


				void EndEncoding() override;
				DKCommandBuffer* CommandBuffer() override;
                bool IsCompleted() const override;

				ComPtr<ID3D12GraphicsCommandList> commandList;
				DKObject<class CommandBuffer> commandBuffer;

                void SetResources(uint32_t set, DKShaderBindingSet*) override;
                void SetViewport(const DKViewport&) override;
                void SetRenderPipelineState(DKRenderPipelineState*) override;
                void SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index) override;
                void SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count) override;
                void SetIndexBuffer(DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type) override;

                void Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance) override;
                void DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance) override;

			private:
				DKArray<D3D12_RESOURCE_BARRIER> rtBarriers;
				DKRenderPassDescriptor renderPassDesc;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
