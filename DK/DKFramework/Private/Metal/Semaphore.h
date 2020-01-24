//
//  File: Semaphore.h
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
    class Semaphore : public DKGpuSemaphore
    {
    public:
        Semaphore(DKGraphicsDevice*, id<MTLEvent>);
        ~Semaphore();

        id<MTLEvent> event;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_METAL
