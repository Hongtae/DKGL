//
//  File: Texture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKTexture.h"
#include "Types.h"
#include "DeviceMemory.h"

namespace DKFramework::Private::Vulkan
{
	class Texture : public DKTexture
	{
	public:
		Texture(DKGraphicsDevice*, VkImage, VkImageView, const VkImageCreateInfo*);
		~Texture();

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

        DKObject<DeviceMemory>      deviceMemory;
		DKObject<DKGraphicsDevice>  device;

		uint32_t Width() const override
		{
			DKASSERT_DEBUG(extent.width > 0);
			return extent.width;
		}
		uint32_t Height() const override
		{
			DKASSERT_DEBUG(extent.height > 0);
			return extent.height;
		}
		uint32_t Depth() const override
		{
			DKASSERT_DEBUG(extent.depth > 0);
			return extent.depth;
		}

		uint32_t MipmapCount() const override
		{
			DKASSERT_DEBUG(mipLevels > 0);
			return mipLevels;
		}
        uint32_t ArrayLength() const override
        {
            DKASSERT_DEBUG(arrayLayers > 0);
            return arrayLayers;
        }

		DKTexture::Type TextureType() const override
		{
			switch (imageType)
			{
			case VK_IMAGE_TYPE_1D:
				return DKTexture::Type1D;
			case VK_IMAGE_TYPE_2D:
				return DKTexture::Type2D;
			case VK_IMAGE_TYPE_3D:
				return DKTexture::Type3D;
			}
			return DKTexture::TypeUnknown;
		}

		DKPixelFormat PixelFormat() const override
		{
			DKASSERT_DEBUG(format != VK_FORMAT_UNDEFINED);
			return Vulkan::PixelFormat(format);
		}
	};
}

#endif //#if DKGL_ENABLE_VULKAN
