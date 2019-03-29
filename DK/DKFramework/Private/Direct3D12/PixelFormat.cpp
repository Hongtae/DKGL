//
//  File: PixelFormat.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "PixelFormat.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

DXGI_FORMAT PixelFormat::From(DKPixelFormat fmt)
{
	switch (fmt)
	{
	case DKPixelFormat::R8Unorm:			return DXGI_FORMAT_R8_UNORM;
	case DKPixelFormat::R8Snorm:			return DXGI_FORMAT_R8_SNORM;
	case DKPixelFormat::R8Uint:				return DXGI_FORMAT_R8_UINT;
	case DKPixelFormat::R8Sint:				return DXGI_FORMAT_R8_SINT;

	case DKPixelFormat::R16Unorm:			return DXGI_FORMAT_R16_UNORM;
	case DKPixelFormat::R16Snorm:			return DXGI_FORMAT_R16_SNORM;
	case DKPixelFormat::R16Uint:			return DXGI_FORMAT_R16_UINT;
	case DKPixelFormat::R16Sint:			return DXGI_FORMAT_R16_SINT;
	case DKPixelFormat::R16Float:			return DXGI_FORMAT_R16_FLOAT;

	case DKPixelFormat::RG8Unorm:			return DXGI_FORMAT_R8G8_UNORM;
	case DKPixelFormat::RG8Snorm:			return DXGI_FORMAT_R8G8_SNORM;
	case DKPixelFormat::RG8Uint:			return DXGI_FORMAT_R8G8_UINT;
	case DKPixelFormat::RG8Sint:			return DXGI_FORMAT_R8G8_SINT;

	case DKPixelFormat::R32Uint:			return DXGI_FORMAT_R32_UINT;
	case DKPixelFormat::R32Sint:			return DXGI_FORMAT_R32_SINT;
	case DKPixelFormat::R32Float:			return DXGI_FORMAT_R32_FLOAT;

	case DKPixelFormat::RG16Unorm:			return DXGI_FORMAT_R16G16_UNORM;
	case DKPixelFormat::RG16Snorm:			return DXGI_FORMAT_R16G16_SNORM;
	case DKPixelFormat::RG16Uint:			return DXGI_FORMAT_R16G16_UINT;
	case DKPixelFormat::RG16Sint:			return DXGI_FORMAT_R16G16_SINT;
	case DKPixelFormat::RG16Float:			return DXGI_FORMAT_R16G16_FLOAT;

	case DKPixelFormat::RGBA8Unorm:			return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DKPixelFormat::RGBA8Unorm_sRGB:	return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case DKPixelFormat::RGBA8Snorm:			return DXGI_FORMAT_R8G8B8A8_SNORM;
	case DKPixelFormat::RGBA8Uint:			return DXGI_FORMAT_R8G8B8A8_UINT;
	case DKPixelFormat::RGBA8Sint:			return DXGI_FORMAT_R8G8B8A8_SINT;

	case DKPixelFormat::BGRA8Unorm:			return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DKPixelFormat::BGRA8Unorm_sRGB:	return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

	case DKPixelFormat::RGB10A2Unorm:		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case DKPixelFormat::RGB10A2Uint:		return DXGI_FORMAT_R10G10B10A2_UINT;

	case DKPixelFormat::RG11B10Float:		return DXGI_FORMAT_R11G11B10_FLOAT;
	case DKPixelFormat::RGB9E5Float:		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

	case DKPixelFormat::RG32Uint:			return DXGI_FORMAT_R32G32_UINT;
	case DKPixelFormat::RG32Sint:			return DXGI_FORMAT_R32G32_SINT;
	case DKPixelFormat::RG32Float:			return DXGI_FORMAT_R32G32_FLOAT;

	case DKPixelFormat::RGBA16Unorm:		return DXGI_FORMAT_R16G16B16A16_UNORM;
	case DKPixelFormat::RGBA16Snorm:		return DXGI_FORMAT_R16G16B16A16_SNORM;
	case DKPixelFormat::RGBA16Uint:			return DXGI_FORMAT_R16G16B16A16_UINT;
	case DKPixelFormat::RGBA16Sint:			return DXGI_FORMAT_R16G16B16A16_SINT;
	case DKPixelFormat::RGBA16Float:		return DXGI_FORMAT_R16G16B16A16_FLOAT;

	case DKPixelFormat::RGBA32Uint:			return DXGI_FORMAT_R32G32B32A32_UINT;
	case DKPixelFormat::RGBA32Sint:			return DXGI_FORMAT_R32G32B32A32_SINT;
	case DKPixelFormat::RGBA32Float:		return DXGI_FORMAT_R32G32B32A32_FLOAT;

	case DKPixelFormat::D32:				return DXGI_FORMAT_D32_FLOAT;

	case DKPixelFormat::D32S8X24:			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DKPixelFormat PixelFormat::To(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R8_UNORM:				return DKPixelFormat::R8Unorm;
	case DXGI_FORMAT_R8_SNORM:				return DKPixelFormat::R8Snorm;
	case DXGI_FORMAT_R8_UINT:				return DKPixelFormat::R8Uint;
	case DXGI_FORMAT_R8_SINT:				return DKPixelFormat::R8Sint;

	case DXGI_FORMAT_R16_UNORM:				return DKPixelFormat::R16Unorm;
	case DXGI_FORMAT_R16_SNORM:				return DKPixelFormat::R16Snorm;
	case DXGI_FORMAT_R16_UINT:				return DKPixelFormat::R16Uint;
	case DXGI_FORMAT_R16_SINT:				return DKPixelFormat::R16Sint;
	case DXGI_FORMAT_R16_FLOAT:				return DKPixelFormat::R16Float;

	case DXGI_FORMAT_R8G8_UNORM:			return DKPixelFormat::RG8Unorm;
	case DXGI_FORMAT_R8G8_SNORM:			return DKPixelFormat::RG8Snorm;
	case DXGI_FORMAT_R8G8_UINT:				return DKPixelFormat::RG8Uint;
	case DXGI_FORMAT_R8G8_SINT:				return DKPixelFormat::RG8Sint;

	case DXGI_FORMAT_R32_UINT:				return DKPixelFormat::R32Uint;
	case DXGI_FORMAT_R32_SINT:				return DKPixelFormat::R32Sint;
	case DXGI_FORMAT_R32_FLOAT:				return DKPixelFormat::R32Float;

	case DXGI_FORMAT_R16G16_UNORM:			return DKPixelFormat::RG16Unorm;
	case DXGI_FORMAT_R16G16_SNORM:			return DKPixelFormat::RG16Snorm;
	case DXGI_FORMAT_R16G16_UINT:			return DKPixelFormat::RG16Uint;
	case DXGI_FORMAT_R16G16_SINT:			return DKPixelFormat::RG16Sint;
	case DXGI_FORMAT_R16G16_FLOAT:			return DKPixelFormat::RG16Float;

	case DXGI_FORMAT_R8G8B8A8_UNORM:		return DKPixelFormat::RGBA8Unorm;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:	return DKPixelFormat::RGBA8Unorm_sRGB;
	case DXGI_FORMAT_R8G8B8A8_SNORM:		return DKPixelFormat::RGBA8Snorm;
	case DXGI_FORMAT_R8G8B8A8_UINT:			return DKPixelFormat::RGBA8Uint;
	case DXGI_FORMAT_R8G8B8A8_SINT:			return DKPixelFormat::RGBA8Sint;

	case DXGI_FORMAT_B8G8R8A8_UNORM:		return DKPixelFormat::BGRA8Unorm;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:	return DKPixelFormat::BGRA8Unorm_sRGB;

	case DXGI_FORMAT_R10G10B10A2_UNORM:		return DKPixelFormat::RGB10A2Unorm;
	case DXGI_FORMAT_R10G10B10A2_UINT:		return DKPixelFormat::RGB10A2Uint;

	case DXGI_FORMAT_R11G11B10_FLOAT:		return DKPixelFormat::RG11B10Float;
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:	return DKPixelFormat::RGB9E5Float;

	case DXGI_FORMAT_R32G32_UINT:			return DKPixelFormat::RG32Uint;
	case DXGI_FORMAT_R32G32_SINT:			return DKPixelFormat::RG32Sint;
	case DXGI_FORMAT_R32G32_FLOAT:			return DKPixelFormat::RG32Float;

	case DXGI_FORMAT_R16G16B16A16_UNORM:	return DKPixelFormat::RGBA16Unorm;
	case DXGI_FORMAT_R16G16B16A16_SNORM:	return DKPixelFormat::RGBA16Snorm;
	case DXGI_FORMAT_R16G16B16A16_UINT:		return DKPixelFormat::RGBA16Uint;
	case DXGI_FORMAT_R16G16B16A16_SINT:		return DKPixelFormat::RGBA16Sint;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:	return DKPixelFormat::RGBA16Float;

	case DXGI_FORMAT_R32G32B32A32_UINT:		return DKPixelFormat::RGBA32Uint;
	case DXGI_FORMAT_R32G32B32A32_SINT:		return DKPixelFormat::RGBA32Sint;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:	return DKPixelFormat::RGBA32Float;

	case DXGI_FORMAT_D32_FLOAT:				return DKPixelFormat::D32;

	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:	return DKPixelFormat::D32S8X24;

	}
	return DKPixelFormat::Invalid;
}

#endif //#if DKGL_ENABLE_VULKAN
