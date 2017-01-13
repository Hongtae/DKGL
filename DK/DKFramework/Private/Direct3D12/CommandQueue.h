//
//  File: CommandQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include <wrl.h>
#include <D3D12.h>
#include <dxgi1_5.h>
using Microsoft::WRL::ComPtr;

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
				~CommandQueue(void);

				DKObject<DKCommandBuffer> CreateCommandBuffer(void) override;
				DKGraphicsDevice* Device(void) override { return device; }

				// enqueue command lists and return counter for fence value. (required for waiting)
				UINT64 Enqueue(ID3D12CommandList* const* commandLists, UINT numLists, UINT64 proceedAfter = 0);
				// enqueue command lists and waiting for fence before execution.
				UINT64 Enqueue(ID3D12CommandList* const* commandLists, UINT numLists, ID3D12Fence* waitFence, UINT64 waitFenceValue);

				UINT64 EnqueuedCounterValue(void);

				ID3D12Fence* Fence(void);

			private:
				DKObject<DKGraphicsDevice> device;
				ComPtr<ID3D12CommandQueue> queue;

				ComPtr<ID3D12Fence> fence;
				UINT64 fenceCounter;
				DKSpinLock queueLock;
			};
		}
	}
}
#endif //#if DKGL_USE_DIRECT3D
