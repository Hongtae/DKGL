//
//  File: DKRenderPipeline.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKPixelFormat.h"
#include "DKShaderFunction.h"
#include "DKVertexDescriptor.h"
#include "DKDepthStencil.h"
#include "DKBlendState.h"

namespace DKFramework
{
	struct DKRenderPipelineColorAttachmentDescriptor
	{
		uint32_t index;
		DKPixelFormat pixelFormat;

		DKBlendState blendState;
	};

	enum class DKPrimitiveType
	{
		Point,
		Line,
		LineStrip,
		Triangle,
		TriangleStrip,
	};
	enum class DKIndexType
	{
		UInt16,
		UInt32,
	};
	enum class DKTriangleFillMode
	{
		Fill,
		Lines,
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
		CCW,		///< Counter Clockwise
	};
	enum class DKDepthClipMode
	{
		Clip,
		Clamp,
	};

	struct DKRenderPipelineDescriptor
	{
		DKObject<DKShaderFunction> vertexFunction;
		DKObject<DKShaderFunction> fragmentFunction;
		DKVertexDescriptor vertexDescriptor;
		DKArray<DKRenderPipelineColorAttachmentDescriptor> colorAttachments;
		DKPixelFormat depthStencilAttachmentPixelFormat;

        DKDepthStencilDescriptor depthStencilDescriptor;

		DKPrimitiveType primitiveTopology;
		DKTriangleFillMode triangleFillMode = DKTriangleFillMode::Fill;
		DKDepthClipMode depthClipMode = DKDepthClipMode::Clip;
		DKCullMode cullMode = DKCullMode::Back;
		DKFrontFace frontFace = DKFrontFace::CCW;

		bool rasterizationEnabled; // Do we need this flag? What about use null-frag shader instead?
	};

	class DKGraphicsDevice;
	class DKRenderPipelineState
	{
	public:
		virtual ~DKRenderPipelineState() {}
		virtual DKGraphicsDevice* Device() = 0;
	};
}
