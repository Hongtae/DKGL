//
//  File: Types.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKPixelFormat.h"
#include "../../DKShaderFunction.h"
#include "../../DKShaderResource.h"

namespace DKFramework::Private::Metal
{
    inline MTLDataType ShaderDataType(DKShaderDataType t)
    {
        switch (t)
        {
            case DKShaderDataType::Struct:    return MTLDataTypeStruct;
            case DKShaderDataType::Texture:    return MTLDataTypeTexture;
            case DKShaderDataType::Sampler:    return MTLDataTypeSampler;

            case DKShaderDataType::Float:    return MTLDataTypeFloat;
            case DKShaderDataType::Float2:    return MTLDataTypeFloat2;
            case DKShaderDataType::Float3:    return MTLDataTypeFloat3;
            case DKShaderDataType::Float4:    return MTLDataTypeFloat4;

            case DKShaderDataType::Float2x2:    return MTLDataTypeFloat2x2;
            case DKShaderDataType::Float2x3:    return MTLDataTypeFloat2x3;
            case DKShaderDataType::Float2x4:    return MTLDataTypeFloat2x4;

            case DKShaderDataType::Float3x2:    return MTLDataTypeFloat3x2;
            case DKShaderDataType::Float3x3:    return MTLDataTypeFloat3x3;
            case DKShaderDataType::Float3x4:    return MTLDataTypeFloat3x4;

            case DKShaderDataType::Float4x2:    return MTLDataTypeFloat4x2;
            case DKShaderDataType::Float4x3:    return MTLDataTypeFloat4x3;
            case DKShaderDataType::Float4x4:    return MTLDataTypeFloat4x4;

            case DKShaderDataType::Half:    return MTLDataTypeHalf;
            case DKShaderDataType::Half2:    return MTLDataTypeHalf2;
            case DKShaderDataType::Half3:    return MTLDataTypeHalf3;
            case DKShaderDataType::Half4:    return MTLDataTypeHalf4;

            case DKShaderDataType::Half2x2:    return MTLDataTypeHalf2x2;
            case DKShaderDataType::Half2x3:    return MTLDataTypeHalf2x3;
            case DKShaderDataType::Half2x4:    return MTLDataTypeHalf2x4;

            case DKShaderDataType::Half3x2:    return MTLDataTypeHalf3x2;
            case DKShaderDataType::Half3x3:    return MTLDataTypeHalf3x3;
            case DKShaderDataType::Half3x4:    return MTLDataTypeHalf3x4;

            case DKShaderDataType::Half4x2:    return MTLDataTypeHalf4x2;
            case DKShaderDataType::Half4x3:    return MTLDataTypeHalf4x3;
            case DKShaderDataType::Half4x4:    return MTLDataTypeHalf4x4;

            case DKShaderDataType::Int:    return MTLDataTypeInt;
            case DKShaderDataType::Int2:    return MTLDataTypeInt2;
            case DKShaderDataType::Int3:    return MTLDataTypeInt3;
            case DKShaderDataType::Int4:    return MTLDataTypeInt4;

            case DKShaderDataType::UInt:    return MTLDataTypeUInt;
            case DKShaderDataType::UInt2:    return MTLDataTypeUInt2;
            case DKShaderDataType::UInt3:    return MTLDataTypeUInt3;
            case DKShaderDataType::UInt4:    return MTLDataTypeUInt4;

            case DKShaderDataType::Short:    return MTLDataTypeShort;
            case DKShaderDataType::Short2:    return MTLDataTypeShort2;
            case DKShaderDataType::Short3:    return MTLDataTypeShort3;
            case DKShaderDataType::Short4:    return MTLDataTypeShort4;

            case DKShaderDataType::UShort:    return MTLDataTypeUShort;
            case DKShaderDataType::UShort2:    return MTLDataTypeUShort2;
            case DKShaderDataType::UShort3:    return MTLDataTypeUShort3;
            case DKShaderDataType::UShort4:    return MTLDataTypeUShort4;

            case DKShaderDataType::Char:    return MTLDataTypeChar;
            case DKShaderDataType::Char2:    return MTLDataTypeChar2;
            case DKShaderDataType::Char3:    return MTLDataTypeChar3;
            case DKShaderDataType::Char4:    return MTLDataTypeChar4;

            case DKShaderDataType::UChar:    return MTLDataTypeUChar;
            case DKShaderDataType::UChar2:    return MTLDataTypeUChar2;
            case DKShaderDataType::UChar3:    return MTLDataTypeUChar3;
            case DKShaderDataType::UChar4:    return MTLDataTypeUChar4;

            case DKShaderDataType::Bool:    return MTLDataTypeBool;
            case DKShaderDataType::Bool2:    return MTLDataTypeBool2;
            case DKShaderDataType::Bool3:    return MTLDataTypeBool3;
            case DKShaderDataType::Bool4:    return MTLDataTypeBool4;
        }
        return MTLDataTypeNone;
    }

    inline DKShaderDataType ShaderDataType(MTLDataType t)
    {
        switch (t)
        {
            case MTLDataTypeStruct:    return DKShaderDataType::Struct;
            case MTLDataTypeTexture:    return DKShaderDataType::Texture;
            case MTLDataTypeSampler:    return DKShaderDataType::Sampler;

            case MTLDataTypeFloat:    return DKShaderDataType::Float;
            case MTLDataTypeFloat2:    return DKShaderDataType::Float2;
            case MTLDataTypeFloat3:    return DKShaderDataType::Float3;
            case MTLDataTypeFloat4:    return DKShaderDataType::Float4;

            case MTLDataTypeFloat2x2:    return DKShaderDataType::Float2x2;
            case MTLDataTypeFloat2x3:    return DKShaderDataType::Float2x3;
            case MTLDataTypeFloat2x4:    return DKShaderDataType::Float2x4;

            case MTLDataTypeFloat3x2:    return DKShaderDataType::Float3x2;
            case MTLDataTypeFloat3x3:    return DKShaderDataType::Float3x3;
            case MTLDataTypeFloat3x4:    return DKShaderDataType::Float3x4;

            case MTLDataTypeFloat4x2:    return DKShaderDataType::Float4x2;
            case MTLDataTypeFloat4x3:    return DKShaderDataType::Float4x3;
            case MTLDataTypeFloat4x4:    return DKShaderDataType::Float4x4;

            case MTLDataTypeHalf:    return DKShaderDataType::Half;
            case MTLDataTypeHalf2:    return DKShaderDataType::Half2;
            case MTLDataTypeHalf3:    return DKShaderDataType::Half3;
            case MTLDataTypeHalf4:    return DKShaderDataType::Half4;

            case MTLDataTypeHalf2x2:    return DKShaderDataType::Half2x2;
            case MTLDataTypeHalf2x3:    return DKShaderDataType::Half2x3;
            case MTLDataTypeHalf2x4:    return DKShaderDataType::Half2x4;

            case MTLDataTypeHalf3x2:    return DKShaderDataType::Half3x2;
            case MTLDataTypeHalf3x3:    return DKShaderDataType::Half3x3;
            case MTLDataTypeHalf3x4:    return DKShaderDataType::Half3x4;

            case MTLDataTypeHalf4x2:    return DKShaderDataType::Half4x2;
            case MTLDataTypeHalf4x3:    return DKShaderDataType::Half4x3;
            case MTLDataTypeHalf4x4:    return DKShaderDataType::Half4x4;

            case MTLDataTypeInt:    return DKShaderDataType::Int;
            case MTLDataTypeInt2:    return DKShaderDataType::Int2;
            case MTLDataTypeInt3:    return DKShaderDataType::Int3;
            case MTLDataTypeInt4:    return DKShaderDataType::Int4;

            case MTLDataTypeUInt:    return DKShaderDataType::UInt;
            case MTLDataTypeUInt2:    return DKShaderDataType::UInt2;
            case MTLDataTypeUInt3:    return DKShaderDataType::UInt3;
            case MTLDataTypeUInt4:    return DKShaderDataType::UInt4;

            case MTLDataTypeShort:    return DKShaderDataType::Short;
            case MTLDataTypeShort2:    return DKShaderDataType::Short2;
            case MTLDataTypeShort3:    return DKShaderDataType::Short3;
            case MTLDataTypeShort4:    return DKShaderDataType::Short4;

            case MTLDataTypeUShort:    return DKShaderDataType::UShort;
            case MTLDataTypeUShort2:    return DKShaderDataType::UShort2;
            case MTLDataTypeUShort3:    return DKShaderDataType::UShort3;
            case MTLDataTypeUShort4:    return DKShaderDataType::UShort4;

            case MTLDataTypeChar:    return DKShaderDataType::Char;
            case MTLDataTypeChar2:    return DKShaderDataType::Char2;
            case MTLDataTypeChar3:    return DKShaderDataType::Char3;
            case MTLDataTypeChar4:    return DKShaderDataType::Char4;

            case MTLDataTypeUChar:    return DKShaderDataType::UChar;
            case MTLDataTypeUChar2:    return DKShaderDataType::UChar2;
            case MTLDataTypeUChar3:    return DKShaderDataType::UChar3;
            case MTLDataTypeUChar4:    return DKShaderDataType::UChar4;

            case MTLDataTypeBool:    return DKShaderDataType::Bool;
            case MTLDataTypeBool2:    return DKShaderDataType::Bool2;
            case MTLDataTypeBool3:    return DKShaderDataType::Bool3;
            case MTLDataTypeBool4:    return DKShaderDataType::Bool4;
        }
        return DKShaderDataType::None;
    }

    inline MTLPixelFormat PixelFormat(DKPixelFormat fmt)
    {
        switch (fmt)
        {
            case DKPixelFormat::R8Unorm:        return MTLPixelFormatR8Unorm;
            case DKPixelFormat::R8Snorm:        return MTLPixelFormatR8Snorm;
            case DKPixelFormat::R8Uint:            return MTLPixelFormatR8Uint;
            case DKPixelFormat::R8Sint:            return MTLPixelFormatR8Sint;

            case DKPixelFormat::R16Unorm:        return MTLPixelFormatR16Unorm;
            case DKPixelFormat::R16Snorm:        return MTLPixelFormatR16Snorm;
            case DKPixelFormat::R16Uint:        return MTLPixelFormatR16Uint;
            case DKPixelFormat::R16Sint:        return MTLPixelFormatR16Sint;
            case DKPixelFormat::R16Float:        return MTLPixelFormatR16Float;

            case DKPixelFormat::RG8Unorm:        return MTLPixelFormatRG8Unorm;
            case DKPixelFormat::RG8Snorm:        return MTLPixelFormatRG8Snorm;
            case DKPixelFormat::RG8Uint:        return MTLPixelFormatRG8Uint;
            case DKPixelFormat::RG8Sint:        return MTLPixelFormatRG8Sint;

            case DKPixelFormat::R32Uint:        return MTLPixelFormatR32Uint;
            case DKPixelFormat::R32Sint:        return MTLPixelFormatR32Sint;
            case DKPixelFormat::R32Float:        return MTLPixelFormatR32Float;

            case DKPixelFormat::RG16Unorm:        return MTLPixelFormatRG16Unorm;
            case DKPixelFormat::RG16Snorm:        return MTLPixelFormatRG16Snorm;
            case DKPixelFormat::RG16Uint:        return MTLPixelFormatRG16Uint;
            case DKPixelFormat::RG16Sint:        return MTLPixelFormatRG16Sint;
            case DKPixelFormat::RG16Float:        return MTLPixelFormatRG16Float;

            case DKPixelFormat::RGBA8Unorm:            return MTLPixelFormatRGBA8Unorm;
            case DKPixelFormat::RGBA8Unorm_sRGB:    return MTLPixelFormatRGBA8Unorm_sRGB;
            case DKPixelFormat::RGBA8Snorm:            return MTLPixelFormatRGBA8Snorm;
            case DKPixelFormat::RGBA8Uint:            return MTLPixelFormatRGBA8Uint;
            case DKPixelFormat::RGBA8Sint:            return MTLPixelFormatRGBA8Sint;

            case DKPixelFormat::BGRA8Unorm:            return MTLPixelFormatBGRA8Unorm;
            case DKPixelFormat::BGRA8Unorm_sRGB:    return MTLPixelFormatBGRA8Unorm_sRGB;

            case DKPixelFormat::RGB10A2Unorm:    return MTLPixelFormatRGB10A2Unorm;
            case DKPixelFormat::RGB10A2Uint:    return MTLPixelFormatRGB10A2Uint;

            case DKPixelFormat::RG11B10Float:    return MTLPixelFormatRG11B10Float;
            case DKPixelFormat::RGB9E5Float:    return MTLPixelFormatRGB9E5Float;

            case DKPixelFormat::RG32Uint:        return MTLPixelFormatRG32Uint;
            case DKPixelFormat::RG32Sint:        return MTLPixelFormatRG32Sint;
            case DKPixelFormat::RG32Float:        return MTLPixelFormatRG32Float;

            case DKPixelFormat::RGBA16Unorm:    return MTLPixelFormatRGBA16Unorm;
            case DKPixelFormat::RGBA16Snorm:    return MTLPixelFormatRGBA16Snorm;
            case DKPixelFormat::RGBA16Uint:        return MTLPixelFormatRGBA16Uint;
            case DKPixelFormat::RGBA16Sint:        return MTLPixelFormatRGBA16Sint;
            case DKPixelFormat::RGBA16Float:    return MTLPixelFormatRGBA16Float;

            case DKPixelFormat::RGBA32Uint:        return MTLPixelFormatRGBA32Uint;
            case DKPixelFormat::RGBA32Sint:        return MTLPixelFormatRGBA32Sint;
            case DKPixelFormat::RGBA32Float:    return MTLPixelFormatRGBA32Float;

            case DKPixelFormat::D32:            return MTLPixelFormatDepth32Float;

            case DKPixelFormat::D32S8X24:        return MTLPixelFormatDepth32Float_Stencil8;
        }
        return MTLPixelFormatInvalid;
    }

    inline DKPixelFormat PixelFormat(MTLPixelFormat fmt)
    {
        switch (fmt)
        {
            case MTLPixelFormatR8Unorm:                    return DKPixelFormat::R8Unorm;
            case MTLPixelFormatR8Snorm:                    return DKPixelFormat::R8Snorm;
            case MTLPixelFormatR8Uint:                    return DKPixelFormat::R8Uint;
            case MTLPixelFormatR8Sint:                    return DKPixelFormat::R8Sint;
            case MTLPixelFormatR16Unorm:                return DKPixelFormat::R16Unorm;
            case MTLPixelFormatR16Snorm:                return DKPixelFormat::R16Snorm;
            case MTLPixelFormatR16Uint:                    return DKPixelFormat::R16Uint;
            case MTLPixelFormatR16Sint:                    return DKPixelFormat::R16Sint;
            case MTLPixelFormatR16Float:                return DKPixelFormat::R16Float;
            case MTLPixelFormatRG8Unorm:                return DKPixelFormat::RG8Unorm;
            case MTLPixelFormatRG8Snorm:                return DKPixelFormat::RG8Snorm;
            case MTLPixelFormatRG8Uint:                    return DKPixelFormat::RG8Uint;
            case MTLPixelFormatRG8Sint:                    return DKPixelFormat::RG8Sint;
            case MTLPixelFormatR32Uint:                    return DKPixelFormat::R32Uint;
            case MTLPixelFormatR32Sint:                    return DKPixelFormat::R32Sint;
            case MTLPixelFormatR32Float:                return DKPixelFormat::R32Float;
            case MTLPixelFormatRG16Unorm:                return DKPixelFormat::RG16Unorm;
            case MTLPixelFormatRG16Snorm:                return DKPixelFormat::RG16Snorm;
            case MTLPixelFormatRG16Uint:                return DKPixelFormat::RG16Uint;
            case MTLPixelFormatRG16Sint:                return DKPixelFormat::RG16Sint;
            case MTLPixelFormatRG16Float:                return DKPixelFormat::RG16Float;
            case MTLPixelFormatRGBA8Unorm:                return DKPixelFormat::RGBA8Unorm;
            case MTLPixelFormatRGBA8Unorm_sRGB:            return DKPixelFormat::RGBA8Unorm_sRGB;
            case MTLPixelFormatRGBA8Snorm:                return DKPixelFormat::RGBA8Snorm;
            case MTLPixelFormatRGBA8Uint:                return DKPixelFormat::RGBA8Uint;
            case MTLPixelFormatRGBA8Sint:                return DKPixelFormat::RGBA8Sint;
            case MTLPixelFormatBGRA8Unorm:                return DKPixelFormat::RGBA8Unorm;
            case MTLPixelFormatBGRA8Unorm_sRGB:            return DKPixelFormat::RGBA8Unorm_sRGB;
            case MTLPixelFormatRGB10A2Unorm:            return DKPixelFormat::RGB10A2Unorm;
            case MTLPixelFormatRGB10A2Uint:                return DKPixelFormat::RGB10A2Uint;
            case MTLPixelFormatRG11B10Float:            return DKPixelFormat::RG11B10Float;
            case MTLPixelFormatRGB9E5Float:                return DKPixelFormat::RGB9E5Float;
            case MTLPixelFormatRG32Uint:                return DKPixelFormat::RG32Uint;
            case MTLPixelFormatRG32Sint:                return DKPixelFormat::RG32Sint;
            case MTLPixelFormatRG32Float:                return DKPixelFormat::RG32Float;
            case MTLPixelFormatRGBA16Unorm:                return DKPixelFormat::RGBA16Unorm;
            case MTLPixelFormatRGBA16Snorm:                return DKPixelFormat::RGBA16Snorm;
            case MTLPixelFormatRGBA16Uint:                return DKPixelFormat::RGBA16Uint;
            case MTLPixelFormatRGBA16Sint:                return DKPixelFormat::RGBA16Sint;
            case MTLPixelFormatRGBA16Float:                return DKPixelFormat::RGBA16Float;
            case MTLPixelFormatRGBA32Uint:                return DKPixelFormat::RGBA32Uint;
            case MTLPixelFormatRGBA32Sint:                return DKPixelFormat::RGBA32Sint;
            case MTLPixelFormatRGBA32Float:                return DKPixelFormat::RGBA32Float;
            case MTLPixelFormatDepth32Float:            return DKPixelFormat::D32;
            case MTLPixelFormatDepth32Float_Stencil8:    return DKPixelFormat::D32S8X24;
        }
        return DKPixelFormat::Invalid;
    }
}

#endif //#if DKGL_ENABLE_METAL
