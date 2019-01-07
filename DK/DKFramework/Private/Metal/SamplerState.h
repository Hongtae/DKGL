//
//  File: SamplerState.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKSampler.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Metal
{
    class SamplerState : public DKSamplerState
    {
    public:
        SamplerState(DKGraphicsDevice*, id<MTLSamplerState>);
        ~SamplerState();

        id<MTLSamplerState> sampler;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_METAL

