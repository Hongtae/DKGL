//
//  File: Types.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKShader.h"
#include "../../DKShaderResource.h"
#include "../../DKVertexDescriptor.h"

namespace DKFramework::Private::Vulkan
{
    inline DKShader::DescriptorType DescriptorType(VkDescriptorType type)
    {
        switch (type)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return DKShader::DescriptorTypeSampler;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return DKShader::DescriptorTypeTextureSampler;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return DKShader::DescriptorTypeTexture;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return DKShader::DescriptorTypeStorageTexture;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return DKShader::DescriptorTypeUniformTexelBuffer;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return DKShader::DescriptorTypeStorageTexelBuffer;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return DKShader::DescriptorTypeUniformBuffer;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return DKShader::DescriptorTypeStorageBuffer;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            DKASSERT_DESC_DEBUG(0, "Not implemented yet");
            break;
        default:
            DKLogE("Unknown DescriptorType!!");
            DKASSERT_DESC_DEBUG(0, "Unknown descriptor type!");
        }
        return {};
    }
    inline VkDescriptorType DescriptorType(DKShader::DescriptorType type)
    {
        switch (type)
        {
        case DKShader::DescriptorTypeUniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DKShader::DescriptorTypeStorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DKShader::DescriptorTypeStorageTexture:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DKShader::DescriptorTypeTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DKShader::DescriptorTypeTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DKShader::DescriptorTypeUniformTexelBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case DKShader::DescriptorTypeStorageTexelBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case DKShader::DescriptorTypeSampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        default:
            DKLogE("Unknown DescriptorType!!");
            DKASSERT_DESC_DEBUG(0, "Unknown descriptor type!");
        }
        return {};
    }

    inline VkFormat VertexFormat(DKVertexFormat fmt)
    {
        switch (fmt) {
        case DKVertexFormat::UChar2:				return VK_FORMAT_R8G8_UINT;
        case DKVertexFormat::UChar3: 				return VK_FORMAT_R8G8B8_UINT;
        case DKVertexFormat::UChar4: 				return VK_FORMAT_R8G8B8A8_UINT;
        case DKVertexFormat::Char2: 				return VK_FORMAT_R8G8_SINT;
        case DKVertexFormat::Char3: 				return VK_FORMAT_R8G8B8_SINT;
        case DKVertexFormat::Char4: 				return VK_FORMAT_R8G8B8A8_SINT;
        case DKVertexFormat::UChar2Normalized: 		return VK_FORMAT_R8G8_UNORM;
        case DKVertexFormat::UChar3Normalized: 		return VK_FORMAT_R8G8B8_UNORM;
        case DKVertexFormat::UChar4Normalized: 		return VK_FORMAT_R8G8B8A8_UNORM;
        case DKVertexFormat::Char2Normalized: 		return VK_FORMAT_R8G8_SNORM;
        case DKVertexFormat::Char3Normalized: 		return VK_FORMAT_R8G8B8_SNORM;
        case DKVertexFormat::Char4Normalized: 		return VK_FORMAT_R8G8B8A8_SNORM;
        case DKVertexFormat::UShort2: 				return VK_FORMAT_R16G16_UINT;
        case DKVertexFormat::UShort3: 				return VK_FORMAT_R16G16B16_UINT;
        case DKVertexFormat::UShort4: 				return VK_FORMAT_R16G16B16A16_UINT;
        case DKVertexFormat::Short2: 				return VK_FORMAT_R16G16_SINT;
        case DKVertexFormat::Short3: 				return VK_FORMAT_R16G16B16_SINT;
        case DKVertexFormat::Short4: 				return VK_FORMAT_R16G16B16A16_SINT;
        case DKVertexFormat::UShort2Normalized: 	return VK_FORMAT_R16G16_UNORM;
        case DKVertexFormat::UShort3Normalized: 	return VK_FORMAT_R16G16B16_UNORM;
        case DKVertexFormat::UShort4Normalized: 	return VK_FORMAT_R16G16B16A16_UNORM;
        case DKVertexFormat::Short2Normalized: 		return VK_FORMAT_R16G16_SNORM;
        case DKVertexFormat::Short3Normalized: 		return VK_FORMAT_R16G16B16_SNORM;
        case DKVertexFormat::Short4Normalized: 		return VK_FORMAT_R16G16B16A16_SNORM;
        case DKVertexFormat::Half2: 				return VK_FORMAT_R16G16_SFLOAT;
        case DKVertexFormat::Half3: 				return VK_FORMAT_R16G16B16_SFLOAT;
        case DKVertexFormat::Half4: 				return VK_FORMAT_R16G16B16A16_SFLOAT;
        case DKVertexFormat::Float: 				return VK_FORMAT_R32_SFLOAT;
        case DKVertexFormat::Float2: 				return VK_FORMAT_R32G32_SFLOAT;
        case DKVertexFormat::Float3: 				return VK_FORMAT_R32G32B32_SFLOAT;
        case DKVertexFormat::Float4: 				return VK_FORMAT_R32G32B32A32_SFLOAT;
        case DKVertexFormat::Int: 					return VK_FORMAT_R32_SINT;
        case DKVertexFormat::Int2: 					return VK_FORMAT_R32G32_SINT;
        case DKVertexFormat::Int3: 					return VK_FORMAT_R32G32B32_SINT;
        case DKVertexFormat::Int4: 					return VK_FORMAT_R32G32B32A32_SINT;
        case DKVertexFormat::UInt: 					return VK_FORMAT_R32_UINT;
        case DKVertexFormat::UInt2: 				return VK_FORMAT_R32G32_UINT;
        case DKVertexFormat::UInt3: 				return VK_FORMAT_R32G32B32_UINT;
        case DKVertexFormat::UInt4: 				return VK_FORMAT_R32G32B32A32_UINT;
        case DKVertexFormat::Int1010102Normalized: 	return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
        case DKVertexFormat::UInt1010102Normalized: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        }
        DKASSERT_DESC_DEBUG(0, "Unknown type! (or not implemented yet)");
        return VK_FORMAT_UNDEFINED;
    }
}
#endif
