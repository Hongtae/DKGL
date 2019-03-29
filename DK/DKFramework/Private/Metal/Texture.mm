//
//  File: Texture.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "Texture.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

Texture::Texture(DKGraphicsDevice* d, id<MTLTexture> t)
: device(d)
, texture([t retain])
{
    DKASSERT_DEBUG(texture);
}

Texture::~Texture()
{
	//GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	[texture release];
}

#endif //#if DKGL_ENABLE_METAL
