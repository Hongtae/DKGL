//
//  File: Fence.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKGpuResource.h"

namespace DKFramework::Private::Metal
{
    class Fence : public DKGpuEvent
    {
    public:
        Fence(DKGraphicsDevice*, id<MTLFence>);
        ~Fence();

        id<MTLFence> fence;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_METAL
