//
//  File: Semaphore.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "Semaphore.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

Semaphore::Semaphore(DKGraphicsDevice* d, id<MTLEvent> e)
: device(d)
, event([e retain])
{
    DKASSERT_DEBUG(event);
}

Semaphore::~Semaphore()
{
    //GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    [event release];
}

#endif //#if DKGL_ENABLE_METAL
