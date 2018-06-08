//
//  File: DKRenderPipelineReflection.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKShader.h"
#include "DKShaderFunction.h"
#include "DKTexture.h"

namespace DKFramework
{
	struct DKShaderResourceBuffer
	{
		DKShaderDataType dataType;
		uint32_t alignment;
		uint32_t size;

		struct DKShaderResourceStruct* structType;
	};
	struct DKShaderResourceTexture
	{
		DKShaderDataType dataType;
		DKTexture::Type textureType;
	};
	struct DKShaderResourceThreadgroup
	{
		uint32_t alignment;
		uint32_t size;
	};
	struct DKShaderResourceArray
	{
		DKShaderDataType elementType;
		uint32_t arrayLength;
		uint32_t stride;
	};
	struct DKShaderResourceStructMember
	{
		DKString name;
		DKShaderDataType dataType;
		uint32_t offset;
		uint32_t binding; // shader binding location for single item.

		union
		{
			struct DKShaderResourceArray* arrayType;   // for dataType == DKShaderDataType::Array
			struct DKShaderResourceStruct* structType; // for dataType == DKShaderDataType::Struct
		} typeInfo;
	};
	struct DKShaderResourceStruct
	{
		DKArray<DKShaderResourceStructMember> members;
	};
	struct DKShaderResource
	{
		enum Type
		{
			TypeBuffer,
			TypeTexture,
			TypeSampler,
			TypeThreadgroupMemory,
		};

		uint32_t set;
		uint32_t binding;
		DKString name;
		Type type;
		size_t count; // array length
		bool enabled;

		union
		{
			// only one of these is valid depending on the type.
			DKShaderResourceBuffer buffer;
			DKShaderResourceTexture texture;
			DKShaderResourceThreadgroup threadgroup;
		} typeInfo;

		// type data for struct members
		DKMap<DKString, DKShaderResourceArray> arrayTypeMemberMap;
		DKMap<DKString, DKShaderResourceStruct> structTypeMemberMap;
	};

	/**
	@brief Render Pipeline's shader reflection
	*/
	struct DKRenderPipelineReflection
	{
		DKArray<DKShaderResource> vertexResources;
		DKArray<DKShaderResource> fragmentResources;
	};
	/**
	@brief Compute Pipeline's shader reflection
	*/struct DKComputePipelineReflection
	{
		DKArray<DKShaderResource> resources;
	};
}
