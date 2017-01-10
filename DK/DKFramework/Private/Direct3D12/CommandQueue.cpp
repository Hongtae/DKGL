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

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

CommandQueue::~CommandQueue(void)
{
}

DKObject<DKCommandBuffer> CommandQueue::CreateCommandBuffer(void)
{
	ComPtr<ID3D12Device1> dev;
	if (FAILED(this->queue->GetDevice(IID_PPV_ARGS(&dev))))
	{
		DKLog("ERROR: ID3D12Device1::GetDevice() failed");
		return NULL;
	}

	D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ComPtr<ID3D12CommandAllocator> commandAllocator;
	if (FAILED(dev->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator))))
	{
		DKLog("ERROR: ID3D12Device1::CreateCommandAllocator() failed");
		return NULL;
	}

	DKObject<CommandBuffer> buffer = DKOBJECT_NEW CommandBuffer();
	buffer->commandAllocator = commandAllocator;
	buffer->type = type;
	buffer->queue = this;
	return buffer.SafeCast<DKCommandBuffer>();
}

#endif //#if DKGL_USE_DIRECT3D
