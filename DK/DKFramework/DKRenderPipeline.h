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

namespace DKFramework
{
	enum class DKBlendFactor
	{
		Zero,
		One,
		SourceColor,
		OneMinusSourceColor,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestinationColor,
		OneMinusDestinationColor,
		DestinationAlpha,
		OneMinusDestinationAlpha,
		SourceAlphaSaturated,
		BlendColor,
		OneMinusBlendColor,
		BlendAlpha,
		OneMinusBlendAlpha,
	};

	enum class DKBlendOperation
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max,
	};

	enum DKColorWriteMask : uint8_t
	{
		DKColorWriteMaskNone	= 0,
		DKColorWriteMaskRed		= 0x1 << 3,
		DKColorWriteMaskGreen	= 0x1 << 2,
		DKColorWriteMaskBlue	= 0x1 << 1,
		DKColorWriteMaskAlpha	= 0x1 << 0,
		DKColorWriteMaskAll		= 0xf
	};

	struct DKRenderPipelineColorAttachmentDescriptor
	{
		uint32_t index;
		DKPixelFormat pixelFormat;
		DKColorWriteMask writeMask = DKColorWriteMaskAll;

		bool blendingEnabled = false;
		DKBlendOperation alphaBlendOperation = DKBlendOperation::Add;
		DKBlendOperation rgbBlendOperation = DKBlendOperation::Add;

		DKBlendFactor sourceRGBBlendFactor = DKBlendFactor::One;
		DKBlendFactor sourceAlphaBlendFactor = DKBlendFactor::One;
		DKBlendFactor destinationRGBBlendFactor = DKBlendFactor::Zero;
		DKBlendFactor destinationAlphaBlendFactor = DKBlendFactor::Zero;
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
		CCW,	///< Counter Clockwise
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
