//
//  File: TextureBaseT.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKTexture.h"
#include "PixelFormat.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			template <typename BaseClassT> struct TextureBaseT : public BaseClassT
			{
				static_assert(DKTypeConversionTest<BaseClassT, DKTexture>(), "BaseClassT must be DKTexture or DKTexture subclass");

				VkImage					image;
				VkImageType				imageType;
				VkFormat				format;
				VkExtent3D				extent;
				uint32_t				mipLevels;
				uint32_t				arrayLayers;
				VkImageUsageFlags		usage;
				VkSemaphore				waitSemaphore;

				TextureBaseT(VkImage i, const VkImageCreateInfo* ci)
					: image(i)
					, imageType(VK_IMAGE_TYPE_1D)
					, format(VK_FORMAT_UNDEFINED)
					, extent({ 0,0,0 })
					, mipLevels(0)
					, arrayLayers(0)
					, usage(0)
					, waitSemaphore(VK_NULL_HANDLE)
				{
					if (ci)
					{
						imageType = ci->imageType;
						format = ci->format;
						extent = ci->extent;
						mipLevels = ci->mipLevels;
						arrayLayers = ci->arrayLayers;
						usage = ci->usage;
					}
				}

				uint32_t Width(void) const override
				{
					DKASSERT_DEBUG(extent.width > 0);
					return extent.width;
				}
				uint32_t Height(void) const override
				{
					DKASSERT_DEBUG(extent.height > 0);
					return extent.height;
				}
				uint32_t Depth(void) const override
				{
					DKASSERT_DEBUG(extent.depth > 0);
					return extent.depth;
				}

				uint32_t MipmapCount(void) const override
				{
					DKASSERT_DEBUG(mipLevels > 0);
					return mipLevels;
				}

				DKTexture::Type TextureType(void) const override
				{
					switch (imageType)
					{
					case VK_IMAGE_TYPE_1D :
						return arrayLayers > 1 ? DKTexture::Type1DArray : DKTexture::Type1D;
					case VK_IMAGE_TYPE_2D :
						return arrayLayers > 1 ? DKTexture::Type2DArray : DKTexture::Type2D;
					case VK_IMAGE_TYPE_3D :
						return DKTexture::Type3D;
					}
					return DKTexture::TypeUnknown;
				}

				DKPixelFormat PixelFormat(void) const override
				{
					DKASSERT_DEBUG(format != VK_FORMAT_UNDEFINED);
					return PixelFormat::To(format);
				}
			};
		}
	}
}

#endif //#if DKGL_USE_VULKAN
