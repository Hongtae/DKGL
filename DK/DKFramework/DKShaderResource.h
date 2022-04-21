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

        Bool,
        BoolV2,
        BoolV3,
        BoolV4,

        Int8,
        Int8V2,
        Int8V3,
        Int8V4,

        UInt8,
        UInt8V2,
        UInt8V3,
        UInt8V4,

        Int16,
        Int16V2,
        Int16V3,
        Int16V4,

        UInt16,
        UInt16V2,
        UInt16V3,
        UInt16V4,

        Int32,
        Int32V2,
        Int32V3,
        Int32V4,

        UInt32,
        UInt32V2,
        UInt32V3,
        UInt32V4,

        Int64,
        Int64V2,
        Int64V3,
        Int64V4,

        UInt64,
        UInt64V2,
        UInt64V3,
        UInt64V4,

        Float16,
        Float16V2,
        Float16V3,
        Float16V4,
        Float16M2x2,
        Float16M3x2,
        Float16M4x2,
        Float16M2x3,
        Float16M3x3,
        Float16M4x3,
        Float16M2x4,
        Float16M3x4,
        Float16M4x4,

        Float32,
        Float32V2,
        Float32V3,
        Float32V4,
        Float32M2x2,
        Float32M3x2,
        Float32M4x2,
        Float32M2x3,
        Float32M3x3,
        Float32M4x3,
        Float32M2x4,
        Float32M3x4,
        Float32M4x4,

        Float64,
        Float64V2,
        Float64V3,
        Float64V4,
        Float64M2x2,
        Float64M3x2,
        Float64M4x2,
        Float64M2x3,
        Float64M3x3,
        Float64M4x3,
        Float64M2x4,
        Float64M3x4,
        Float64M4x4,
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

            case DKShaderDataType::Bool:        return { 1, 1, 1 };
            case DKShaderDataType::BoolV2:      return { 1, 2, 1 };
            case DKShaderDataType::BoolV3:      return { 1, 3, 1 };
            case DKShaderDataType::BoolV4:      return { 1, 4, 1 };

            case DKShaderDataType::Int8:        return { 1, 1, 1 };
            case DKShaderDataType::Int8V2:      return { 1, 2, 1 };
            case DKShaderDataType::Int8V3:      return { 1, 3, 1 };
            case DKShaderDataType::Int8V4:      return { 1, 4, 1 };

            case DKShaderDataType::UInt8:       return { 1, 1, 1 };
            case DKShaderDataType::UInt8V2:     return { 1, 2, 1 };
            case DKShaderDataType::UInt8V3:     return { 1, 3, 1 };
            case DKShaderDataType::UInt8V4:     return { 1, 4, 1 };

            case DKShaderDataType::Int16:       return { 2, 1, 1 };
            case DKShaderDataType::Int16V2:     return { 2, 2, 1 };
            case DKShaderDataType::Int16V3:     return { 2, 3, 1 };
            case DKShaderDataType::Int16V4:     return { 2, 4, 1 };

            case DKShaderDataType::UInt16:      return { 2, 1, 1 };
            case DKShaderDataType::UInt16V2:    return { 2, 2, 1 };
            case DKShaderDataType::UInt16V3:    return { 2, 3, 1 };
            case DKShaderDataType::UInt16V4:    return { 2, 4, 1 };

            case DKShaderDataType::Int32:       return { 4, 1, 1 };
            case DKShaderDataType::Int32V2:     return { 4, 2, 1 };
            case DKShaderDataType::Int32V3:     return { 4, 3, 1 };
            case DKShaderDataType::Int32V4:     return { 4, 4, 1 };

            case DKShaderDataType::UInt32:      return { 4, 1, 1 };
            case DKShaderDataType::UInt32V2:    return { 4, 2, 1 };
            case DKShaderDataType::UInt32V3:    return { 4, 3, 1 };
            case DKShaderDataType::UInt32V4:    return { 4, 4, 1 };

            case DKShaderDataType::Int64:       return { 8, 1, 1 };
            case DKShaderDataType::Int64V2:     return { 8, 2, 1 };
            case DKShaderDataType::Int64V3:     return { 8, 3, 1 };
            case DKShaderDataType::Int64V4:     return { 8, 4, 1 };

            case DKShaderDataType::UInt64:      return { 8, 1, 1 };
            case DKShaderDataType::UInt64V2:    return { 8, 2, 1 };
            case DKShaderDataType::UInt64V3:    return { 8, 3, 1 };
            case DKShaderDataType::UInt64V4:    return { 8, 4, 1 };

            case DKShaderDataType::Float16:     return { 2, 1, 1 };
            case DKShaderDataType::Float16V2:   return { 2, 2, 1 };
            case DKShaderDataType::Float16V3:   return { 2, 3, 1 };
            case DKShaderDataType::Float16V4:   return { 2, 4, 1 };
            case DKShaderDataType::Float16M2x2: return { 2, 2, 2 };
            case DKShaderDataType::Float16M3x2: return { 2, 3, 2 };
            case DKShaderDataType::Float16M4x2: return { 2, 4, 2 };
            case DKShaderDataType::Float16M2x3: return { 2, 2, 3 };
            case DKShaderDataType::Float16M3x3: return { 2, 3, 3 };
            case DKShaderDataType::Float16M4x3: return { 2, 4, 3 };
            case DKShaderDataType::Float16M2x4: return { 2, 2, 4 };
            case DKShaderDataType::Float16M3x4: return { 2, 3, 4 };
            case DKShaderDataType::Float16M4x4: return { 2, 4, 4 };

            case DKShaderDataType::Float32:     return { 4, 1, 1 };
            case DKShaderDataType::Float32V2:   return { 4, 2, 1 };
            case DKShaderDataType::Float32V3:   return { 4, 3, 1 };
            case DKShaderDataType::Float32V4:   return { 4, 4, 1 };
            case DKShaderDataType::Float32M2x2: return { 4, 2, 2 };
            case DKShaderDataType::Float32M3x2: return { 4, 3, 2 };
            case DKShaderDataType::Float32M4x2: return { 4, 4, 2 };
            case DKShaderDataType::Float32M2x3: return { 4, 2, 3 };
            case DKShaderDataType::Float32M3x3: return { 4, 3, 3 };
            case DKShaderDataType::Float32M4x3: return { 4, 4, 3 };
            case DKShaderDataType::Float32M2x4: return { 4, 2, 4 };
            case DKShaderDataType::Float32M3x4: return { 4, 3, 4 };
            case DKShaderDataType::Float32M4x4: return { 4, 4, 4 };

            case DKShaderDataType::Float64:     return { 8, 1, 1 };
            case DKShaderDataType::Float64V2:   return { 8, 2, 1 };
            case DKShaderDataType::Float64V3:   return { 8, 3, 1 };
            case DKShaderDataType::Float64V4:   return { 8, 4, 1 };
            case DKShaderDataType::Float64M2x2: return { 8, 2, 2 };
            case DKShaderDataType::Float64M3x2: return { 8, 3, 2 };
            case DKShaderDataType::Float64M4x2: return { 8, 4, 2 };
            case DKShaderDataType::Float64M2x3: return { 8, 2, 3 };
            case DKShaderDataType::Float64M3x3: return { 8, 3, 3 };
            case DKShaderDataType::Float64M4x3: return { 8, 4, 3 };
            case DKShaderDataType::Float64M2x4: return { 8, 2, 4 };
            case DKShaderDataType::Float64M3x4: return { 8, 3, 4 };
            case DKShaderDataType::Float64M4x4: return { 8, 4, 4 };

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
        uint32_t size;  // declared size
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
