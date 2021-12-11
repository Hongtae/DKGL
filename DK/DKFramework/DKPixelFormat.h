//
//  File: DKPixelFormat.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	enum class DKPixelFormat
	{
		Invalid = 0,
		
		// 8 bit formats
		R8Unorm,
		R8Snorm,
		R8Uint,
		R8Sint,

		// 16 bit formats
		R16Unorm,
		R16Snorm,
		R16Uint,
		R16Sint,
		R16Float,

		RG8Unorm,
		RG8Snorm,
		RG8Uint,
		RG8Sint,

		// 32 bit formats
		R32Uint,
		R32Sint,
		R32Float,

		RG16Unorm,
		RG16Snorm,
		RG16Uint,
		RG16Sint,
		RG16Float,

		RGBA8Unorm,
		RGBA8Unorm_sRGB,
		RGBA8Snorm,
		RGBA8Uint,
		RGBA8Sint,

		BGRA8Unorm,
		BGRA8Unorm_sRGB,

		// packed 32 bit formats
		RGB10A2Unorm,
		RGB10A2Uint,

		RG11B10Float,
		RGB9E5Float,

		// 64 bit formats
		RG32Uint,
		RG32Sint,
		RG32Float,

		RGBA16Unorm,
		RGBA16Snorm,
		RGBA16Uint,
		RGBA16Sint,
		RGBA16Float,

		// 128 bit formats
		RGBA32Uint,
		RGBA32Sint,
		RGBA32Float,

		// Depth
		D32Float,

        // Stencil (Uint)
        S8,

		// Depth Stencil
		D32FloatS8, // 32-depth, 8-stencil, 24-unused.
	};

	constexpr bool DKPixelFormatIsColorFormat(DKPixelFormat pf)
	{
		return pf > DKPixelFormat::Invalid &&
			pf < DKPixelFormat::D32Float;
	}
	constexpr bool DKPixelFormatIsDepthFormat(DKPixelFormat pf)
	{
        switch (pf)
        {
        case DKPixelFormat::D32Float:
        case DKPixelFormat::D32FloatS8:
            return true;
        }
        return false;
	}
    constexpr bool DKPixelFormatIsStencilFormat(DKPixelFormat pf)
    {
        switch (pf)
        {
        case DKPixelFormat::S8:
        case DKPixelFormat::D32FloatS8:
            return true;
        }
        return false;
    }
    constexpr int32_t DKPixelFormatBytesPerPixel(DKPixelFormat pf)
    {
        switch (pf)
        {
            // 8 bit formats
        case DKPixelFormat::R8Unorm:
        case DKPixelFormat::R8Snorm:
        case DKPixelFormat::R8Uint:
        case DKPixelFormat::R8Sint:
            return 1;
            break;
            // 16 bit formats
        case DKPixelFormat::R16Unorm:
        case DKPixelFormat::R16Snorm:
        case DKPixelFormat::R16Uint:
        case DKPixelFormat::R16Sint:
        case DKPixelFormat::R16Float:

        case DKPixelFormat::RG8Unorm:
        case DKPixelFormat::RG8Snorm:
        case DKPixelFormat::RG8Uint:
        case DKPixelFormat::RG8Sint:
            return 2;
            break;

            // 32 bit formats
        case DKPixelFormat::R32Uint:
        case DKPixelFormat::R32Sint:
        case DKPixelFormat::R32Float:

        case DKPixelFormat::RG16Unorm:
        case DKPixelFormat::RG16Snorm:
        case DKPixelFormat::RG16Uint:
        case DKPixelFormat::RG16Sint:
        case DKPixelFormat::RG16Float:

        case DKPixelFormat::RGBA8Unorm:
        case DKPixelFormat::RGBA8Unorm_sRGB:
        case DKPixelFormat::RGBA8Snorm:
        case DKPixelFormat::RGBA8Uint:
        case DKPixelFormat::RGBA8Sint:

        case DKPixelFormat::BGRA8Unorm:
        case DKPixelFormat::BGRA8Unorm_sRGB:


            // packed 32 bit formats
        case DKPixelFormat::RGB10A2Unorm:
        case DKPixelFormat::RGB10A2Uint:

        case DKPixelFormat::RG11B10Float:
        case DKPixelFormat::RGB9E5Float:
            return 4;
            break;

            // 64 bit formats
        case DKPixelFormat::RG32Uint:
        case DKPixelFormat::RG32Sint:
        case DKPixelFormat::RG32Float:

        case DKPixelFormat::RGBA16Unorm:
        case DKPixelFormat::RGBA16Snorm:
        case DKPixelFormat::RGBA16Uint:
        case DKPixelFormat::RGBA16Sint:
        case DKPixelFormat::RGBA16Float:
            return 8;
            break;

            // 128 bit formats
        case DKPixelFormat::RGBA32Uint:
        case DKPixelFormat::RGBA32Sint:
        case DKPixelFormat::RGBA32Float:
            return 16;
            break;
            // Depth
        case DKPixelFormat::D32Float:
            return 4;
            break;
            // Stencil (Uint)
        case DKPixelFormat::S8:
            return 1;
            break;

            // Depth Stencil
        case DKPixelFormat::D32FloatS8: // 32-depth: 8-stencil: 24-unused.
            return 8;
            break;
        }
        return 0; // unsupported pixel format!
    }
}
