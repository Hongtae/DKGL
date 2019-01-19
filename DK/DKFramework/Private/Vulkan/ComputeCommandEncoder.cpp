//
//  File: ComputeCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "ComputeCommandEncoder.h"
#include "ComputePipelineState.h"
#include "ShaderBindingSet.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ComputeCommandEncoder::Resources::Resources(class CommandBuffer* b)
    : cb(b)
    , commandBuffer(VK_NULL_HANDLE)
{
}

ComputeCommandEncoder::Resources::~Resources()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(cb->Queue()->Device());
    VkDevice device = dev->device;


    if (commandBuffer)
        cb->ReleaseEncodingBuffer(commandBuffer);
}


ComputeCommandEncoder::ComputeCommandEncoder(VkCommandBuffer vcb, class CommandBuffer* cb)
    : commandBuffer(cb)
{
    resources = DKOBJECT_NEW Resources(cb);
    resources->commandBuffer = vcb;
    DKASSERT_DEBUG(resources->commandBuffer);
}

ComputeCommandEncoder::~ComputeCommandEncoder()
{
}

void ComputeCommandEncoder::EndEncoding()
{
}

DKCommandBuffer* ComputeCommandEncoder::CommandBuffer()
{
    return commandBuffer;
}

void ComputeCommandEncoder::SetResources(uint32_t set, DKShaderBindingSet*)
{

}

void ComputeCommandEncoder::SetComputePipelineState(DKComputePipelineState* ps)
{
 
}

#endif //#if DKGL_ENABLE_VULKAN
