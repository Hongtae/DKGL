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
#include "Semaphore.h"
#include "TimelineSemaphore.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ComputeCommandEncoder::Encoder::Encoder(class CommandBuffer* cb)
    : commandBuffer(cb)
{
    commands.Reserve(InitialNumberOfCommands);
    setupCommands.Reserve(InitialNumberOfCommands);
    cleanupCommands.Reserve(InitialNumberOfCommands);
}

ComputeCommandEncoder::Encoder::~Encoder()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(commandBuffer->Queue()->Device());
    VkDevice device = dev->device;
}

bool ComputeCommandEncoder::Encoder::Encode(VkCommandBuffer commandBuffer)
{
    // recording commands
    EncodingState state = { this };
    // collect image layout transition
    for (ShaderBindingSet* bs : shaderBindingSets)
    {
        bs->CollectImageViewLayouts(state.imageLayoutMap, state.imageViewLayoutMap);
    }
    for (EncoderCommand* c : setupCommands)
    {
        c->Invoke(commandBuffer, state);
    }
    // Set image layout transition
    state.imageLayoutMap.EnumerateForward([&](decltype(state.imageLayoutMap)::Pair& pair)
    {
        Image* image = pair.key;
        VkImageLayout layout = pair.value;
        VkAccessFlags accessMask = Image::CommonLayoutAccessMask(layout);

        image->SetLayout(layout,
                         accessMask,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         state.encoder->commandBuffer->QueueFamily()->familyIndex,
                         commandBuffer);
    });
    for (EncoderCommand* c : commands)
    {
        c->Invoke(commandBuffer, state);
    }
    for (EncoderCommand* c : cleanupCommands)
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

void ComputeCommandEncoder::WaitEvent(const DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<const Semaphore*>(event));
    const Semaphore* semaphore = static_cast<const Semaphore*>(event);

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    encoder->AddWaitSemaphore(semaphore->semaphore, pipelineStages, semaphore->NextWaitValue());
    encoder->events.Add(const_cast<DKGpuEvent*>(event));
}

void ComputeCommandEncoder::SignalEvent(const DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<const Semaphore*>(event));
    const Semaphore* semaphore = static_cast<const Semaphore*>(event);

    encoder->AddSignalSemaphore(semaphore->semaphore, semaphore->NextSignalValue());
    encoder->events.Add(const_cast<DKGpuEvent*>(event));
}

void ComputeCommandEncoder::WaitSemaphoreValue(const DKGpuSemaphore* sema, uint64_t value)
{
    DKASSERT_DEBUG(dynamic_cast<const TimelineSemaphore*>(sema));
    const TimelineSemaphore* semaphore = static_cast<const TimelineSemaphore*>(sema);

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    encoder->AddWaitSemaphore(semaphore->semaphore, value, pipelineStages);
    encoder->semaphores.Add(const_cast<DKGpuSemaphore*>(sema));
}

void ComputeCommandEncoder::SignalSemaphoreValue(const DKGpuSemaphore* sema, uint64_t value)
{
    DKASSERT_DEBUG(dynamic_cast<const TimelineSemaphore*>(sema));
    const TimelineSemaphore* semaphore = static_cast<const TimelineSemaphore*>(sema);

    encoder->AddSignalSemaphore(semaphore->semaphore, value);
    encoder->semaphores.Add(const_cast<DKGpuSemaphore*>(sema));
}

void ComputeCommandEncoder::SetResources(uint32_t index, const DKShaderBindingSet* set)
{
    DKObject<ShaderBindingSet> bindingSet = nullptr;
    if (set)
    {
        DKASSERT_DEBUG(dynamic_cast<const ShaderBindingSet*>(set) != nullptr);
        auto p = static_cast<const ShaderBindingSet*>(set);
        bindingSet = const_cast<ShaderBindingSet*>(p);
        encoder->shaderBindingSets.Add(bindingSet);
    }

    DKObject<EncoderCommand> preCommand = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        if (bindingSet)
        {
            DKObject<DescriptorSet> ds = bindingSet->CreateDescriptorSet(state.imageViewLayoutMap);
            DKASSERT_DEBUG(ds);

            if (ds)
            {
                state.bindingSetMap.Update(bindingSet, ds);

                // keep ownership 
                state.encoder->descriptorSets.Add(ds);
            }
        }
    });
    encoder->setupCommands.Add(preCommand);

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        if (state.pipelineState)
        {
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

            if (bindingSet)
            {
                if (auto p = state.bindingSetMap.Find(bindingSet); p)
                {
                    descriptorSet = p->value->descriptorSet;
                    DKASSERT_DEBUG(descriptorSet != VK_NULL_HANDLE);
                }
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

void ComputeCommandEncoder::SetComputePipelineState(const DKComputePipelineState* ps)
{
    DKASSERT_DEBUG(dynamic_cast<const ComputePipelineState*>(ps));
    const ComputePipelineState* pipeline = static_cast<const ComputePipelineState*>(ps);

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline);
        state.pipelineState = const_cast<ComputePipelineState*>(pipeline);
    });
    encoder->commands.Add(command);
    encoder->pipelineStateObjects.Add(const_cast<ComputePipelineState*>(pipeline));
}

void ComputeCommandEncoder::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState & state) mutable
    {
        vkCmdDispatch(commandBuffer, numGroupsX, numGroupsY, numGroupsZ);
    });
    encoder->commands.Add(command);
}

#endif //#if DKGL_ENABLE_VULKAN
