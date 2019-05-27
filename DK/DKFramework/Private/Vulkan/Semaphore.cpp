//
//  File: Semaphore.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Semaphore.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

Semaphore::Semaphore(DKGraphicsDevice* dev, VkSemaphore s)
    : device(dev)
    , semaphore(s)
{
}

Semaphore::~Semaphore()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    vkDestroySemaphore(dev->device, semaphore, dev->allocationCallbacks);
}

#endif //#if DKGL_ENABLE_VULKAN
