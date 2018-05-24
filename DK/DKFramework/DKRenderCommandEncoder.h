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

namespace DKFramework
{
	enum class DKPrimitiveType
	{
		Point,
		Line,
		LineStrip,
		Triangle,
		TriangleStrip,
	};

	enum class DKPrimitiveIndexType
	{
		UInt6,
		UInt32,
	};

	enum class DKVisibilityResultMode
	{
		Disabled,
		Boolean,
		Counting,
	};

	enum class DKCullMode
	{
		None,
		Front,
		Back,
	};

	enum class DKFrontFace
	{
		CW,		///< Clockwise
		CCW,	///< Counter Clockwise
	};

	enum class DKDepthClipMode
	{
		Clip,
		Clamp,
	};

	enum class DKTriangleFillMode
	{
		Fill,
		Lines,
	};

	enum class DKIndexType
	{
		UInt16,
		UInt32,
	};

	/// @brief Command encoder for GPU render operation.
	class DKRenderCommandEncoder : public DKCommandEncoder
	{
	public:
		virtual ~DKRenderCommandEncoder(void) {}

		virtual void SetRenderPipelineState(DKRenderPipelineState* state) = 0;
		virtual void SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index) = 0;
		virtual void SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count) = 0;
		virtual void SetIndexBuffer(DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type) = 0;

		virtual void Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance) = 0;
		virtual void DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance) = 0;

	};
}
