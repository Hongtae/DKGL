//
//  File: CommandBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D

#include "CommandAllocator.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

CommandAllocator::CommandAllocator(ID3D12CommandAllocator* a, D3D12_COMMAND_LIST_TYPE t)
	: allocator(a)
	, type(t)
	, enqueuedCounter(0)
{
}

CommandAllocator::~CommandAllocator(void)
{
}

void CommandAllocator::SetPendingState(ID3D12Fence* f, UINT64 v)
{
	DKASSERT_DEBUG(this->fence == nullptr);
	DKASSERT_DEBUG(f != nullptr);

	fence = f;
	enqueuedCounter = v;
}

bool CommandAllocator::Reset(void)
{
	HRESULT hr = allocator->Reset();
	if (SUCCEEDED(allocator->Reset()))
	{
		this->fence = nullptr;
		return true;
	}
	
	DKLog("ID3D12CommandAllocator::Reset() failed!");
	return false;
}

bool CommandAllocator::IsCompleted(void)
{
	if (this->fence)
	{
		return this->fence->GetCompletedValue() >= enqueuedCounter;
	}
	return true;
}

#endif //#if DKGL_USE_DIRECT3D
