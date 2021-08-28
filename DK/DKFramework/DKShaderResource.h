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
    struct DKShaderDataTypeSize
    {
        uint32_t width;
        uint32_t rows;
        uint32_t columns;

        uint32_t Bytes() const noexcept { return width * rows * columns; }

        constexpr DKShaderDataTypeSize(uint32_t w, uint32_t r, uint32_t c)
            : width(w), rows(r), columns(c) {}

        constexpr DKShaderDataTypeSize(DKShaderDataType type = DKShaderDataType::None)
            : DKShaderDataTypeSize([type]() constexpr -> DKShaderDataTypeSize
        {
            switch (type)
            {
            case DKShaderDataType::None:
            case DKShaderDataType::Struct:
            case DKShaderDataType::Texture:
            case DKShaderDataType::Sampler:
                return { 0, 0, 0 };

            case DKShaderDataType::Float:       return { 4, 1, 1 };
            case DKShaderDataType::Float2:      return { 4, 2, 1 };
            case DKShaderDataType::Float3:      return { 4, 3, 1 };
            case DKShaderDataType::Float4:      return { 4, 4, 1 };

            case DKShaderDataType::Float2x2:    return { 4, 2, 2 };
            case DKShaderDataType::Float2x3:    return { 4, 2, 3 };
            case DKShaderDataType::Float2x4:    return { 4, 2, 4 };

            case DKShaderDataType::Float3x2:    return { 4, 3, 2 };
            case DKShaderDataType::Float3x3:    return { 4, 3, 3 };
            case DKShaderDataType::Float3x4:    return { 4, 3, 4 };

            case DKShaderDataType::Float4x2:    return { 4, 4, 2 };
            case DKShaderDataType::Float4x3:    return { 4, 4, 3 };
            case DKShaderDataType::Float4x4:    return { 4, 4, 4 };

            case DKShaderDataType::Half:        return { 2, 1, 1 };
            case DKShaderDataType::Half2:       return { 2, 2, 1 };
            case DKShaderDataType::Half3:       return { 2, 3, 1 };
            case DKShaderDataType::Half4:       return { 2, 4, 1 };

            case DKShaderDataType::Half2x2:     return { 2, 2, 2 };
            case DKShaderDataType::Half2x3:     return { 2, 2, 3 };
            case DKShaderDataType::Half2x4:     return { 2, 2, 4 };

            case DKShaderDataType::Half3x2:     return { 2, 3, 2 };
            case DKShaderDataType::Half3x3:     return { 2, 3, 3 };
            case DKShaderDataType::Half3x4:     return { 2, 3, 4 };

            case DKShaderDataType::Half4x2:     return { 2, 4, 2 };
            case DKShaderDataType::Half4x3:     return { 2, 4, 3 };
            case DKShaderDataType::Half4x4:     return { 2, 4, 4 };

            case DKShaderDataType::Int:         return { 4, 1, 1 };
            case DKShaderDataType::Int2:        return { 4, 2, 1 };
            case DKShaderDataType::Int3:        return { 4, 3, 1 };
            case DKShaderDataType::Int4:        return { 4, 4, 1 };

            case DKShaderDataType::UInt:        return { 4, 1, 1 };
            case DKShaderDataType::UInt2:       return { 4, 2, 1 };
            case DKShaderDataType::UInt3:       return { 4, 3, 1 };
            case DKShaderDataType::UInt4:       return { 4, 4, 1 };

            case DKShaderDataType::Short:       return { 2, 1, 1 };
            case DKShaderDataType::Short2:      return { 2, 2, 1 };
            case DKShaderDataType::Short3:      return { 2, 3, 1 };
            case DKShaderDataType::Short4:      return { 2, 4, 1 };

            case DKShaderDataType::UShort:      return { 2, 1, 1 };
            case DKShaderDataType::UShort2:     return { 2, 2, 1 };
            case DKShaderDataType::UShort3:     return { 2, 3, 1 };
            case DKShaderDataType::UShort4:     return { 2, 4, 1 };

            case DKShaderDataType::Char:        return { 1, 1, 1 };
            case DKShaderDataType::Char2:       return { 1, 2, 1 };
            case DKShaderDataType::Char3:       return { 1, 3, 1 };
            case DKShaderDataType::Char4:       return { 1, 4, 1 };

            case DKShaderDataType::UChar:       return { 1, 1, 1 };
            case DKShaderDataType::UChar2:      return { 1, 2, 1 };
            case DKShaderDataType::UChar3:      return { 1, 3, 1 };
            case DKShaderDataType::UChar4:      return { 1, 4, 1 };

            case DKShaderDataType::Bool:        return { 1, 1, 1 };
            case DKShaderDataType::Bool2:       return { 1, 2, 1 };
            case DKShaderDataType::Bool3:       return { 1, 3, 1 };
            case DKShaderDataType::Bool4:       return { 1, 4, 1 };

            default:
                DKASSERT_DESC_DEBUG(0, "Unknown data type");
                break;
            }
            return { 0, 0, 0 };
        }()) {}
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
		uint32_t alignment;
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
			TypeTextureSampler, // texture and sampler (combined)
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

        // struct members (struct only)
        DKArray<DKShaderResourceStructMember> members;
	};
    /**
    Shader reflection data for push constants.
    */
	struct DKShaderPushConstantLayout
	{
		DKString name;
		uint32_t offset;
		uint32_t size;
		uint32_t stages;
		DKArray<DKShaderResourceStructMember> members;
	};
}
