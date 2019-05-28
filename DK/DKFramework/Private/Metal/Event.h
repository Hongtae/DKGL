//
//  File: Event.h
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
    class Event : public DKGpuEvent
    {
    public:
        Event(DKGraphicsDevice*, id<MTLEvent>);
        ~Event();

        id<MTLEvent> event;
        DKObject<DKGraphicsDevice> device;

        uint64_t NextWaitValue();
        uint64_t NextSignalValue();

    private:
        DKAtomicNumber64 waitValue;
        DKAtomicNumber64 signalValue;
    };
}
#endif //#if DKGL_ENABLE_METAL
