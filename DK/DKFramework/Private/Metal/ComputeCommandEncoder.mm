//
//  File: RenderCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "ComputeCommandEncoder.h"
#include "ComputePipelineState.h"
#include "ShaderBindingSet.h"
#include "Event.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

#pragma mark - ComputeCommandEncoder::Encoder
bool ComputeCommandEncoder::Encoder::Encode(id<MTLCommandBuffer> buffer)
{
    for (DKGpuEvent* e : waitEvents)
    {
        DKASSERT_DEBUG(dynamic_cast<Event*>(e));
        Event* event = static_cast<Event*>(e);

        [buffer encodeWaitForEvent:event->event
                             value:event->NextWaitValue()];
    }

    id<MTLComputeCommandEncoder> encoder = [buffer computeCommandEncoder];
    EncodingState state = {};
    for (EncoderCommand* command : commands )
    {
        command->Invoke(encoder, state);
    }
    [encoder endEncoding];

    for (DKGpuEvent* e : signalEvents)
    {
        DKASSERT_DEBUG(dynamic_cast<Event*>(e));
        Event* event = static_cast<Event*>(e);

        [buffer encodeSignalEvent:event->event
                            value:event->NextSignalValue()];
    }

    return true;
}

#pragma mark - ComputeCommandEncoder
ComputeCommandEncoder::ComputeCommandEncoder(class CommandBuffer* b)
: commandBuffer(b)
{
	encoder = DKOBJECT_NEW Encoder();
	encoder->commands.Reserve(CommandEncoder::InitialNumberOfCommands);
}

ComputeCommandEncoder::~ComputeCommandEncoder()
{
}

void ComputeCommandEncoder::EndEncoding()
{
	DKASSERT_DEBUG(!IsCompleted());
	encoder->commands.ShrinkToFit();
	commandBuffer->EndEncoder(this, encoder);
	encoder = NULL;
}

void ComputeCommandEncoder::WaitEvent(DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<Event*>(event));
    encoder->waitEvents.Add(event);
}

void ComputeCommandEncoder::SignalEvent(DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<Event*>(event));
    encoder->signalEvents.Add(event);
}

void ComputeCommandEncoder::SetResources(uint32_t set, DKShaderBindingSet* binds)
{
    DKASSERT_DEBUG(!IsCompleted());
    DKASSERT_DEBUG(dynamic_cast<ShaderBindingSet*>(binds));
    DKObject<ShaderBindingSet> bs = static_cast<ShaderBindingSet*>(binds);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLComputeCommandEncoder> encoder, EncodingState& state)
    {
        if (state.pipelineState)
        {
            // bind resources
            bs->BindResources(set, state.pipelineState->bindings.resourceBindings,
                              [&](const ShaderBindingSet::BufferObject* bufferObjects, uint32_t index, size_t numBuffers)
                              {
                                  id<MTLBuffer>* buffers = new id<MTLBuffer>[numBuffers];
                                  NSUInteger* offsets = new NSUInteger[numBuffers];
                                  for (size_t i = 0; i < numBuffers; ++i)
                                  {
                                      buffers[i] = bufferObjects[i].buffer->buffer;
                                      offsets[i] = bufferObjects[i].offset;
                                  }
                                  [encoder setBuffers:buffers
                                              offsets:offsets
                                            withRange:NSMakeRange(index, numBuffers)];
                                  delete[] buffers;
                                  delete[] offsets;
                              },
                              [&](const ShaderBindingSet::TextureObject* textureObjects, uint32_t index, size_t numTextures)
                              {
                                  id<MTLTexture>* textures = new id<MTLTexture>[numTextures];
                                  for (size_t i = 0; i < numTextures; ++i)
                                      textures[i] = textureObjects[i]->texture;
                                  [encoder setTextures:textures
                                             withRange:NSMakeRange(index, numTextures)];
                                  delete[] textures;
                              },
                              [&](const ShaderBindingSet::SamplerObject* samplerObjects, uint32_t index, size_t numSamplers)
                              {
                                  id<MTLSamplerState>* samplers = new id<MTLSamplerState>[numSamplers];
                                  for (size_t i = 0; i < numSamplers; ++i)
                                      samplers[i] = samplerObjects[i]->sampler;
                                  [encoder setSamplerStates:samplers
                                                  withRange:NSMakeRange(index, numSamplers)];
                                  delete[] samplers;
                              });
        }
    });
    encoder->commands.Add(command);
}

void ComputeCommandEncoder::SetComputePipelineState(DKComputePipelineState* ps)
{
    DKASSERT_DEBUG(!IsCompleted());
    DKASSERT_DEBUG(dynamic_cast<ComputePipelineState*>(ps));
    DKObject<ComputePipelineState> pipeline = static_cast<ComputePipelineState*>(ps);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLComputeCommandEncoder> encoder, EncodingState& state)
    {
        id<MTLComputePipelineState> pipelineState = pipeline->pipelineState;
        [encoder setComputePipelineState:pipelineState];
        state.pipelineState = pipeline;
    });
    encoder->commands.Add(command);
}

void ComputeCommandEncoder::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
    DKASSERT_DEBUG(!IsCompleted());

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLComputeCommandEncoder> encoder, EncodingState & state)
    {
        if (state.pipelineState)
        {
            [encoder dispatchThreadgroups:MTLSizeMake(numGroupsX, numGroupsY, numGroupsZ)
                    threadsPerThreadgroup:state.pipelineState->workgroupSize] ;
        }
        else
        {
            DKASSERT_DEBUG(0);
        }
    });
    encoder->commands.Add(command);
}

#endif //#if DKGL_ENABLE_METAL
