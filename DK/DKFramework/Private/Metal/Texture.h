//
//  File: Texture.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKTexture.h"
#include "../../DKGraphicsDevice.h"
#include "Types.h"

namespace DKFramework::Private::Metal
{
	class Texture : public DKTexture
	{
	public:
		Texture(DKGraphicsDevice*, id<MTLTexture>);
		~Texture();

		uint32_t Width() const override
		{
			return static_cast<uint32_t>(texture.width);
		}

		uint32_t Height() const override
		{
			return static_cast<uint32_t>(texture.height);
		}

		uint32_t Depth() const override
		{
			return static_cast<uint32_t>(texture.depth);
		}

		uint32_t MipmapCount() const override
		{
			return static_cast<uint32_t>(texture.mipmapLevelCount);
		}

        uint32_t ArrayLength() const override
        {
            return static_cast<uint32_t>(texture.arrayLength);
        }

        DKTexture::Type TextureType() const override
        {
            MTLTextureType t = texture.textureType;
            switch (t)
            {
                case MTLTextureType1D:
                case MTLTextureType1DArray:
                    return DKTexture::Type1D;
                case MTLTextureType2D:
                case MTLTextureType2DArray:
                    return DKTexture::Type2D;
                case MTLTextureType3D:
                    return DKTexture::Type3D;
                case MTLTextureTypeCube:
                    return DKTexture::TypeCube;
            }
            return DKTexture::TypeUnknown;
		}

		DKPixelFormat PixelFormat() const override
		{
            return Metal::PixelFormat(texture.pixelFormat);
		}

		id<MTLTexture> texture;
		DKObject<DKGraphicsDevice> device;
	};
}
#endif //#if DKGL_ENABLE_METAL
