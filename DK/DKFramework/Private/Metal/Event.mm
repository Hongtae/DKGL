//
//  File: Event.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "Event.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

Event::Event(DKGraphicsDevice* d, id<MTLEvent> e)
: device(d)
, event([e retain])
, waitValue(0)
, signalValue(0)
{
    DKASSERT_DEBUG(event);
}

Event::~Event()
{
    //GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    [event release];
}

uint64_t Event::NextWaitValue() const
{
    auto n = waitValue.Increment();
    return n+1;
}

uint64_t Event::NextSignalValue() const
{
    auto n = signalValue.Increment();
    return n+1;
}

#endif //#if DKGL_ENABLE_METAL
