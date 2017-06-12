//
//  File: CommandBuffer.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include <TargetConditionals.h>

#include "CommandBuffer.h"
#include "RenderTarget.h"
#include "RenderCommandEncoder.h"
#include "ComputeCommandEncoder.h"
#include "BlitCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

CommandBuffer::CommandBuffer(id<MTLCommandBuffer> cb, DKCommandQueue* q)
: buffer(nil)
, queue(q)
, activeEncoder(NULL)
, committed(false)
{
	buffer = [cb retain];
	DKASSERT_DEBUG(buffer != nil);
	DKASSERT_DEBUG(buffer.status == MTLCommandBufferStatusNotEnqueued);
}

CommandBuffer::~CommandBuffer(void)
{
	[buffer release];
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(const DKRenderPassDescriptor& rp)
{
	if (committed)
	{
		DKLogE("ERROR: DKCommandBuffer(%p) has already been committed and cannot be reused.", this);
		return NULL;
	}
	if (activeEncoder)
	{
		DKLogE("ERROR: DKCommandBuffer(%p) has active encoder. Close active encoder first.", this);
		return NULL;
	}

	MTLRenderPassDescriptor* rpDesc = [MTLRenderPassDescriptor renderPassDescriptor];

	auto CopyAttachmentProperties = [](const DKRenderPassAttachmentDescriptor* src, MTLRenderPassAttachmentDescriptor* dst)
	{
		switch (src->loadAction)
		{
			case DKRenderPassAttachmentDescriptor::LoadActionDontCare:
				dst.loadAction = MTLLoadActionDontCare;	break;
			case DKRenderPassAttachmentDescriptor::LoadActionLoad:
				dst.loadAction = MTLLoadActionLoad;		break;
			case DKRenderPassAttachmentDescriptor::LoadActionClear:
				dst.loadAction = MTLLoadActionClear;	break;
		}
		switch (src->storeAction)
		{
			case DKRenderPassAttachmentDescriptor::StoreActionDontCare:
				dst.storeAction = MTLStoreActionDontCare;	break;
			case DKRenderPassAttachmentDescriptor::StoreActionStore:
				dst.storeAction = MTLStoreActionStore;		break;
		}
	};

	for (size_t i = 0, c = rp.colorAttachments.Count(); i < c ; ++i)
	{
		const DKRenderPassColorAttachmentDescriptor& ca = rp.colorAttachments.Value(i);
		MTLRenderPassColorAttachmentDescriptor* colorAttachment = nil;

		const RenderTarget* rt = ca.renderTarget.SafeCast<RenderTarget>();
		if (rt)
		{
			colorAttachment = [[[MTLRenderPassColorAttachmentDescriptor alloc] init] autorelease];
			colorAttachment.texture = rt->texture;
			colorAttachment.clearColor = { ca.clearColor.r, ca.clearColor.g, ca.clearColor.b, ca.clearColor.a };
			CopyAttachmentProperties(&ca, colorAttachment);

		}
		[rpDesc.colorAttachments setObject:colorAttachment atIndexedSubscript:i];
	 }

	if (rp.depthStencilAttachment.renderTarget)
	{
		const RenderTarget* rt = rp.depthStencilAttachment.renderTarget.SafeCast<RenderTarget>();
		if (rt)
		{
			bool hasDepth = false;
			bool hasStencil = false;

			switch (rt->texture.pixelFormat)
			{
#if TARGET_OS_IPHONE
#else
				case MTLPixelFormatDepth16Unorm:
					hasDepth = true;
					break;
				case MTLPixelFormatDepth24Unorm_Stencil8:
					hasDepth = hasStencil = true;
					break;
				case MTLPixelFormatX24_Stencil8:
					hasStencil = true;
					break;
#endif
				case MTLPixelFormatDepth32Float:
					hasDepth = true;
					break;
				case MTLPixelFormatStencil8:
					hasStencil = true;
					break;
				case MTLPixelFormatDepth32Float_Stencil8:
					hasDepth = hasStencil = true;
					break;
				case MTLPixelFormatX32_Stencil8:
					hasStencil = true;
					break;
			}
			if (hasDepth)
			{
				MTLRenderPassDepthAttachmentDescriptor* depthAttachment = [[[MTLRenderPassDepthAttachmentDescriptor alloc] init] autorelease];
				depthAttachment.texture = rt->texture;
				depthAttachment.clearDepth = rp.depthStencilAttachment.clearDepth;
				CopyAttachmentProperties(&rp.depthStencilAttachment, depthAttachment);
				rpDesc.depthAttachment = depthAttachment;
			}
			if (hasStencil)
			{
				MTLRenderPassStencilAttachmentDescriptor* stencilAttachment = [[[MTLRenderPassStencilAttachmentDescriptor alloc] init] autorelease];
				stencilAttachment.texture = rt->texture;
				stencilAttachment.clearStencil = rp.depthStencilAttachment.clearStencil;
				CopyAttachmentProperties(&rp.depthStencilAttachment, stencilAttachment);
				rpDesc.stencilAttachment = stencilAttachment;
			}
		}
	}

	id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:rpDesc];
	if (encoder)
	{
		DKObject<RenderCommandEncoder> enc = DKOBJECT_NEW RenderCommandEncoder(encoder, this);
		DKObject<DKRenderCommandEncoder> r = enc.SafeCast<DKRenderCommandEncoder>();
		activeEncoder = r;
		return r;
	}
	return NULL;
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder(void)
{
	if (committed)
	{
		DKLogE("ERROR: DKCommandBuffer(%p) has already been committed and cannot be reused.", this);
		return NULL;
	}
	if (activeEncoder)
	{
		DKLogE("ERROR: DKCommandBuffer(%p) has active encoder. Close active encoder first.", this);
		return NULL;
	}

	id<MTLComputeCommandEncoder> encoder = [buffer computeCommandEncoder];
	if (encoder)
	{
		DKObject<ComputeCommandEncoder> enc = DKOBJECT_NEW ComputeCommandEncoder(encoder, this);
		DKObject<DKComputeCommandEncoder> r = enc.SafeCast<DKComputeCommandEncoder>();
		activeEncoder = r;
		return r;
	}
	return NULL;
}

DKObject<DKBlitCommandEncoder> CommandBuffer::CreateBlitCommandEncoder(void)
{
	if (committed)
	{
		DKLogE("ERROR: DKCommandBuffer(%p) has already been committed and cannot be reused.", this);
		return NULL;
	}
	if (activeEncoder)
	{
		DKLogE("ERROR: DKCommandBuffer(%p) has active encoder. Close active encoder first.", this);
		return NULL;
	}

	id<MTLBlitCommandEncoder> encoder = [buffer blitCommandEncoder];
	if (encoder)
	{
		DKObject<BlitCommandEncoder> enc = DKOBJECT_NEW BlitCommandEncoder(encoder, this);
		DKObject<DKBlitCommandEncoder> r = enc.SafeCast<DKBlitCommandEncoder>();
		activeEncoder = r;
		return r;
	}
	return NULL;
}

bool CommandBuffer::Commit(void)
{
	if (!committed)
	{
		[buffer commit];
		committed = true;
		return true;
	}
	return false;
}

bool CommandBuffer::WaitUntilCompleted(void)
{
	if (committed)
	{
		[buffer waitUntilCompleted];
	}
	return false;
}

void CommandBuffer::EndEncoder(DKCommandEncoder* enc)
{
	DKASSERT_DEBUG(activeEncoder == enc);
	activeEncoder = NULL;
}

#endif //#if DKGL_ENABLE_METAL
