//
//  File: DKRenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKRenderPass.h"
#include "DKRenderPipeline.h"
#include "DKCommandEncoder.h"
#include "DKGpuBuffer.h"
#include "DKShaderBindingSet.h"

namespace DKFramework
{
	enum class DKVisibilityResultMode
	{
		Disabled,
		Boolean,
		Counting,
	};
	struct DKViewport
	{
		float x;
		float y;
		float width;
		float height;
		float nearZ;
		float farZ;
	};
	/// @brief Command encoder for GPU render operation.
	class DKRenderCommandEncoder : public DKCommandEncoder
	{
	public:
		virtual ~DKRenderCommandEncoder() {}

        virtual void SetResources(uint32_t set, const DKShaderBindingSet*) = 0;
		virtual void SetViewport(const DKViewport&) = 0;
		virtual void SetRenderPipelineState(const DKRenderPipelineState* state) = 0;
		virtual void SetVertexBuffer(const DKGpuBuffer* buffer, size_t offset, uint32_t index) = 0;
		virtual void SetVertexBuffers(const DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count) = 0;
		virtual void SetIndexBuffer(const DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type) = 0;

        virtual void PushConstant(uint32_t stages, uint32_t offset, uint32_t size, const void*) = 0;

		virtual void Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance) = 0;
		virtual void DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance) = 0;

	};
}
