//
//  File: RenderTarget.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "RenderTarget.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

RenderTarget::RenderTarget(id<MTLTexture> t, DKGraphicsDevice* d)
: TextureBaseT(t)
, device(d)
{

}

RenderTarget::~RenderTarget(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

}

#endif //#if DKGL_ENABLE_METAL
