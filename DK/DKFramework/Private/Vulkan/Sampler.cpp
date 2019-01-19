//
//  File: Sampler.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "Sampler.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

Sampler::Sampler(DKGraphicsDevice* dev, VkSampler s)
    : device(dev)
    , sampler(s)
{

}

Sampler::~Sampler()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    vkDestroySampler(dev->device, sampler, dev->allocationCallbacks);
}

#endif //#if DKGL_ENABLE_VULKAN
