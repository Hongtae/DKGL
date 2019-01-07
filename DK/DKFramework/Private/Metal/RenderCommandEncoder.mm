//
//  File: RenderCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL

#include "RenderCommandEncoder.h"
#include "RenderPipelineState.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

RenderCommandEncoder::RenderCommandEncoder(MTLRenderPassDescriptor* rpDesc, CommandBuffer* b)
: buffer(b)
{
	reusableEncoder = DKOBJECT_NEW ReusableEncoder();
	reusableEncoder->renderPassDescriptor = [rpDesc retain];
	reusableEncoder->encoderCommands.Reserve(ReusableCommandEncoder::InitialNumberOfCommands);
}

RenderCommandEncoder::~RenderCommandEncoder()
{
}

void RenderCommandEncoder::EndEncoding()
{
	DKASSERT_DEBUG(!IsCompleted());
	reusableEncoder->encoderCommands.ShrinkToFit();
	buffer->EndEncoder(this, reusableEncoder);
	reusableEncoder = NULL; // release data
}

void RenderCommandEncoder::SetResources(uint32_t set, DKShaderBindingSet* binds)
{
}

void RenderCommandEncoder::SetViewport(const DKViewport& v)
{
	DKASSERT_DEBUG(!IsCompleted());
	MTLViewport viewport = {v.x, v.y, v.width, v.height, v.nearZ, v.farZ};
	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, Resources& res)
	{
		[encoder setViewport:viewport];
	});
	reusableEncoder->encoderCommands.Add(command);
}

void RenderCommandEncoder::SetRenderPipelineState(DKRenderPipelineState* ps)
{
	DKASSERT_DEBUG(!IsCompleted());
	DKASSERT_DEBUG(dynamic_cast<RenderPipelineState*>(ps));
	DKObject<RenderPipelineState> pipeline = static_cast<RenderPipelineState*>(ps);

	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, Resources& res)
	{
		id<MTLRenderPipelineState> pipelineState = pipeline->pipelineState;
		[encoder setRenderPipelineState:pipelineState];
		res.pipelineState = pipeline;
	});
	reusableEncoder->encoderCommands.Add(command);
}

void RenderCommandEncoder::SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index)
{
	DKASSERT_DEBUG(!IsCompleted());
	DKASSERT_DEBUG(dynamic_cast<class Buffer*>(buffer));
	DKObject<class Buffer> vertexBuffer = static_cast<class Buffer*>(buffer);

	//GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->buffer->Queue()->Device());

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, Resources& res)
	{
        NSUInteger bufferIndex = index;
        if (res.pipelineState)
            bufferIndex = res.pipelineState->vertexBindings.inputAttributeIndexOffset + index;
		[encoder setVertexBuffer:vertexBuffer->buffer offset:offset atIndex:bufferIndex];
	});
	reusableEncoder->encoderCommands.Add(command);
}

void RenderCommandEncoder::SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count)
{
	DKASSERT_DEBUG(!IsCompleted());
	if (count > 0)
	{
		DKArray<DKObject<class Buffer>> objects;
		DKArray<id<MTLBuffer>> vertexBuffers;
		DKArray<NSUInteger> bufferOffsets;
		objects.Reserve(count);
		vertexBuffers.Reserve(count);
		bufferOffsets.Reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			DKASSERT_DEBUG(dynamic_cast<class Buffer*>(buffers[i]));
			DKObject<class Buffer> vb = static_cast<class Buffer*>(buffers[i]);
			objects.Add(vb);
			vertexBuffers.Add(vb->buffer);
			bufferOffsets.Add(offsets[i]);
		}
		DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, Resources& res)
		{
			[encoder setVertexBuffers:vertexBuffers offsets:bufferOffsets withRange:NSMakeRange(index, objects.Count())];
		});
		reusableEncoder->encoderCommands.Add(command);
	}
}

void RenderCommandEncoder::SetIndexBuffer(DKGpuBuffer* buffer, size_t offset, DKIndexType type)
{
	DKASSERT_DEBUG(!IsCompleted());
	DKASSERT_DEBUG(dynamic_cast<class Buffer*>(buffer));
	DKObject<class Buffer> indexBuffer = static_cast<class Buffer*>(buffer);
	MTLIndexType indexType;
	switch (type)
	{
		case DKIndexType::UInt16:	indexType = MTLIndexTypeUInt16; break;
		case DKIndexType::UInt32:	indexType = MTLIndexTypeUInt32; break;
		default:
			indexType = MTLIndexTypeUInt16; break;
	}
	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, Resources& res)
	{
		res.indexBuffer = indexBuffer;
		res.indexBufferOffset = offset;
		res.indexBufferType = indexType;
	});
	reusableEncoder->encoderCommands.Add(command);
}

void RenderCommandEncoder::Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance)
{
	DKASSERT_DEBUG(!IsCompleted());

	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, Resources& res)
	{
		[encoder drawPrimitives:res.pipelineState->primitiveType
					vertexStart:0
					vertexCount:numVertices
				  instanceCount:numInstances
				   baseInstance:baseInstance];
	});
	reusableEncoder->encoderCommands.Add(command);
}

void RenderCommandEncoder::DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance)
{
	DKASSERT_DEBUG(!IsCompleted());

	DKObject<EncoderCommand> command = DKFunction([=](id<MTLRenderCommandEncoder> encoder, Resources& res)
	{
		[encoder drawIndexedPrimitives:res.pipelineState->primitiveType
							indexCount:numIndices
							 indexType:res.indexBufferType
						   indexBuffer:res.indexBuffer->buffer
					 indexBufferOffset:indexOffset
						 instanceCount:numInstances
							baseVertex:vertexOffset
						  baseInstance:baseInstance];
	});
	reusableEncoder->encoderCommands.Add(command);
}
#endif //#if DKGL_ENABLE_METAL
