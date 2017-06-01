//
//  File: RenderCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12

#include "RenderCommandEncoder.h"
#include "RenderTarget.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

RenderCommandEncoder::RenderCommandEncoder(ID3D12GraphicsCommandList* cm, CommandBuffer* cb,  const DKRenderPassDescriptor& desc)
	: commandList(cm)
	, commandBuffer(cb)
	, renderPassDesc(desc)
{
	DKASSERT_DEBUG(commandList != nullptr);

	size_t numAttachments = renderPassDesc.colorAttachments.Count();
	DKArray<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
	rtvHandles.Reserve(numAttachments);
	RenderTarget* depthStencilRT = nullptr;

	if (renderPassDesc.depthStencilAttachment.renderTarget)
		numAttachments += 1;

	rtBarriers.Reserve(numAttachments);

	for (DKRenderPassColorAttachmentDescriptor& colorAttachment : renderPassDesc.colorAttachments)
	{
		RenderTarget* rt = colorAttachment.renderTarget.SafeCast<RenderTarget>();
		if (rt)
		{
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				rt->resource.Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			rtBarriers.Add(barrier);

			rtvHandles.Add(rt->rtvHandle);
		}
	}
	if (renderPassDesc.depthStencilAttachment.renderTarget)
	{
		RenderTarget* rt = renderPassDesc.depthStencilAttachment.renderTarget.SafeCast<RenderTarget>();
		if (rt)
		{
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				rt->resource.Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			rtBarriers.Add(barrier);

			depthStencilRT = rt;
		}
	}

	// setup root-signature
	// setup pipeline object!

	if (rtBarriers.Count() > 0)
	{
		commandList->ResourceBarrier(rtBarriers.Count(), rtBarriers);
	}
	if (rtvHandles.Count() > 0 || depthStencilRT)
	{
		commandList->OMSetRenderTargets(rtvHandles.Count(), rtvHandles, FALSE, 
										depthStencilRT ? &depthStencilRT->rtvHandle : nullptr);

	}
	for (DKRenderPassColorAttachmentDescriptor& colorAttachment : renderPassDesc.colorAttachments)
	{
		if (colorAttachment.loadAction == DKRenderPassAttachmentDescriptor::LoadActionClear)
		{
			RenderTarget* rt = colorAttachment.renderTarget.SafeCast<RenderTarget>();
			if (rt)
				commandList->ClearRenderTargetView(rt->rtvHandle, colorAttachment.clearColor.val, 0, nullptr);
		}
	}
	if (renderPassDesc.depthStencilAttachment.renderTarget)
	{
		if (renderPassDesc.depthStencilAttachment.loadAction == DKRenderPassAttachmentDescriptor::LoadActionClear)
		{
			RenderTarget* rt = renderPassDesc.depthStencilAttachment.renderTarget.SafeCast<RenderTarget>();
			if (rt)
				commandList->ClearDepthStencilView(rt->rtvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
												   renderPassDesc.depthStencilAttachment.clearDepth,
												   renderPassDesc.depthStencilAttachment.clearStencil,
												   0, nullptr);
		}
	}
}

RenderCommandEncoder::~RenderCommandEncoder(void)
{
}

void RenderCommandEncoder::EndEncoding(void)
{
	if (commandList)
	{
		if (rtBarriers.Count() > 0)
		{
			for (D3D12_RESOURCE_BARRIER& barrier : rtBarriers)
			{
				DKASSERT_DEBUG(barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
				D3D12_RESOURCE_STATES after = barrier.Transition.StateAfter;
				D3D12_RESOURCE_STATES before = barrier.Transition.StateBefore;
				barrier.Transition.StateAfter = before;
				barrier.Transition.StateBefore = after;
			}
			commandList->ResourceBarrier(rtBarriers.Count(), rtBarriers);
		}
		commandList->Close();

		commandBuffer->FinishCommandList(commandList.Get());
		commandList = nullptr;
	}
}

DKCommandBuffer* RenderCommandEncoder::Buffer(void)
{
	return commandBuffer;
}

#endif //#if DKGL_ENABLE_DIRECT3D12
