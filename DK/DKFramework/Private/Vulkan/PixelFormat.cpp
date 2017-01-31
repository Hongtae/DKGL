//
//  File: PixelFormat.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include "PixelFormat.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

VkFormat PixelFormat::From(DKPixelFormat fmt)
{
	switch (fmt)
	{
	case DKPixelFormat::R8Unorm:			return VK_FORMAT_R8_UNORM;
	case DKPixelFormat::R8Snorm:			return VK_FORMAT_R8_SNORM;
	case DKPixelFormat::R8Uint:				return VK_FORMAT_R8_UINT;
	case DKPixelFormat::R8Sint:				return VK_FORMAT_R8_SINT;

	case DKPixelFormat::R16Unorm:			return VK_FORMAT_R16_UNORM;
	case DKPixelFormat::R16Snorm:			return VK_FORMAT_R16_SNORM;
	case DKPixelFormat::R16Uint:			return VK_FORMAT_R16_UINT;
	case DKPixelFormat::R16Sint:			return VK_FORMAT_R16_SINT;
	case DKPixelFormat::R16Float:			return VK_FORMAT_R16_SFLOAT;

	case DKPixelFormat::RG8Unorm:			return VK_FORMAT_R8G8_UNORM;
	case DKPixelFormat::RG8Snorm:			return VK_FORMAT_R8G8_SNORM;
	case DKPixelFormat::RG8Uint:			return VK_FORMAT_R8G8_UINT;
	case DKPixelFormat::RG8Sint:			return VK_FORMAT_R8G8_SINT;

	case DKPixelFormat::R32Uint:			return VK_FORMAT_R32_UINT;
	case DKPixelFormat::R32Sint:			return VK_FORMAT_R32_SINT;
	case DKPixelFormat::R32Float:			return VK_FORMAT_R32_SFLOAT;

	case DKPixelFormat::RG16Unorm:			return VK_FORMAT_R16G16_UNORM;
	case DKPixelFormat::RG16Snorm:			return VK_FORMAT_R16G16_SNORM;
	case DKPixelFormat::RG16Uint:			return VK_FORMAT_R16G16_UINT;
	case DKPixelFormat::RG16Sint:			return VK_FORMAT_R16G16_SINT;
	case DKPixelFormat::RG16Float:			return VK_FORMAT_R16G16_SFLOAT;

	case DKPixelFormat::RGBA8Unorm:			return VK_FORMAT_R8G8B8A8_UNORM;
	case DKPixelFormat::RGBA8Unorm_sRGB:	return VK_FORMAT_R8G8B8A8_SRGB;
	case DKPixelFormat::RGBA8Snorm:			return VK_FORMAT_R8G8B8A8_SNORM;
	case DKPixelFormat::RGBA8Uint:			return VK_FORMAT_R8G8B8A8_UINT;
	case DKPixelFormat::RGBA8Sint:			return VK_FORMAT_R8G8B8A8_SINT;

	case DKPixelFormat::BGRA8Unorm:			return VK_FORMAT_B8G8R8A8_UNORM;
	case DKPixelFormat::BGRA8Unorm_sRGB:	return VK_FORMAT_B8G8R8A8_SRGB;

	case DKPixelFormat::RGB10A2Unorm:		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case DKPixelFormat::RGB10A2Uint:		return VK_FORMAT_A2B10G10R10_UINT_PACK32;

	case DKPixelFormat::RG11B10Float:		return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	case DKPixelFormat::RGB9E5Float:		return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

	case DKPixelFormat::RG32Uint:			return VK_FORMAT_R32G32_UINT;
	case DKPixelFormat::RG32Sint:			return VK_FORMAT_R32G32_SINT;
	case DKPixelFormat::RG32Float:			return VK_FORMAT_R32G32_SFLOAT;

	case DKPixelFormat::RGBA16Unorm:		return VK_FORMAT_R16G16B16A16_UNORM;
	case DKPixelFormat::RGBA16Snorm:		return VK_FORMAT_R16G16B16A16_SNORM;
	case DKPixelFormat::RGBA16Uint:			return VK_FORMAT_R16G16B16A16_UINT;
	case DKPixelFormat::RGBA16Sint:			return VK_FORMAT_R16G16B16A16_SINT;
	case DKPixelFormat::RGBA16Float:		return VK_FORMAT_R16G16B16A16_SFLOAT;

	case DKPixelFormat::RGBA32Uint:			return VK_FORMAT_R32G32B32A32_UINT;
	case DKPixelFormat::RGBA32Sint:			return VK_FORMAT_R32G32B32A32_SINT;
	case DKPixelFormat::RGBA32Float:		return VK_FORMAT_R32G32B32A32_SFLOAT;

	case DKPixelFormat::D32:				return VK_FORMAT_D32_SFLOAT;

	case DKPixelFormat::D32S8X24:			return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}
	return VK_FORMAT_UNDEFINED;
}

DKPixelFormat PixelFormat::To(VkFormat fmt)
{
	switch (fmt)
	{
	case VK_FORMAT_R8_UNORM:					return DKPixelFormat::R8Unorm;
	case VK_FORMAT_R8_SNORM:					return DKPixelFormat::R8Snorm;
	case VK_FORMAT_R8_UINT:						return DKPixelFormat::R8Uint;
	case VK_FORMAT_R8_SINT:						return DKPixelFormat::R8Sint;

	case VK_FORMAT_R16_UNORM:					return DKPixelFormat::R16Unorm;
	case VK_FORMAT_R16_SNORM:					return DKPixelFormat::R16Snorm;
	case VK_FORMAT_R16_UINT:					return DKPixelFormat::R16Uint;
	case VK_FORMAT_R16_SINT:					return DKPixelFormat::R16Sint;
	case VK_FORMAT_R16_SFLOAT:					return DKPixelFormat::R16Float;

	case VK_FORMAT_R8G8_UNORM:					return DKPixelFormat::RG8Unorm;
	case VK_FORMAT_R8G8_SNORM:					return DKPixelFormat::RG8Snorm;
	case VK_FORMAT_R8G8_UINT:					return DKPixelFormat::RG8Uint;
	case VK_FORMAT_R8G8_SINT:					return DKPixelFormat::RG8Sint;

	case VK_FORMAT_R32_UINT:					return DKPixelFormat::R32Uint;
	case VK_FORMAT_R32_SINT:					return DKPixelFormat::R32Sint;
	case VK_FORMAT_R32_SFLOAT:					return DKPixelFormat::R32Float;

	case VK_FORMAT_R16G16_UNORM:				return DKPixelFormat::RG16Unorm;
	case VK_FORMAT_R16G16_SNORM:				return DKPixelFormat::RG16Snorm;
	case VK_FORMAT_R16G16_UINT:					return DKPixelFormat::RG16Uint;
	case VK_FORMAT_R16G16_SINT:					return DKPixelFormat::RG16Sint;
	case VK_FORMAT_R16G16_SFLOAT:				return DKPixelFormat::RG16Float;

	case VK_FORMAT_R8G8B8A8_UNORM:				return DKPixelFormat::RGBA8Unorm;
	case VK_FORMAT_R8G8B8A8_SRGB:				return DKPixelFormat::RGBA8Unorm_sRGB;
	case VK_FORMAT_R8G8B8A8_SNORM:				return DKPixelFormat::RGBA8Snorm;
	case VK_FORMAT_R8G8B8A8_UINT:				return DKPixelFormat::RGBA8Uint;
	case VK_FORMAT_R8G8B8A8_SINT:				return DKPixelFormat::RGBA8Sint;

	case VK_FORMAT_B8G8R8A8_UNORM:				return DKPixelFormat::BGRA8Unorm;
	case VK_FORMAT_B8G8R8A8_SRGB:				return DKPixelFormat::BGRA8Unorm_sRGB;

	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:	return DKPixelFormat::RGB10A2Unorm;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:		return DKPixelFormat::RGB10A2Uint;

	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:		return DKPixelFormat::RG11B10Float;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:		return DKPixelFormat::RGB9E5Float;

	case VK_FORMAT_R32G32_UINT:					return DKPixelFormat::RG32Uint;
	case VK_FORMAT_R32G32_SINT:					return DKPixelFormat::RG32Sint;
	case VK_FORMAT_R32G32_SFLOAT:				return DKPixelFormat::RG32Float;

	case VK_FORMAT_R16G16B16A16_UNORM:			return DKPixelFormat::RGBA16Unorm;
	case VK_FORMAT_R16G16B16A16_SNORM:			return DKPixelFormat::RGBA16Snorm;
	case VK_FORMAT_R16G16B16A16_UINT:			return DKPixelFormat::RGBA16Uint;
	case VK_FORMAT_R16G16B16A16_SINT:			return DKPixelFormat::RGBA16Sint;
	case VK_FORMAT_R16G16B16A16_SFLOAT:			return DKPixelFormat::RGBA16Float;

	case VK_FORMAT_R32G32B32A32_UINT:			return DKPixelFormat::RGBA32Uint;
	case VK_FORMAT_R32G32B32A32_SINT:			return DKPixelFormat::RGBA32Sint;
	case VK_FORMAT_R32G32B32A32_SFLOAT:			return DKPixelFormat::RGBA32Float;

	case VK_FORMAT_D32_SFLOAT:					return DKPixelFormat::D32;

	case VK_FORMAT_D32_SFLOAT_S8_UINT:			return DKPixelFormat::D32S8X24;

	}
	return DKPixelFormat::Invalid;
}

#endif //#if DKGL_USE_VULKAN
