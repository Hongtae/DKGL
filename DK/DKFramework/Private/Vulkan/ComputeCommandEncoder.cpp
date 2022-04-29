//
//  File: ComputeCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2022 Hongtae Kim. All rights reserved.
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
    for (DescriptorSet* ds : descriptorSets)
    {
        ds->CollectImageViewLayouts(state.imageLayoutMap, state.imageViewLayoutMap);
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

    encoder->AddWaitSemaphore(semaphore->semaphore, semaphore->NextWaitValue(), pipelineStages);
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
    DKObject<DescriptorSet> descriptorSet = nullptr;
    if (set)
    {
        DKASSERT_DEBUG(dynamic_cast<const ShaderBindingSet*>(set) != nullptr);
        const ShaderBindingSet* bindingSet = static_cast<const ShaderBindingSet*>(set);
        descriptorSet = bindingSet->CreateDescriptorSet();
        DKASSERT_DEBUG(descriptorSet);
        encoder->descriptorSets.Add(descriptorSet);
    }
    if (descriptorSet == nullptr)
        return;

    DKObject<EncoderCommand> preCommand = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        descriptorSet->UpdateImageViewLayouts(state.imageViewLayoutMap);
    });
    encoder->setupCommands.Add(preCommand);

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        if (state.pipelineState)
        {
            VkDescriptorSet ds = descriptorSet->descriptorSet;
            DKASSERT_DEBUG(ds != VK_NULL_HANDLE);

            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_COMPUTE,
                                    state.pipelineState->layout,
                                    index,
                                    1,
                                    &ds,
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

void ComputeCommandEncoder::PushConstant(uint32_t stages, uint32_t offset, uint32_t size, const void* data)
{
    VkShaderStageFlags stageFlags = 0;
    for (auto& stage : {
        std::pair(DKShaderStage::Compute, VK_SHADER_STAGE_COMPUTE_BIT),
         })
    {
        if (stages & (uint32_t)stage.first)
            stageFlags |= stage.second;
    }
    if (stageFlags && size > 0)
    {
        DKASSERT_DEBUG(data);

        DKObject<DKArray<uint8_t>> buffer = DKOBJECT_NEW DKArray<uint8_t>((const uint8_t*)data, size);

        DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
        {
            if (state.pipelineState)
            {
                vkCmdPushConstants(commandBuffer,
                                   state.pipelineState->layout,
                                   stageFlags,
                                   offset, size,
                                   *buffer);
            }
        });
        encoder->commands.Add(command);
    }
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
