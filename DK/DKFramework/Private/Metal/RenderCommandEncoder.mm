//
//  File: RenderCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "RenderCommandEncoder.h"
#include "RenderPipelineState.h"
#include "ShaderBindingSet.h"
#include "GraphicsDevice.h"
#include "Event.h"
#include "Semaphore.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

#pragma mark - RnederCommandEncoder::Encoder
RenderCommandEncoder::Encoder::Encoder(MTLRenderPassDescriptor* rpd)
: renderPassDescriptor([rpd retain])
{
    commands.Reserve(InitialNumberOfCommands);
}

RenderCommandEncoder::Encoder::~Encoder()
{
    [renderPassDescriptor autorelease];
}

bool RenderCommandEncoder::Encoder::Encode(id<MTLCommandBuffer> buffer)
{
    waitEvents.EnumerateForward([&](Event* event) {
        [buffer encodeWaitForEvent:event->event
                             value:event->NextWaitValue()];
    });
    waitSemaphores.EnumerateForward([&](DKMap<Semaphore*, uint64_t>::Pair& pair) {
        [buffer encodeWaitForEvent:pair.key->event
                             value:pair.value];
    });

    bool result = false;
    DKASSERT_DEBUG(renderPassDescriptor);
    if (renderPassDescriptor)
    {
        id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        EncodingState state = {};
        for (EncoderCommand* command : commands )
        {
            command->Invoke(encoder, state);
        }
        [encoder endEncoding];

        result = true;
    }

    signalEvents.EnumerateForward([&](Event* event) {
        [buffer encodeSignalEvent:event->event
                            value:event->NextSignalValue()];
    });
    signalSemaphores.EnumerateForward([&](DKMap<Semaphore*, uint64_t>::Pair& pair) {
        [buffer encodeSignalEvent:pair.key->event
                            value:pair.value];
    });

    return result;
}

#pragma mark - RenderCommandEncoder
RenderCommandEncoder::RenderCommandEncoder(MTLRenderPassDescriptor* rpDesc, class CommandBuffer* b)
: commandBuffer(b)
{
	encoder = DKOBJECT_NEW Encoder(rpDesc);
}

RenderCommandEncoder::~RenderCommandEncoder()
{
}

void RenderCommandEncoder::EndEncoding()
{
	DKASSERT_DEBUG(!IsCompleted());
	encoder->commands.ShrinkToFit();
	commandBuffer->EndEncoder(this, encoder);
	encoder = NULL; // release data
}

void RenderCommandEncoder::WaitEvent(const DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<const Event*>(event));
    encoder->events.Add(const_cast<DKGpuEvent*>(event));
    encoder->waitEvents.Insert(const_cast<Event*>(static_cast<const Event*>(event)));
}

void RenderCommandEncoder::SignalEvent(const DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<const Event*>(event));
    encoder->events.Add(const_cast<DKGpuEvent*>(event));
    encoder->signalEvents.Insert(const_cast<Event*>(static_cast<const Event*>(event)));
}

void RenderCommandEncoder::WaitSemaphoreValue(const DKGpuSemaphore* semaphore, uint64_t value)
{
    DKASSERT_DEBUG(dynamic_cast<const Semaphore*>(semaphore));
    Semaphore* s = const_cast<Semaphore*>(static_cast<const Semaphore*>(semaphore));
    if (auto p = encoder->waitSemaphores.Find(s); p)
    {
        if (value > p->value)
            p->value = value;
    }
    else
    {
        encoder->waitSemaphores.Insert(s, value);
        encoder->semaphores.Add(const_cast<DKGpuSemaphore*>(semaphore));
    }
}

void RenderCommandEncoder::SignalSemaphoreValue(const DKGpuSemaphore* semaphore, uint64_t value)
{
    DKASSERT_DEBUG(dynamic_cast<const Semaphore*>(semaphore));
    Semaphore* s = const_cast<Semaphore*>(static_cast<const Semaphore*>(semaphore));
    if (auto p = encoder->signalSemaphores.Find(s); p)
    {
        if (value > p->value)
            p->value = value;
    }
    else
    {
        encoder->signalSemaphores.Insert(s, value);
        encoder->semaphores.Add(const_cast<DKGpuSemaphore*>(semaphore));
    }
}

void RenderCommandEncoder::SetResources(uint32_t set, const DKShaderBindingSet* binds)
{
    DKASSERT_DEBUG(!IsCompleted());
    DKASSERT_DEBUG(dynamic_cast<const ShaderBindingSet*>(binds));
    DKObject<ShaderBindingSet> bs = const_cast<ShaderBindingSet*>(static_cast<const ShaderBindingSet*>(binds));

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, EncodingState& state)
    {
        if (state.pipelineState)
        {
            // bind vertex resources
            bs->BindResources(set, state.pipelineState->vertexBindings.resourceBindings, 
                [&](const ShaderBindingSet::BufferObject* bufferObjects, uint32_t index, size_t numBuffers)
                {
                    id<MTLBuffer>* buffers = new id<MTLBuffer>[numBuffers];
                    NSUInteger* offsets = new NSUInteger[numBuffers];
                    for (size_t i = 0; i < numBuffers; ++i)
                    {
                        buffers[i] = bufferObjects[i].buffer->buffer;
                        offsets[i] = bufferObjects[i].offset;
                    }
                    [encoder setVertexBuffers : buffers
                        offsets : offsets
                        withRange : NSMakeRange(index, numBuffers)];
                    delete[] buffers;
                    delete[] offsets;
                },
                [&](const ShaderBindingSet::TextureObject* textureObjects, uint32_t index, size_t numTextures)
                {
                    id<MTLTexture>* textures = new id<MTLTexture>[numTextures];
                    for (size_t i = 0; i < numTextures; ++i)
                        textures[i] = textureObjects[i]->texture;
                    [encoder setVertexTextures:textures
                                    withRange:NSMakeRange(index, numTextures)];
                    delete[] textures;
                },
                [&](const ShaderBindingSet::SamplerObject* samplerObjects, uint32_t index, size_t numSamplers)
                {
                    id<MTLSamplerState>* samplers = new id<MTLSamplerState>[numSamplers];
                    for (size_t i = 0; i < numSamplers; ++i)
                        samplers[i] = samplerObjects[i]->sampler;
                    [encoder setVertexSamplerStates:samplers
                                        withRange:NSMakeRange(index, numSamplers)];
                    delete[] samplers;
                });
            // bind fragment resources
            bs->BindResources(set, state.pipelineState->fragmentBindings.resourceBindings,
                [&](const ShaderBindingSet::BufferObject* bufferObjects, uint32_t index, size_t numBuffers)
                {
                    id<MTLBuffer>* buffers = new id<MTLBuffer>[numBuffers];
                    NSUInteger* offsets = new NSUInteger[numBuffers];
                    for (size_t i = 0; i < numBuffers; ++i)
                    {
                        buffers[i] = bufferObjects[i].buffer->buffer;
                        offsets[i] = bufferObjects[i].offset;
                    }
                    [encoder setFragmentBuffers:buffers
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
                    [encoder setFragmentTextures:textures
                                        withRange:NSMakeRange(index, numTextures)];
                    delete[] textures;
                },
                [&](const ShaderBindingSet::SamplerObject* samplerObjects, uint32_t index, size_t numSamplers)
                {
                    id<MTLSamplerState>* samplers = new id<MTLSamplerState>[numSamplers];
                    for (size_t i = 0; i < numSamplers; ++i)
                        samplers[i] = samplerObjects[i]->sampler;
                    [encoder setFragmentSamplerStates:samplers
                                            withRange:NSMakeRange(index, numSamplers)];
                    delete[] samplers;
                });
        }
    });
    encoder->commands.Add(command);
}

void RenderCommandEncoder::SetViewport(const DKViewport& v)
{
	DKASSERT_DEBUG(!IsCompleted());
	MTLViewport viewport = {v.x, v.y, v.width, v.height, v.nearZ, v.farZ};
	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, EncodingState& state)
	{
		[encoder setViewport:viewport];
	});
	encoder->commands.Add(command);
}

void RenderCommandEncoder::SetRenderPipelineState(const DKRenderPipelineState* ps)
{
	DKASSERT_DEBUG(!IsCompleted());
	DKASSERT_DEBUG(dynamic_cast<const RenderPipelineState*>(ps));
	DKObject<RenderPipelineState> pipeline = const_cast<RenderPipelineState*>(static_cast<const RenderPipelineState*>(ps));

	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, EncodingState& state)
	{
		id<MTLRenderPipelineState> pipelineState = pipeline->pipelineState;
		[encoder setRenderPipelineState:pipelineState];
        [encoder setDepthStencilState:pipeline->depthStencilState];
        [encoder setDepthClipMode:pipeline->depthClipMode];
        [encoder setTriangleFillMode:pipeline->triangleFillMode];
        [encoder setFrontFacingWinding:pipeline->frontFacingWinding];
        [encoder setCullMode:pipeline->cullMode];
		state.pipelineState = pipeline;
	});
	encoder->commands.Add(command);
}

void RenderCommandEncoder::SetVertexBuffer(const DKGpuBuffer* buffer, size_t offset, uint32_t index)
{
	DKASSERT_DEBUG(!IsCompleted());
	DKASSERT_DEBUG(dynamic_cast<const Buffer*>(buffer));
	DKObject<Buffer> vertexBuffer = const_cast<Buffer*>(static_cast<const Buffer*>(buffer));

	//GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->buffer->Queue()->Device());

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, EncodingState& state)
	{
        NSUInteger bufferIndex = index;
        if (state.pipelineState)
            bufferIndex = state.pipelineState->vertexBindings.inputAttributeIndexOffset + index;
		[encoder setVertexBuffer:vertexBuffer->buffer offset:offset atIndex:bufferIndex];
	});
	encoder->commands.Add(command);
}

void RenderCommandEncoder::SetVertexBuffers(const DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count)
{
	DKASSERT_DEBUG(!IsCompleted());
	if (count > 0)
	{
		DKArray<DKObject<Buffer>> objects;
		DKArray<id<MTLBuffer>> vertexBuffers;
		DKArray<NSUInteger> bufferOffsets;
		objects.Reserve(count);
		vertexBuffers.Reserve(count);
		bufferOffsets.Reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			DKASSERT_DEBUG(dynamic_cast<const Buffer*>(buffers[i]));
			const Buffer* vb = static_cast<const Buffer*>(buffers[i]);
			objects.Add(const_cast<Buffer*>(vb));
			vertexBuffers.Add(vb->buffer);
			bufferOffsets.Add(offsets[i]);
		}
		DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, EncodingState& state)
		{
			[encoder setVertexBuffers:vertexBuffers offsets:bufferOffsets withRange:NSMakeRange(index, objects.Count())];
		});
		encoder->commands.Add(command);
	}
}

void RenderCommandEncoder::SetIndexBuffer(const DKGpuBuffer* buffer, size_t offset, DKIndexType type)
{
	DKASSERT_DEBUG(!IsCompleted());
	DKASSERT_DEBUG(dynamic_cast<const Buffer*>(buffer));
	DKObject<Buffer> indexBuffer = const_cast<Buffer*>(static_cast<const Buffer*>(buffer));
	MTLIndexType indexType;
	switch (type)
	{
		case DKIndexType::UInt16:	indexType = MTLIndexTypeUInt16; break;
		case DKIndexType::UInt32:	indexType = MTLIndexTypeUInt32; break;
		default:
			indexType = MTLIndexTypeUInt16; break;
	}
	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, EncodingState& state)
	{
		state.indexBuffer = indexBuffer;
		state.indexBufferOffset = offset;
		state.indexBufferType = indexType;
	});
	encoder->commands.Add(command);
}

void RenderCommandEncoder::Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance)
{
	DKASSERT_DEBUG(!IsCompleted());

	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, EncodingState& state)
	{
		[encoder drawPrimitives:state.pipelineState->primitiveType
					vertexStart:0
					vertexCount:numVertices
				  instanceCount:numInstances
				   baseInstance:baseInstance];
	});
	encoder->commands.Add(command);
}

void RenderCommandEncoder::DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance)
{
	DKASSERT_DEBUG(!IsCompleted());

	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, EncodingState& state)
	{
        if (vertexOffset == 0 && baseInstance == 0)
        {
            [encoder drawIndexedPrimitives:state.pipelineState->primitiveType
                                indexCount:numIndices
                                 indexType:state.indexBufferType
                               indexBuffer:state.indexBuffer->buffer
                         indexBufferOffset:indexOffset
                             instanceCount:numInstances];
        }
        else
        {
            // require feature set 'MTLFeatureSet_iOS_GPUFamily3_v1' for iOS (iPhone 6S or later)
#if TARGET_OS_IPHONE
            static BOOL supported = [this]()
            {
                //class CommandBuffer* cb = this->
                DKGraphicsDevice* dev = this->commandBuffer->Queue()->Device();
                id<MTLDevice> device = static_cast<GraphicsDevice*>(DKGraphicsDeviceInterface::Instance(dev))->device;
                return [device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1];
            }();
            if (!supported)
            {
                DKLogE("Draw Failed: Feature not supported! (MTLFeatureSet_iOS_GPUFamily3_v1 or later required)");
                return;
            }
#endif
            [encoder drawIndexedPrimitives:state.pipelineState->primitiveType
                                indexCount:numIndices
                                 indexType:state.indexBufferType
                               indexBuffer:state.indexBuffer->buffer
                         indexBufferOffset:indexOffset
                             instanceCount:numInstances
                                baseVertex:vertexOffset
                              baseInstance:baseInstance];
        }
	});
	encoder->commands.Add(command);
}
#endif //#if DKGL_ENABLE_METAL
