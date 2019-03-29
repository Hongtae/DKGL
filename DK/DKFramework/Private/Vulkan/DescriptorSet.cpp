//
//  File: DescriptorSet.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "GraphicsDevice.h"
#include "DescriptorSet.h"
#include "DescriptorPool.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

DescriptorSet::DescriptorSet(DKGraphicsDevice* dev,
                             DescriptorPool* pool,
                             VkDescriptorSet ds)
    : device(dev)
    , descriptorSet(ds)
    , descriptorPool(pool)
{
}

DescriptorSet::~DescriptorSet()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    dev->DestroyDescriptorSets(descriptorPool, &descriptorSet, 1);
}

#endif //#if DKGL_ENABLE_VULKAN
