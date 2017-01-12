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

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

CommandQueue::CommandQueue(ID3D12CommandQueue* q, ID3D12Fence* f, DKGraphicsDevice* d)
	: device(d)
	, queue(q)
	, fence(f)
{
	fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
}

CommandQueue::~CommandQueue(void)
{
	CloseHandle(fenceEvent);
}

DKObject<DKCommandBuffer> CommandQueue::CreateCommandBuffer(void)
{
	GraphicsDevice* dc = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	CommandAllocator* commandAllocator = dc->DequeueReusableCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
	if (commandAllocator)
	{
		DKObject<CommandBuffer> buffer = DKOBJECT_NEW CommandBuffer(commandAllocator, this);
		return buffer.SafeCast<DKCommandBuffer>();
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

UINT64 CommandQueue::CompletedFenceValue(void)
{
	return fence->GetCompletedValue();
}

ID3D12Fence* CommandQueue::Fence(void)
{
	return fence.Get();
}

bool CommandQueue::WaitFence(UINT64 value, DWORD timeout)
{
	if (fence->GetCompletedValue() < value)
	{
		fence->SetEventOnCompletion(value, fenceEvent);
		return WaitForSingleObject(fenceEvent, timeout) == WAIT_OBJECT_0;
	}
	return true;
}

#endif //#if DKGL_USE_DIRECT3D
