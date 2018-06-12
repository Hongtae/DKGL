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
	struct DKShaderResourceStructMember
	{
		DKShaderDataType dataType;
		DKString name;
		uint32_t offset;
		uint32_t size;
		uint32_t count; // array length
		uint32_t stride; // stride between array elements

		DKString typeInfoKey; // arrayType or structType key
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

		uint32_t count; // array length
		uint32_t stride; // stride between array elements

		bool writable;
		bool enabled;

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
