//
//  File: PixelFormat.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "PixelFormat.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;


MTLPixelFormat PixelFormat::From(DKPixelFormat fmt)
{
	switch (fmt)
	{
	case DKPixelFormat::R8Unorm:		return MTLPixelFormatR8Unorm;
	case DKPixelFormat::R8Snorm:		return MTLPixelFormatR8Snorm;
	case DKPixelFormat::R8Uint:			return MTLPixelFormatR8Uint;
	case DKPixelFormat::R8Sint:			return MTLPixelFormatR8Sint;

	case DKPixelFormat::R16Unorm:		return MTLPixelFormatR16Unorm;
	case DKPixelFormat::R16Snorm:		return MTLPixelFormatR16Snorm;
	case DKPixelFormat::R16Uint:		return MTLPixelFormatR16Uint;
	case DKPixelFormat::R16Sint:		return MTLPixelFormatR16Sint;
	case DKPixelFormat::R16Float:		return MTLPixelFormatR16Float;

	case DKPixelFormat::RG8Unorm:		return MTLPixelFormatRG8Unorm;
	case DKPixelFormat::RG8Snorm:		return MTLPixelFormatRG8Snorm;
	case DKPixelFormat::RG8Uint:		return MTLPixelFormatRG8Uint;
	case DKPixelFormat::RG8Sint:		return MTLPixelFormatRG8Sint;

	case DKPixelFormat::R32Uint:		return MTLPixelFormatR32Uint;
	case DKPixelFormat::R32Sint:		return MTLPixelFormatR32Sint;
	case DKPixelFormat::R32Float:		return MTLPixelFormatR32Float;

	case DKPixelFormat::RG16Unorm:		return MTLPixelFormatRG16Unorm;
	case DKPixelFormat::RG16Snorm:		return MTLPixelFormatRG16Snorm;
	case DKPixelFormat::RG16Uint:		return MTLPixelFormatRG16Uint;
	case DKPixelFormat::RG16Sint:		return MTLPixelFormatRG16Sint;
	case DKPixelFormat::RG16Float:		return MTLPixelFormatRG16Float;

	case DKPixelFormat::RGBA8Unorm:			return MTLPixelFormatRGBA8Unorm;
	case DKPixelFormat::RGBA8Unorm_sRGB:	return MTLPixelFormatRGBA8Unorm_sRGB;
	case DKPixelFormat::RGBA8Snorm:			return MTLPixelFormatRGBA8Snorm;
	case DKPixelFormat::RGBA8Uint:			return MTLPixelFormatRGBA8Uint;
	case DKPixelFormat::RGBA8Sint:			return MTLPixelFormatRGBA8Sint;

	case DKPixelFormat::BGRA8Unorm:			return MTLPixelFormatBGRA8Unorm;
	case DKPixelFormat::BGRA8Unorm_sRGB:	return MTLPixelFormatBGRA8Unorm_sRGB;

	case DKPixelFormat::RGB10A2Unorm:	return MTLPixelFormatRGB10A2Unorm;
	case DKPixelFormat::RGB10A2Uint:	return MTLPixelFormatRGB10A2Uint;

	case DKPixelFormat::RG11B10Float:	return MTLPixelFormatRG11B10Float;
	case DKPixelFormat::RGB9E5Float:	return MTLPixelFormatRGB9E5Float;

	case DKPixelFormat::RG32Uint:		return MTLPixelFormatRG32Uint;
	case DKPixelFormat::RG32Sint:		return MTLPixelFormatRG32Sint;
	case DKPixelFormat::RG32Float:		return MTLPixelFormatRG32Float;

	case DKPixelFormat::RGBA16Unorm:	return MTLPixelFormatRGBA16Unorm;
	case DKPixelFormat::RGBA16Snorm:	return MTLPixelFormatRGBA16Snorm;
	case DKPixelFormat::RGBA16Uint:		return MTLPixelFormatRGBA16Uint;
	case DKPixelFormat::RGBA16Sint:		return MTLPixelFormatRGBA16Sint;
	case DKPixelFormat::RGBA16Float:	return MTLPixelFormatRGBA16Float;

	case DKPixelFormat::RGBA32Uint:		return MTLPixelFormatRGBA32Uint;
	case DKPixelFormat::RGBA32Sint:		return MTLPixelFormatRGBA32Sint;
	case DKPixelFormat::RGBA32Float:	return MTLPixelFormatRGBA32Float;

	case DKPixelFormat::D32:			return MTLPixelFormatDepth32Float;

	case DKPixelFormat::D32S8X24:		return MTLPixelFormatDepth32Float_Stencil8;
	}
	return MTLPixelFormatInvalid;
}

DKPixelFormat PixelFormat::To(MTLPixelFormat fmt)
{
	switch (fmt)
	{
	case MTLPixelFormatR8Unorm:					return DKPixelFormat::R8Unorm;
	case MTLPixelFormatR8Snorm:					return DKPixelFormat::R8Snorm;
	case MTLPixelFormatR8Uint:					return DKPixelFormat::R8Uint;
	case MTLPixelFormatR8Sint:					return DKPixelFormat::R8Sint;
	case MTLPixelFormatR16Unorm:				return DKPixelFormat::R16Unorm;
	case MTLPixelFormatR16Snorm:				return DKPixelFormat::R16Snorm;
	case MTLPixelFormatR16Uint:					return DKPixelFormat::R16Uint;
	case MTLPixelFormatR16Sint:					return DKPixelFormat::R16Sint;
	case MTLPixelFormatR16Float:				return DKPixelFormat::R16Float;
	case MTLPixelFormatRG8Unorm:				return DKPixelFormat::RG8Unorm;
	case MTLPixelFormatRG8Snorm:				return DKPixelFormat::RG8Snorm;
	case MTLPixelFormatRG8Uint:					return DKPixelFormat::RG8Uint;
	case MTLPixelFormatRG8Sint:					return DKPixelFormat::RG8Sint;
	case MTLPixelFormatR32Uint:					return DKPixelFormat::R32Uint;
	case MTLPixelFormatR32Sint:					return DKPixelFormat::R32Sint;
	case MTLPixelFormatR32Float:				return DKPixelFormat::R32Float;
	case MTLPixelFormatRG16Unorm:				return DKPixelFormat::RG16Unorm;
	case MTLPixelFormatRG16Snorm:				return DKPixelFormat::RG16Snorm;
	case MTLPixelFormatRG16Uint:				return DKPixelFormat::RG16Uint;
	case MTLPixelFormatRG16Sint:				return DKPixelFormat::RG16Sint;
	case MTLPixelFormatRG16Float:				return DKPixelFormat::RG16Float;
	case MTLPixelFormatRGBA8Unorm:				return DKPixelFormat::RGBA8Unorm;
	case MTLPixelFormatRGBA8Unorm_sRGB:			return DKPixelFormat::RGBA8Unorm_sRGB;
	case MTLPixelFormatRGBA8Snorm:				return DKPixelFormat::RGBA8Snorm;
	case MTLPixelFormatRGBA8Uint:				return DKPixelFormat::RGBA8Uint;
	case MTLPixelFormatRGBA8Sint:				return DKPixelFormat::RGBA8Sint;
	case MTLPixelFormatBGRA8Unorm:				return DKPixelFormat::RGBA8Unorm;
	case MTLPixelFormatBGRA8Unorm_sRGB:			return DKPixelFormat::RGBA8Unorm_sRGB;
	case MTLPixelFormatRGB10A2Unorm:			return DKPixelFormat::RGB10A2Unorm;
	case MTLPixelFormatRGB10A2Uint:				return DKPixelFormat::RGB10A2Uint;
	case MTLPixelFormatRG11B10Float:			return DKPixelFormat::RG11B10Float;
	case MTLPixelFormatRGB9E5Float:				return DKPixelFormat::RGB9E5Float;
	case MTLPixelFormatRG32Uint:				return DKPixelFormat::RG32Uint;
	case MTLPixelFormatRG32Sint:				return DKPixelFormat::RG32Sint;
	case MTLPixelFormatRG32Float:				return DKPixelFormat::RG32Float;
	case MTLPixelFormatRGBA16Unorm:				return DKPixelFormat::RGBA16Unorm;
	case MTLPixelFormatRGBA16Snorm:				return DKPixelFormat::RGBA16Snorm;
	case MTLPixelFormatRGBA16Uint:				return DKPixelFormat::RGBA16Uint;
	case MTLPixelFormatRGBA16Sint:				return DKPixelFormat::RGBA16Sint;
	case MTLPixelFormatRGBA16Float:				return DKPixelFormat::RGBA16Float;
	case MTLPixelFormatRGBA32Uint:				return DKPixelFormat::RGBA32Uint;
	case MTLPixelFormatRGBA32Sint:				return DKPixelFormat::RGBA32Sint;
	case MTLPixelFormatRGBA32Float:				return DKPixelFormat::RGBA32Float;
	case MTLPixelFormatDepth32Float:			return DKPixelFormat::D32;
	case MTLPixelFormatDepth32Float_Stencil8:	return DKPixelFormat::D32S8X24;
	}
	return DKPixelFormat::Invalid;
}

#endif //#if DKGL_ENABLE_METAL

