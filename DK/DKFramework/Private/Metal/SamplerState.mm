//
//  File: SamplerState.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "SamplerState.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

SamplerState::SamplerState(DKGraphicsDevice* d, id<MTLSamplerState> s)
: device(d)
, sampler([s retain])
{
    DKASSERT_DEBUG(sampler);
}

SamplerState::~SamplerState()
{
    //GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    [sampler release];
}

#endif //#if DKGL_ENABLE_METAL

