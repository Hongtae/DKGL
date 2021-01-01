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
    struct DKVertexFormatInfo
    {
        uint32_t typeSize;
        uint32_t components;
        bool normalized;

        size_t Bytes() const noexcept { return typeSize * components; }

        constexpr DKVertexFormatInfo(uint32_t s, uint32_t c=1, bool n=false)
            : typeSize(s), components(c), normalized(n) {}

        constexpr DKVertexFormatInfo(DKVertexFormat format)
            : DKVertexFormatInfo([format]() constexpr -> DKVertexFormatInfo
            {
                switch (format)
                {
                case DKVertexFormat::UChar2:                return { 1, 2 };
                case DKVertexFormat::UChar3:                return { 1, 3 };
                case DKVertexFormat::UChar4:                return { 1, 4 };

                case DKVertexFormat::Char2:                 return { 1, 2 };
                case DKVertexFormat::Char3:                 return { 1, 3 };
                case DKVertexFormat::Char4:                 return { 1, 4 };

                case DKVertexFormat::UChar2Normalized:      return { 1, 2, true };
                case DKVertexFormat::UChar3Normalized:      return { 1, 3, true };
                case DKVertexFormat::UChar4Normalized:      return { 1, 4, true };

                case DKVertexFormat::Char2Normalized:       return { 1, 2, true };
                case DKVertexFormat::Char3Normalized:       return { 1, 3, true };
                case DKVertexFormat::Char4Normalized:       return { 1, 4, true };

                case DKVertexFormat::UShort2:               return { 2, 2 };
                case DKVertexFormat::UShort3:               return { 2, 3 };
                case DKVertexFormat::UShort4:               return { 2, 4 };

                case DKVertexFormat::Short2:                return { 2, 2 };
                case DKVertexFormat::Short3:                return { 2, 3 };
                case DKVertexFormat::Short4:                return { 2, 4 };

                case DKVertexFormat::UShort2Normalized:     return { 2, 2, true };
                case DKVertexFormat::UShort3Normalized:     return { 2, 3, true };
                case DKVertexFormat::UShort4Normalized:     return { 2, 4, true };

                case DKVertexFormat::Short2Normalized:      return { 2, 2, true };
                case DKVertexFormat::Short3Normalized:      return { 2, 3, true };
                case DKVertexFormat::Short4Normalized:      return { 2, 4, true };

                case DKVertexFormat::Half2:                 return { 2, 2 };
                case DKVertexFormat::Half3:                 return { 2, 3 };
                case DKVertexFormat::Half4:                 return { 2, 4 };

                case DKVertexFormat::Float:                 return { 4, 1 };
                case DKVertexFormat::Float2:                return { 4, 2 };
                case DKVertexFormat::Float3:                return { 4, 3 };
                case DKVertexFormat::Float4:                return { 4, 4 };

                case DKVertexFormat::Int:                   return { 4, 1 };
                case DKVertexFormat::Int2:                  return { 4, 2 };
                case DKVertexFormat::Int3:                  return { 4, 3 };
                case DKVertexFormat::Int4:                  return { 4, 4 };

                case DKVertexFormat::UInt:                  return { 4, 1 };
                case DKVertexFormat::UInt2:                 return { 4, 2 };
                case DKVertexFormat::UInt3:                 return { 4, 3 };
                case DKVertexFormat::UInt4:                 return { 4, 4 };

                case DKVertexFormat::Int1010102Normalized:  return { 4, 1, true };
                case DKVertexFormat::UInt1010102Normalized: return { 4, 1, true };

                default:
                    DKASSERT_DESC_DEBUG(0, "Unknown format");
                    break;
                }
                return { 0, 0 };
        }()) {}
    };

	enum class DKVertexStepRate
	{
		Vertex = 0,
		Instance,
	};

	struct DKVertexBufferLayoutDescriptor
	{
		DKVertexStepRate step;
		uint32_t stride;
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
