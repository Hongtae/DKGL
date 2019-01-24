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

void ComputeCommandEncoder::AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags flags)
{
    if (semaphore != VK_NULL_HANDLE)
    {
        if (!semaphorePipelineStageMasks.Insert(semaphore, flags))
            semaphorePipelineStageMasks.Value(semaphore) |= flags;
    }
}

void ComputeCommandEncoder::AddSignalSemaphore(VkSemaphore semaphore)
{
    if (semaphore != VK_NULL_HANDLE)
        signalSemaphores.Insert(semaphore);
}

void ComputeCommandEncoder::EndEncoding()
{
    VkResult err = vkEndCommandBuffer(resources->commandBuffer);
    if (err != VK_SUCCESS)
    {
        DKLogE("ERROR: vkEndCommandBuffer failed: %s", VkResultCStr(err));
        DKASSERT(err == VK_SUCCESS);
    }

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &resources->commandBuffer;

    resources->waitSemaphores.Reserve(semaphorePipelineStageMasks.Count());
    resources->waitStageMasks.Reserve(semaphorePipelineStageMasks.Count());

    semaphorePipelineStageMasks.EnumerateForward([&](decltype(semaphorePipelineStageMasks)::Pair& pair)
    {
        resources->waitSemaphores.Add(pair.key);
        resources->waitStageMasks.Add(pair.value);
    });

    resources->signalSemaphores.Reserve(signalSemaphores.Count());
    signalSemaphores.EnumerateForward([&](VkSemaphore semaphore)
    {
        resources->signalSemaphores.Add(semaphore);
    });

    DKASSERT_DEBUG(resources->waitSemaphores.Count() == resources->waitStageMasks.Count());

    submitInfo.waitSemaphoreCount = resources->waitSemaphores.Count();
    submitInfo.pWaitSemaphores = resources->waitSemaphores;
    submitInfo.pWaitDstStageMask = resources->waitStageMasks;
    submitInfo.signalSemaphoreCount = resources->signalSemaphores.Count();
    submitInfo.pSignalSemaphores = resources->signalSemaphores;

    DKObject<DKOperation> submitCallback = DKFunction([](DKObject<Resources> res)
    {
        res->updateResources.EnumerateForward([](decltype(res->updateResources)::Pair& pair)
        {
            if (pair.value)
                pair.value->UpdateDescriptorSet();
        });
    })->Invocation(resources);

    DKObject<DKOperation> completedCallback = DKFunction([](DKObject<Resources> res)
    {
        res = NULL;
    })->Invocation(resources);

    commandBuffer->Submit(submitInfo, submitCallback, completedCallback);

    resources = NULL;
    semaphorePipelineStageMasks.Clear();
    signalSemaphores.Clear();
}

DKCommandBuffer* ComputeCommandEncoder::CommandBuffer()
{
    return commandBuffer;
}

void ComputeCommandEncoder::SetResources(uint32_t index, DKShaderBindingSet* set)
{
    ShaderBindingSet* bindingSet = nullptr;

    if (set)
    {
        DKASSERT_DEBUG(dynamic_cast<ShaderBindingSet*>(set) != nullptr);
        bindingSet = static_cast<ShaderBindingSet*>(set);
        bindingSet->UpdateDescriptorSet();

        // keep ownership 
        resources->shaderBindingSets.Add(bindingSet);
    }

    if (resources->pipelineState)
    {
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (bindingSet)
            descriptorSet = bindingSet->descriptorSet;

        vkCmdBindDescriptorSets(resources->commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            resources->pipelineState->layout,
            index,
            1,
            &descriptorSet,
            0,      // dynamic offsets
            0);
    }
    else
    {
        resources->unboundResources.Update(index, bindingSet);
    }
}

void ComputeCommandEncoder::SetComputePipelineState(DKComputePipelineState* ps)
{
    DKASSERT_DEBUG(dynamic_cast<ComputePipelineState*>(ps));
    ComputePipelineState* pipeline = static_cast<ComputePipelineState*>(ps);
    vkCmdBindPipeline(resources->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline);
    resources->pipelineStateObjects.Add(pipeline);
    resources->pipelineState = pipeline;

    // bind descriptor sets
    resources->unboundResources.EnumerateForward([&](decltype(resources->unboundResources)::Pair& pair)
    {
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (pair.value)
        {
            ShaderBindingSet* bindingSet = pair.value;
            DKASSERT_DEBUG(bindingSet);

            descriptorSet = bindingSet->descriptorSet;
            DKASSERT_DEBUG(descriptorSet != VK_NULL_HANDLE);

            vkCmdBindDescriptorSets(resources->commandBuffer,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipeline->layout,
                pair.key,
                1,
                &descriptorSet,
                0,      // dynamic offsets
                0);

            if (bindingSet->layoutFlags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)
                resources->updateResources.Update(pair.key, pair.value);
        }
    });
    resources->unboundResources.Clear();
}

#endif //#if DKGL_ENABLE_VULKAN
