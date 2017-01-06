//
//  File: DKRenderPipeline.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKRenderPass.h"
#include "DKRenderCommandEncoder.h"

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

	enum class DKColorWriteMask
	{
		None	= 0,
		Red		= 0x1 << 3,
		Green	= 0x1 << 2,
		Blue	= 0x1 << 1,
		Alpha	= 0x1 << 0,
		All		= 0xf
	};

	class DKRenderPipelineDescriptor
	{
	};

	class DKRenderPipelineColorAttachmentDescriptor
	{
	};

	class DKRenderPipelineState
	{
	};
}
