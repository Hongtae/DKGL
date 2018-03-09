//
//  File: Texture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2018 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKTexture.h"
#include "PixelFormat.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class Texture : public DKTexture
			{
			public:
				Texture(DKGraphicsDevice*, VkImage, VkImageView, const VkImageCreateInfo*);
				~Texture(void);

				VkImage					image;
				VkImageType				imageType;
				VkFormat				format;
				VkExtent3D				extent;
				uint32_t				mipLevels;
				uint32_t				arrayLayers;
				VkImageUsageFlags		usage;

				VkImageView				imageView;
				VkSemaphore				waitSemaphore;
				VkSemaphore				signalSemaphore;

				DKObject<DKGraphicsDevice> device;

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
					case VK_IMAGE_TYPE_1D:
						return arrayLayers > 1 ? DKTexture::Type1DArray : DKTexture::Type1D;
					case VK_IMAGE_TYPE_2D:
						return arrayLayers > 1 ? DKTexture::Type2DArray : DKTexture::Type2D;
					case VK_IMAGE_TYPE_3D:
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

#endif //#if DKGL_ENABLE_VULKAN
