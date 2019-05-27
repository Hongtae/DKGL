//
//  File: Fence.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "Fence.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

Fence::Fence(DKGraphicsDevice* d, id<MTLFence> f)
: device(d)
, fence([f retain])
{
    DKASSERT_DEBUG(fence);
}

Fence::~Fence()
{
    //GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    [fence release];
}

#endif //#if DKGL_ENABLE_METAL
