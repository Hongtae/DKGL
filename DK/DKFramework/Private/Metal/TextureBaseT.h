//
//  File: TextureBaseT.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#import <Metal/Metal.h>

#include "../../DKTexture.h"
#include "PixelFormat.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			template <typename BaseClassT> struct TextureBaseT : public BaseClassT
			{
				static_assert(DKTypeConversionTest<BaseClassT, DKTexture>(), "BaseClassT must be DKTexture or DKTexture subclass");

				id<MTLTexture> texture;

				TextureBaseT(id<MTLTexture> tex) : texture(nil)
				{
					texture = [tex retain];
				}
				virtual ~TextureBaseT(void)
				{
					[texture release];
				}

				uint32_t Width(void) override
				{
					return texture.width;
				}

				uint32_t Height(void) override
				{
					return texture.height;
				}

				uint32_t Depth(void) override
				{
					return texture.depth;
				}

				uint32_t MipmapCount(void) override
				{
					return texture.mipmapLevelCount;
				}

				DKTexture::Type TextureType(void) override
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

				DKPixelFormat PixelFormat(void) override
				{
					return PixelFormat::To(texture.pixelFormat);
				}
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
