//
//  File: CommandBuffer.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include <TargetConditionals.h>
#include "CommandBuffer.h"
#include "CommandQueue.h"
#include "RenderCommandEncoder.h"
#include "ComputeCommandEncoder.h"
#include "CopyCommandEncoder.h"
#include "Texture.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

CommandBuffer::CommandBuffer(DKCommandQueue* q)
: queue(q)
{
	DKASSERT_DEBUG(queue.SafeCast<CommandQueue>());
}

CommandBuffer::~CommandBuffer()
{
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(const DKRenderPassDescriptor& rp)
{
	DKObject<DKRenderCommandEncoder> r = NULL;
	@autoreleasepool
	{
		MTLRenderPassDescriptor* rpDesc = [MTLRenderPassDescriptor renderPassDescriptor];

		auto CopyAttachmentProperties = [](const DKRenderPassAttachmentDescriptor* src, MTLRenderPassAttachmentDescriptor* dst)
		{
			switch (src->loadAction)
			{
				case DKRenderPassAttachmentDescriptor::LoadActionDontCare:
					dst.loadAction = MTLLoadActionDontCare;
                    break;
				case DKRenderPassAttachmentDescriptor::LoadActionLoad:
					dst.loadAction = MTLLoadActionLoad;
                    break;
				case DKRenderPassAttachmentDescriptor::LoadActionClear:
					dst.loadAction = MTLLoadActionClear;
                    break;
			}
			switch (src->storeAction)
			{
				case DKRenderPassAttachmentDescriptor::StoreActionDontCare:
					dst.storeAction = MTLStoreActionDontCare;
                    break;
				case DKRenderPassAttachmentDescriptor::StoreActionStore:
					dst.storeAction = MTLStoreActionStore;
                    break;
			}
		};

		for (size_t i = 0, c = rp.colorAttachments.Count(); i < c ; ++i)
		{
			const DKRenderPassColorAttachmentDescriptor& ca = rp.colorAttachments.Value(i);
			MTLRenderPassColorAttachmentDescriptor* colorAttachment = nil;

			const Texture* rt = ca.renderTarget.SafeCast<Texture>();
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
			const Texture* rt = rp.depthStencilAttachment.renderTarget.SafeCast<Texture>();
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

		DKObject<RenderCommandEncoder> enc = DKOBJECT_NEW RenderCommandEncoder(rpDesc, this);
		r = enc.SafeCast<DKRenderCommandEncoder>();
	}
	return r;
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder()
{
	DKObject<ComputeCommandEncoder> enc = DKOBJECT_NEW ComputeCommandEncoder(this);
	DKObject<DKComputeCommandEncoder> r = enc.SafeCast<DKComputeCommandEncoder>();
	return r;
}

DKObject<DKCopyCommandEncoder> CommandBuffer::CreateCopyCommandEncoder()
{
	DKObject<CopyCommandEncoder> enc = DKOBJECT_NEW CopyCommandEncoder(this);
	DKObject<DKCopyCommandEncoder> r = enc.SafeCast<DKCopyCommandEncoder>();
	return r;
}

bool CommandBuffer::Commit()
{
	@autoreleasepool
	{
		id<MTLCommandQueue> q = queue.StaticCast<CommandQueue>()->queue;
		id<MTLCommandBuffer> buffer = [q commandBuffer];

		for (CommandEncoder* enc : encoders)
		{
			if (!enc->Encode(buffer))
				return false;
		}
		[buffer commit];
	}
	return true;
}

void CommandBuffer::EndEncoder(DKCommandEncoder* enc, CommandEncoder* encoder)
{
	DKASSERT_DEBUG(encoder);

	encoders.Add(encoder);
}

#endif //#if DKGL_ENABLE_METAL
