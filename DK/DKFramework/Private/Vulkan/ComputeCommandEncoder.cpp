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

ComputeCommandEncoder::Encoder::Encoder(class CommandBuffer* cb)
    : commandBuffer(cb)
{
}

ComputeCommandEncoder::Encoder::~Encoder()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(commandBuffer->Queue()->Device());
    VkDevice device = dev->device;
}

bool ComputeCommandEncoder::Encoder::Encode(VkCommandBuffer commandBuffer)
{
     // recording commands
    EncodingState state = {};
    for (EncoderCommand* c : commands)
    {
        c->Invoke(commandBuffer, state);
    }
    return true;
}

ComputeCommandEncoder::ComputeCommandEncoder(class CommandBuffer* cb)
    : commandBuffer(cb)
{
    encoder = DKOBJECT_NEW Encoder(cb);
}

void ComputeCommandEncoder::EndEncoding()
{
    commandBuffer->EndEncoder(this, encoder);
    encoder = nullptr;
}

void ComputeCommandEncoder::SetResources(uint32_t index, DKShaderBindingSet* set)
{
    DKObject<DescriptorSet> ds = nullptr;

    if (set)
    {
        DKASSERT_DEBUG(dynamic_cast<ShaderBindingSet*>(set) != nullptr);
        ShaderBindingSet* bindingSet = static_cast<ShaderBindingSet*>(set);

        // keep ownership 
        ds = bindingSet->CreateDescriptorSet();
        encoder->descriptorSets.Add(ds);
    }

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        if (state.pipelineState)
        {
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            if (ds)
            {
                descriptorSet = ds->descriptorSet;
            }

            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_COMPUTE,
                                    state.pipelineState->layout,
                                    index,
                                    1,
                                    &descriptorSet,
                                    0,      // dynamic offsets
                                    0);
        }
    });
    encoder->commands.Add(command);
}

void ComputeCommandEncoder::SetComputePipelineState(DKComputePipelineState* ps)
{
    DKASSERT_DEBUG(dynamic_cast<ComputePipelineState*>(ps));
    ComputePipelineState* pipeline = static_cast<ComputePipelineState*>(ps);

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline);
        state.pipelineState = pipeline;
    });
    encoder->commands.Add(command);
    encoder->pipelineStateObjects.Add(pipeline);
}

#endif //#if DKGL_ENABLE_VULKAN
