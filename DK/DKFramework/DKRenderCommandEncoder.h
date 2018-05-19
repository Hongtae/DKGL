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

	/// @brief Command encoder for GPU render operation.
	class DKRenderCommandEncoder : public DKCommandEncoder
	{
	public:
		virtual ~DKRenderCommandEncoder(void) {}

		virtual void SetRenderPipelineState(DKRenderPipelineState*) = 0;
	};
}
