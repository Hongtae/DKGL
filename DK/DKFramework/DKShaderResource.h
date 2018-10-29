//
//  File: DKShaderResource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2018 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKTexture.h"

namespace DKFramework
{
	enum class DKShaderDataType
	{
		Unknown = -1,
		None = 0,

		Struct,
		Texture,
		Sampler,

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

	enum class DKShaderStage
	{
		Unknown = 0,
		Vertex = 1U,
		TessellationControl = 1U << 1,
		TessellationEvaluation = 1U << 2,
		Geometry = 1U << 3,
		Fragment = 1U << 4,
		Compute = 1U << 5,
	};

	/**
	 Shader reflection data for buffer type.
	 */
	struct DKShaderResourceBuffer
	{
		DKShaderDataType dataType;
		//uint32_t alignment;
		uint32_t size;
	};
	/**
	 Shader reflection data for texture type.
	 */
	struct DKShaderResourceTexture
	{
		DKShaderDataType dataType;
		DKTexture::Type textureType;
	};
	/**
	 Shader reflection data for threadgroup type.
	 */
	struct DKShaderResourceThreadgroup
	{
		uint32_t alignment;
		uint32_t size;
	};
	/**
	 Shader reflection data for struct member type.
	 */
	struct DKShaderResourceStructMember
	{
		DKShaderDataType dataType;
		DKString name;
        uint32_t offset;
        //uint32_t size;
		uint32_t count; // array length
		uint32_t stride; // stride between array elements

		DKString typeInfoKey; // structType key
	};
	/**
	 Shader reflection data for struct type.
	 */
	struct DKShaderResourceStruct
	{
		DKArray<DKShaderResourceStructMember> members;
	};
	/**
	 Shader reflection data for resource descriptor.
	 */
	struct DKShaderResource
	{
		enum Type
		{
			TypeBuffer,
			TypeTexture,
			TypeSampler,
			TypeSampledTexture,
		};
		enum Access
		{
			AccessReadOnly,
			AccessWriteOnly,
			AccessReadWrite,
		};

		uint32_t set;
		uint32_t binding;
		DKString name;
		Type type;
		uint32_t stages;

		uint32_t count; // array length
		uint32_t stride; // stride between array elements

		bool enabled;
		Access access;

		union
		{
			// only one of these is valid depending on the type.
			DKShaderResourceBuffer buffer;
			DKShaderResourceTexture texture;
			DKShaderResourceThreadgroup threadgroup;
		} typeInfo;

		DKString typeInfoKey; // structType key

		// type data for struct members
		DKMap<DKString, DKShaderResourceStruct> structTypeMemberMap;
	};

	struct DKShaderPushConstantLayout
	{
		DKString name;
		uint32_t offset;
		uint32_t size;
		uint32_t stages;
		DKArray<DKShaderPushConstantLayout> memberLayouts;
	};
}
