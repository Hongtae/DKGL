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
		D32,

		// Depth Stencil
		D32S8X24,
	};
}
