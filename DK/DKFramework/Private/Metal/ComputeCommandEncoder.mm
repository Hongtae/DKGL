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

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ComputeCommandEncoder::ComputeCommandEncoder(class CommandBuffer* b)
: commandBuffer(b)
{
	reusableEncoder = DKOBJECT_NEW ReusableEncoder();
	reusableEncoder->encoderCommands.Reserve(ReusableCommandEncoder::InitialNumberOfCommands);
}

ComputeCommandEncoder::~ComputeCommandEncoder()
{
}

void ComputeCommandEncoder::EndEncoding()
{
	DKASSERT_DEBUG(!IsCompleted());
	reusableEncoder->encoderCommands.ShrinkToFit();
	commandBuffer->EndEncoder(this, reusableEncoder);
	reusableEncoder = NULL;
}

void ComputeCommandEncoder::SetResources(uint32_t set, DKShaderBindingSet* binds)
{
    DKASSERT_DEBUG(!IsCompleted());
    DKASSERT_DEBUG(dynamic_cast<ShaderBindingSet*>(binds));
    DKObject<ShaderBindingSet> bs = static_cast<ShaderBindingSet*>(binds);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLComputeCommandEncoder> encoder, Resources& res)
    {
        if (res.pipelineState)
        {
            // bind resources
            bs->BindResources(set, res.pipelineState->bindings.resourceBindings,
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
    reusableEncoder->encoderCommands.Add(command);
}

void ComputeCommandEncoder::SetComputePipelineState(DKComputePipelineState* ps)
{
    DKASSERT_DEBUG(!IsCompleted());
    DKASSERT_DEBUG(dynamic_cast<ComputePipelineState*>(ps));
    DKObject<ComputePipelineState> pipeline = static_cast<ComputePipelineState*>(ps);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLComputeCommandEncoder> encoder, Resources& res)
    {
        id<MTLComputePipelineState> pipelineState = pipeline->pipelineState;
        [encoder setComputePipelineState:pipelineState];
        res.pipelineState = pipeline;
    });
    reusableEncoder->encoderCommands.Add(command);
}

#endif //#if DKGL_ENABLE_METAL
