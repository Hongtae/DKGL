//
//  File: CommandQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKCommandQueue.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class CommandQueue : public DKCommandQueue
			{
			public:
				CommandQueue(ID3D12CommandQueue*, ID3D12Fence*, DKGraphicsDevice*);
				~CommandQueue();

				DKObject<DKCommandBuffer> CreateCommandBuffer() override;
				DKObject<DKSwapChain> CreateSwapChain(DKWindow*) override;
				DKGraphicsDevice* Device() override { return device; }

				// enqueue command lists and return counter for fence value. (required for waiting)
				UINT64 Enqueue(ID3D12CommandList* const* commandLists, UINT numLists, UINT64 proceedAfter = 0);
				// enqueue command lists and waiting for fence before execution.
				UINT64 Enqueue(ID3D12CommandList* const* commandLists, UINT numLists, ID3D12Fence* waitFence, UINT64 waitFenceValue);

				UINT64 EnqueuedCounterValue();


				ComPtr<ID3D12CommandQueue> queue;
				ComPtr<ID3D12Fence> fence;

			private:
				DKObject<DKGraphicsDevice> device;

				UINT64 fenceCounter;
				DKSpinLock queueLock;
			};
		}
	}
}
#endif //#if DKGL_ENABLE_DIRECT3D12
