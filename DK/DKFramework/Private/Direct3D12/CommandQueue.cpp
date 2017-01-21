//
//  File: CommandQueue.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "CommandAllocator.h"
#include "GraphicsDevice.h"
#include "SwapChain.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

CommandQueue::CommandQueue(ID3D12CommandQueue* q, ID3D12Fence* f, DKGraphicsDevice* d)
	: device(d)
	, queue(q)
	, fence(f)
	, fenceCounter(0)
{
	fence->Signal(fenceCounter);
}

CommandQueue::~CommandQueue(void)
{
	// wait until queue completed
	DKCriticalSection<DKSpinLock> guard(queueLock);
	if (fence->GetCompletedValue() < fenceCounter)
	{
		HANDLE fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		fence->SetEventOnCompletion(fenceCounter, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}
}

DKObject<DKCommandBuffer> CommandQueue::CreateCommandBuffer(void)
{
	GraphicsDevice* dc = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	CommandAllocator* commandAllocator = dc->RetrieveReusableCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
	if (commandAllocator)
	{
		DKObject<CommandBuffer> buffer = DKOBJECT_NEW CommandBuffer(commandAllocator, this);
		return buffer.SafeCast<DKCommandBuffer>();
	}
	return NULL;
}

DKObject<DKSwapChain> CommandQueue::CreateSwapChain(DKWindow* window)
{
	DKObject<SwapChain> swapChain = DKOBJECT_NEW SwapChain(this, window);
	if (swapChain->Setup())
	{

		return swapChain.SafeCast<DKSwapChain>();
	}
	return NULL;
}

UINT64 CommandQueue::Enqueue(ID3D12CommandList* const* commandLists, UINT numLists, UINT64 proceedAfter)
{
	DKCriticalSection<DKSpinLock> guard(queueLock);
	fenceCounter++;
	if (fence->GetCompletedValue() < proceedAfter && proceedAfter < fenceCounter)
		queue->Wait(fence.Get(), proceedAfter);
	queue->ExecuteCommandLists(numLists, commandLists);
	queue->Signal(fence.Get(), fenceCounter);
	return fenceCounter;
}

UINT64 CommandQueue::Enqueue(ID3D12CommandList* const* commandLists, UINT numLists, ID3D12Fence* waitFence, UINT64 waitFenceValue)
{
	DKCriticalSection<DKSpinLock> guard(queueLock);
	fenceCounter++;
	if (waitFence->GetCompletedValue() < waitFenceValue)
		queue->Wait(waitFence, waitFenceValue);
	queue->ExecuteCommandLists(numLists, commandLists);
	queue->Signal(fence.Get(), fenceCounter);
	return fenceCounter;
}

UINT64 CommandQueue::EnqueuedCounterValue(void)
{
	DKCriticalSection<DKSpinLock> guard(queueLock);
	return fenceCounter;
}

#endif //#if DKGL_USE_DIRECT3D
