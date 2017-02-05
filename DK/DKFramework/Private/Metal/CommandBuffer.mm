//
//  File: CommandBuffer.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_METAL

#include "CommandBuffer.h"
#include "RenderCommandEncoder.h"
#include "RenderTarget.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

CommandBuffer::CommandBuffer(id<MTLCommandBuffer> cb, DKCommandQueue* q)
: buffer(nil)
, queue(q)
, committed(false)
{
	buffer = [cb retain];
}

CommandBuffer::~CommandBuffer(void)
{
	[buffer autorelease];
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(const DKRenderPassDescriptor& rp)
{
	MTLRenderPassDescriptor* rpDesc = [MTLRenderPassDescriptor renderPassDescriptor];

	auto CopyProperties = [](const DKRenderPassAttachmentDescriptor* src, MTLRenderPassAttachmentDescriptor* dst)
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
			CopyProperties(&ca, colorAttachment);

		}
		[rpDesc.colorAttachments setObject:colorAttachment atIndexedSubscript:i];
	 }

	if (rp.depthStencilAttachment.renderTarget)
	{
		const RenderTarget* rt = rp.depthStencilAttachment.renderTarget.SafeCast<RenderTarget>();
		if (rt)
		{
		}
	}

	id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:rpDesc];
	if (encoder)
	{
		DKObject<RenderCommandEncoder> rce = DKOBJECT_NEW RenderCommandEncoder(this, encoder);
		return rce.SafeCast<DKRenderCommandEncoder>();
	}
	return NULL;
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder(void)
{
	return NULL;
}

DKObject<DKBlitCommandEncoder> CommandBuffer::CreateBlitCommandEncoder(void)
{
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

#endif //#if DKGL_USE_METAL
