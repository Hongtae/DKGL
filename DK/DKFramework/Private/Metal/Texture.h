//
//  File: Texture.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKTexture.h"
#include "../../DKGraphicsDevice.h"
#include "PixelFormat.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class Texture : public DKTexture
			{
			public:
				Texture(DKGraphicsDevice*, id<MTLTexture>);
				~Texture(void);

				uint32_t Width(void) const override
				{
					return static_cast<uint32_t>(texture.width);
				}

				uint32_t Height(void) const override
				{
					return static_cast<uint32_t>(texture.height);
				}

				uint32_t Depth(void) const override
				{
					return static_cast<uint32_t>(texture.depth);
				}

				uint32_t MipmapCount(void) const override
				{
					return static_cast<uint32_t>(texture.mipmapLevelCount);
				}

				DKTexture::Type TextureType(void) const override
				{
					MTLTextureType t = texture.textureType;
					switch (t)
					{
						case MTLTextureType1D:
							return DKTexture::Type1D;
						case MTLTextureType1DArray:
							return DKTexture::Type1DArray;
						case MTLTextureType2D:
							return DKTexture::Type2D;
						case MTLTextureType2DArray:
							return DKTexture::Type2DArray;
						case MTLTextureType3D:
							return DKTexture::Type3D;
						case MTLTextureTypeCube:
							return DKTexture::TypeCube;
					}
					return DKTexture::TypeUnknown;
				}

				DKPixelFormat PixelFormat(void) const override
				{
					return PixelFormat::To(texture.pixelFormat);
				}

				id<MTLTexture> texture;
				DKObject<DKGraphicsDevice> device;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
