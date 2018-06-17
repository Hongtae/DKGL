//
//  File: Types.mm
//  Platform: macOS: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "Types.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

MTLDataType ShaderDataType::From(DKShaderDataType t)
{
	switch (t)
	{
		case DKShaderDataType::Struct:	return MTLDataTypeStruct;
		case DKShaderDataType::Texture:	return MTLDataTypeTexture;
		case DKShaderDataType::Sampler:	return MTLDataTypeSampler;

		case DKShaderDataType::Float:	return MTLDataTypeFloat;
		case DKShaderDataType::Float2:	return MTLDataTypeFloat2;
		case DKShaderDataType::Float3:	return MTLDataTypeFloat3;
		case DKShaderDataType::Float4:	return MTLDataTypeFloat4;

		case DKShaderDataType::Float2x2:	return MTLDataTypeFloat2x2;
		case DKShaderDataType::Float2x3:	return MTLDataTypeFloat2x3;
		case DKShaderDataType::Float2x4:	return MTLDataTypeFloat2x4;

		case DKShaderDataType::Float3x2:	return MTLDataTypeFloat3x2;
		case DKShaderDataType::Float3x3:	return MTLDataTypeFloat3x3;
		case DKShaderDataType::Float3x4:	return MTLDataTypeFloat3x4;

		case DKShaderDataType::Float4x2:	return MTLDataTypeFloat4x2;
		case DKShaderDataType::Float4x3:	return MTLDataTypeFloat4x3;
		case DKShaderDataType::Float4x4:	return MTLDataTypeFloat4x4;

		case DKShaderDataType::Half:	return MTLDataTypeHalf;
		case DKShaderDataType::Half2:	return MTLDataTypeHalf2;
		case DKShaderDataType::Half3:	return MTLDataTypeHalf3;
		case DKShaderDataType::Half4:	return MTLDataTypeHalf4;

		case DKShaderDataType::Half2x2:	return MTLDataTypeHalf2x2;
		case DKShaderDataType::Half2x3:	return MTLDataTypeHalf2x3;
		case DKShaderDataType::Half2x4:	return MTLDataTypeHalf2x4;

		case DKShaderDataType::Half3x2:	return MTLDataTypeHalf3x2;
		case DKShaderDataType::Half3x3:	return MTLDataTypeHalf3x3;
		case DKShaderDataType::Half3x4:	return MTLDataTypeHalf3x4;

		case DKShaderDataType::Half4x2:	return MTLDataTypeHalf4x2;
		case DKShaderDataType::Half4x3:	return MTLDataTypeHalf4x3;
		case DKShaderDataType::Half4x4:	return MTLDataTypeHalf4x4;

		case DKShaderDataType::Int:	return MTLDataTypeInt;
		case DKShaderDataType::Int2:	return MTLDataTypeInt2;
		case DKShaderDataType::Int3:	return MTLDataTypeInt3;
		case DKShaderDataType::Int4:	return MTLDataTypeInt4;

		case DKShaderDataType::UInt:	return MTLDataTypeUInt;
		case DKShaderDataType::UInt2:	return MTLDataTypeUInt2;
		case DKShaderDataType::UInt3:	return MTLDataTypeUInt3;
		case DKShaderDataType::UInt4:	return MTLDataTypeUInt4;

		case DKShaderDataType::Short:	return MTLDataTypeShort;
		case DKShaderDataType::Short2:	return MTLDataTypeShort2;
		case DKShaderDataType::Short3:	return MTLDataTypeShort3;
		case DKShaderDataType::Short4:	return MTLDataTypeShort4;

		case DKShaderDataType::UShort:	return MTLDataTypeUShort;
		case DKShaderDataType::UShort2:	return MTLDataTypeUShort2;
		case DKShaderDataType::UShort3:	return MTLDataTypeUShort3;
		case DKShaderDataType::UShort4:	return MTLDataTypeUShort4;

		case DKShaderDataType::Char:	return MTLDataTypeChar;
		case DKShaderDataType::Char2:	return MTLDataTypeChar2;
		case DKShaderDataType::Char3:	return MTLDataTypeChar3;
		case DKShaderDataType::Char4:	return MTLDataTypeChar4;

		case DKShaderDataType::UChar:	return MTLDataTypeUChar;
		case DKShaderDataType::UChar2:	return MTLDataTypeUChar2;
		case DKShaderDataType::UChar3:	return MTLDataTypeUChar3;
		case DKShaderDataType::UChar4:	return MTLDataTypeUChar4;

		case DKShaderDataType::Bool:	return MTLDataTypeBool;
		case DKShaderDataType::Bool2:	return MTLDataTypeBool2;
		case DKShaderDataType::Bool3:	return MTLDataTypeBool3;
		case DKShaderDataType::Bool4:	return MTLDataTypeBool4;
	}
	return MTLDataTypeNone;
}

DKShaderDataType ShaderDataType::To(MTLDataType t)
{
	switch (t)
	{
		case MTLDataTypeStruct:	return DKShaderDataType::Struct;
		case MTLDataTypeTexture:	return DKShaderDataType::Texture;
		case MTLDataTypeSampler:	return DKShaderDataType::Sampler;

		case MTLDataTypeFloat:	return DKShaderDataType::Float;
		case MTLDataTypeFloat2:	return DKShaderDataType::Float2;
		case MTLDataTypeFloat3:	return DKShaderDataType::Float3;
		case MTLDataTypeFloat4:	return DKShaderDataType::Float4;

		case MTLDataTypeFloat2x2:	return DKShaderDataType::Float2x2;
		case MTLDataTypeFloat2x3:	return DKShaderDataType::Float2x3;
		case MTLDataTypeFloat2x4:	return DKShaderDataType::Float2x4;

		case MTLDataTypeFloat3x2:	return DKShaderDataType::Float3x2;
		case MTLDataTypeFloat3x3:	return DKShaderDataType::Float3x3;
		case MTLDataTypeFloat3x4:	return DKShaderDataType::Float3x4;

		case MTLDataTypeFloat4x2:	return DKShaderDataType::Float4x2;
		case MTLDataTypeFloat4x3:	return DKShaderDataType::Float4x3;
		case MTLDataTypeFloat4x4:	return DKShaderDataType::Float4x4;

		case MTLDataTypeHalf:	return DKShaderDataType::Half;
		case MTLDataTypeHalf2:	return DKShaderDataType::Half2;
		case MTLDataTypeHalf3:	return DKShaderDataType::Half3;
		case MTLDataTypeHalf4:	return DKShaderDataType::Half4;

		case MTLDataTypeHalf2x2:	return DKShaderDataType::Half2x2;
		case MTLDataTypeHalf2x3:	return DKShaderDataType::Half2x3;
		case MTLDataTypeHalf2x4:	return DKShaderDataType::Half2x4;

		case MTLDataTypeHalf3x2:	return DKShaderDataType::Half3x2;
		case MTLDataTypeHalf3x3:	return DKShaderDataType::Half3x3;
		case MTLDataTypeHalf3x4:	return DKShaderDataType::Half3x4;

		case MTLDataTypeHalf4x2:	return DKShaderDataType::Half4x2;
		case MTLDataTypeHalf4x3:	return DKShaderDataType::Half4x3;
		case MTLDataTypeHalf4x4:	return DKShaderDataType::Half4x4;

		case MTLDataTypeInt:	return DKShaderDataType::Int;
		case MTLDataTypeInt2:	return DKShaderDataType::Int2;
		case MTLDataTypeInt3:	return DKShaderDataType::Int3;
		case MTLDataTypeInt4:	return DKShaderDataType::Int4;

		case MTLDataTypeUInt:	return DKShaderDataType::UInt;
		case MTLDataTypeUInt2:	return DKShaderDataType::UInt2;
		case MTLDataTypeUInt3:	return DKShaderDataType::UInt3;
		case MTLDataTypeUInt4:	return DKShaderDataType::UInt4;

		case MTLDataTypeShort:	return DKShaderDataType::Short;
		case MTLDataTypeShort2:	return DKShaderDataType::Short2;
		case MTLDataTypeShort3:	return DKShaderDataType::Short3;
		case MTLDataTypeShort4:	return DKShaderDataType::Short4;

		case MTLDataTypeUShort:	return DKShaderDataType::UShort;
		case MTLDataTypeUShort2:	return DKShaderDataType::UShort2;
		case MTLDataTypeUShort3:	return DKShaderDataType::UShort3;
		case MTLDataTypeUShort4:	return DKShaderDataType::UShort4;

		case MTLDataTypeChar:	return DKShaderDataType::Char;
		case MTLDataTypeChar2:	return DKShaderDataType::Char2;
		case MTLDataTypeChar3:	return DKShaderDataType::Char3;
		case MTLDataTypeChar4:	return DKShaderDataType::Char4;

		case MTLDataTypeUChar:	return DKShaderDataType::UChar;
		case MTLDataTypeUChar2:	return DKShaderDataType::UChar2;
		case MTLDataTypeUChar3:	return DKShaderDataType::UChar3;
		case MTLDataTypeUChar4:	return DKShaderDataType::UChar4;

		case MTLDataTypeBool:	return DKShaderDataType::Bool;
		case MTLDataTypeBool2:	return DKShaderDataType::Bool2;
		case MTLDataTypeBool3:	return DKShaderDataType::Bool3;
		case MTLDataTypeBool4:	return DKShaderDataType::Bool4;
	}
	return DKShaderDataType::None;
}

#endif
