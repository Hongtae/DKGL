//
//  File: DKShaderFunction.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	class DKGraphicsDevice;

	enum class DKShaderDataType
	{
		None = 0,

		Struct,
		Array,

		Float,
		Float2,
		Float3,
		Float4,

		Float2x2,
		Float2x3,
		Float2x4,

		Float3x2,
		Float3x3,
		Float3x4,

		Float4x2,
		Float4x3,
		Float4x4,

		Half,
		Half2,
		Half3,
		Half4,

		Half2x2,
		Half2x3,
		Half2x4,

		Half3x2,
		Half3x3,
		Half3x4,

		Half4x2,
		Half4x3,
		Half4x4,

		Int,
		Int2,
		Int3,
		Int4,

		UInt,
		UInt2,
		UInt3,
		UInt4,

		Short,
		Short2,
		Short3,
		Short4,

		UShort,
		UShort2,
		UShort3,
		UShort4,
		
		Char,
		Char2,
		Char3,
		Char4,
		
		UChar,
		UChar2,
		UChar3,
		UChar4,
		
		Bool,
		Bool2,
		Bool3,
		Bool4,
	};

	struct DKShaderAttribute
	{
		DKString name;
		uint32_t index;
		DKShaderDataType type;
		bool active;
		bool patchControlPointData;
		bool patchData;
	};

	using DKVertexAttribute = DKShaderAttribute;

	class DKShaderFunction
	{
	public:
		struct Constant
		{
			DKString name;
			DKShaderDataType type;
			uint32_t index;
			bool required;
		};

		virtual ~DKShaderFunction(void) {}

		virtual const DKArray<DKVertexAttribute>& VertexAttributes(void) const = 0;
		virtual const DKArray<DKShaderAttribute>& StageInputAttributes(void) const = 0;
		virtual const DKMap<DKString, Constant>& FunctionConstants(void) const = 0;

		virtual DKGraphicsDevice* Device(void) = 0;
	};
}
