//
//  File: Texture.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_METAL

#include "Texture.h"
#include "PixelFormat.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

Texture::Texture(id<MTLTexture> t)
: texture(nil)
{
	texture = [t retain];
}

Texture::~Texture(void)
{
	[texture autorelease];
}

uint32_t Texture::Width(void)
{
	return texture.width;
}

uint32_t Texture::Height(void)
{
	return texture.height;
}

uint32_t Texture::Depth(void)
{
	return texture.depth;
}

uint32_t Texture::MipmapCount(void)
{
	return texture.mipmapLevelCount;
}

Texture::Type Texture::TextureType(void)
{
	MTLTextureType t = texture.textureType;
	switch (t)
	{
		case MTLTextureType1D:
			return Type1D;
		case MTLTextureType1DArray:
			return Type1DArray;
		case MTLTextureType2D:
			return Type2D;
		case MTLTextureType2DArray:
			return Type2DArray;
		case MTLTextureType3D:
			return Type3D;
		case MTLTextureTypeCube:
			return TypeCube;
	}
	return TypeUnknown;
}

DKPixelFormat Texture::PixelFormat(void)
{
	return PixelFormat::To(texture.pixelFormat);
}

#endif //#if DKGL_USE_METAL
