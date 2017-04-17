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
#include "ComputeCommandEncoder.h"
#include "BlitCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;


CommandBuffer::CommandBuffer(CommandAllocator* a, DKCommandQueue* q)
	: commandAllocator(a)
	, queue(q)
{
}

CommandBuffer::~CommandBuffer(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->Device());
	dev->ReleaseCommandAllocator(this->commandAllocator);

	for (ComPtr<ID3D12GraphicsCommandList>& list : commandLists)
	{
		dev->ReleaseCommandList(list.Get());
	}
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(const DKRenderPassDescriptor& desc)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->Device());
	ComPtr<ID3D12GraphicsCommandList> cm = dev->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	cm->Reset(commandAllocator->allocator.Get(), nullptr);

	DKObject<RenderCommandEncoder> encoder = DKOBJECT_NEW RenderCommandEncoder(cm.Get(), this, desc);
	return encoder.SafeCast<DKRenderCommandEncoder>();
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->Device());
	ComPtr<ID3D12GraphicsCommandList> cm = dev->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	cm->Reset(commandAllocator->allocator.Get(), nullptr);

	DKObject<ComputeCommandEncoder> encoder = DKOBJECT_NEW ComputeCommandEncoder(cm.Get(), this);
	return encoder.SafeCast<DKComputeCommandEncoder>();
}

DKObject<DKBlitCommandEncoder> CommandBuffer::CreateBlitCommandEncoder(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->Device());
	ComPtr<ID3D12GraphicsCommandList> cm = dev->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	cm->Reset(commandAllocator->allocator.Get(), nullptr);

	DKObject<BlitCommandEncoder> encoder = DKOBJECT_NEW BlitCommandEncoder(cm.Get(), this);
	return encoder.SafeCast<DKBlitCommandEncoder>();
}

void CommandBuffer::FinishCommandList(ID3D12GraphicsCommandList* list)
{
	commandLists.Add(list);
}

bool CommandBuffer::Commit(void)
{
	if (commandLists.Count() > 0 && commandAllocator->IsCompleted())
	{
		CommandQueue* commandQueue = this->queue.StaticCast<CommandQueue>();
		ComPtr<ID3D12GraphicsCommandList>* lists = this->commandLists;
		UINT numLists = static_cast<UINT>(this->commandLists.Count());

		static_assert(sizeof(ComPtr<ID3D12GraphicsCommandList>) == sizeof(ID3D12CommandList*), "");

		UINT64 enqueued = commandQueue->Enqueue(reinterpret_cast<ID3D12CommandList * const *>(lists), numLists);
		this->commandAllocator->SetPendingState(commandQueue->fence.Get(), enqueued);
		return true;
	}
	return false;
}

bool CommandBuffer::WaitUntilCompleted(void)
{
	return commandAllocator->WaitUntilCompleted(INFINITE);
}

#endif //#if DKGL_USE_DIRECT3D
