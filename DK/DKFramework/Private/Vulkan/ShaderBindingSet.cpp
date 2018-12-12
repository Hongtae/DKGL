//
//  File: ShaderBindingSet.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "GraphicsDevice.h"
#include "ShaderBindingSet.h"
#include "DescriptorPool.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ShaderBindingSet::ShaderBindingSet(DKGraphicsDevice* dev, VkDescriptorSet set, DescriptorPool* pool)
    : device(dev)
    , descriptorSet(set)
    , descriptorPool(pool)    
{
    DKASSERT_DEBUG(descriptorSet != VK_NULL_HANDLE);
    DKASSERT_DEBUG(descriptorPool);
}


ShaderBindingSet::~ShaderBindingSet()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    dev->DestroyDescriptorSet(descriptorSet, descriptorPool);
}

void ShaderBindingSet::SetBuffer(uint32_t binding, DKGpuBuffer*)
{

}

void ShaderBindingSet::SetTexture(uint32_t binding, DKTexture*)
{

}

void ShaderBindingSet::SetSamplerState(uint32_t binding, DKSamplerState*)
{

}

void ShaderBindingSet::SetTextureSampler(uint32_t binding, DKTexture*, DKSamplerState*)
{

}

#endif //#if DKGL_ENABLE_VULKAN
