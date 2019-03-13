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
#include "Image.h"

namespace DKFramework::Private::Vulkan
{
    class ImageView : public DKTexture
    {
    public:
        ImageView(Image*, VkImageView, const VkImageViewCreateInfo&);
        ImageView();
        ~ImageView();

        VkImageView				imageView;
        VkSemaphore				waitSemaphore;
        VkSemaphore				signalSemaphore;

        DKObject<Image> image;

        uint32_t Width() const override
        {
            return image->Width();
        }
        uint32_t Height() const override
        {
            return image->Height();
        }
        uint32_t Depth() const override
        {
            return image->Depth();
        }
        uint32_t MipmapCount() const override
        {
            return image->MipmapCount();
        }
        uint32_t ArrayLength() const override
        {
            return image->ArrayLength();
        }
        DKTexture::Type TextureType() const override
        {
            return image->TextureType();
        }
        DKPixelFormat PixelFormat() const override
        {
            return image->PixelFormat();
        }
    };
}

#endif //#if DKGL_ENABLE_VULKAN
