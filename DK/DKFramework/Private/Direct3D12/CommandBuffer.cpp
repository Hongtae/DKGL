//
//  File: CommandBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "CommandBuffer.h"
#include "CommandQueue.h"
#include "GraphicsDevice.h"
#include "RenderCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;


CommandBuffer::CommandBuffer(CommandAllocator* a, DKCommandQueue* q)
	: commandAllocator(a)
	, queue(q)
{
}

CommandBuffer::~CommandBuffer(void)
{
	GraphicsDevice* dc = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->Device());
	dc->EnqueueReusableCommandAllocator(this->commandAllocator);

	for (ComPtr<ID3D12CommandList>& list : commandLists)
	{
		dc->EnqueueReusableCommandList(list.Get());
	}
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(DKRenderPassDescriptor*)
{
	GraphicsDevice* dc = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->Device());
	ComPtr<ID3D12CommandList> cm = dc->DequeueReusableCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	DKObject<RenderCommandEncoder> encoder = DKOBJECT_NEW RenderCommandEncoder(cm.Get());
	return encoder.SafeCast<DKRenderCommandEncoder>();
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
	if (commandLists.Count() > 0 && commandAllocator->IsCompleted())
	{
		CommandQueue* commandQueue = this->queue.StaticCast<CommandQueue>();
		ComPtr<ID3D12CommandList>* lists = this->commandLists;
		UINT numLists = this->commandLists.Count();

		static_assert(sizeof(ComPtr<ID3D12CommandList>) == sizeof(ID3D12CommandList*), "");

		UINT64 enqueued = commandQueue->Enqueue(reinterpret_cast<ID3D12CommandList * const *>(lists), numLists);
		this->commandAllocator->SetPendingState(commandQueue->Fence(), enqueued);
		return true;
	}
	return false;
}

bool CommandBuffer::WaitUntilCompleted(void)
{
	return commandAllocator->WaitUntilCompleted(INFINITE);
}

#endif //#if DKGL_USE_DIRECT3D
