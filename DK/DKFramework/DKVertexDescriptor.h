//
//  File: DKVertexDescriptor.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	enum class DKVertexFormat
	{
		Invalid = 0,

		UChar2,
		UChar3,
		UChar4,

		Char2,
		Char3,
		Char4,

		UChar2Normalized,
		UChar3Normalized,
		UChar4Normalized,

		Char2Normalized,
		Char3Normalized,
		Char4Normalized,

		UShort2,
		UShort3,
		UShort4,

		Short2,
		Short3,
		Short4,

		UShort2Normalized,
		UShort3Normalized,
		UShort4Normalized,

		Short2Normalized,
		Short3Normalized,
		Short4Normalized,

		Half2,
		Half3,
		Half4,

		Float,
		Float2,
		Float3,
		Float4,

		Int,
		Int2,
		Int3,
		Int4,

		UInt,
		UInt2,
		UInt3,
		UInt4,
		
		Int1010102Normalized,
		UInt1010102Normalized,
	};

	enum class DKVertexStepRate
	{
		Vertex = 0,
		Instance,
	};

	struct DKVertexBufferLayoutDescriptor
	{
		DKVertexStepRate step = DKVertexStepRate::Vertex;
		uint32_t stride = 0;
		uint32_t bufferIndex;
	};

	struct DKVertexAttributeDescriptor
	{
		DKVertexFormat format;
		uint32_t offset;
		uint32_t bufferIndex;
		uint32_t location;
	};

	struct DKVertexDescriptor
	{
		DKArray<DKVertexAttributeDescriptor> attributes;
		DKArray<DKVertexBufferLayoutDescriptor> layouts;
	};
}
