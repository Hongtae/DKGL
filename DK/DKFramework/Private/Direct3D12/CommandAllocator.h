//
//  File: CommandAllocator.h
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


namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class CommandAllocator
			{
			public:
				CommandAllocator(ID3D12CommandAllocator*, D3D12_COMMAND_LIST_TYPE);
				~CommandAllocator(void);

				ComPtr<ID3D12CommandAllocator> allocator;
				D3D12_COMMAND_LIST_TYPE type;

				void SetPendingState(ID3D12Fence* fence, UINT64 enqueued);
				bool Reset(void);
				bool IsCompleted(void);
				bool WaitUntilCompleted(DWORD timeout);

				UINT64 PendingCounter(void) { return enqueuedCounter; }

			private:
				// for enqueued allocator
				UINT64 enqueuedCounter;
				ComPtr<ID3D12Fence> fence;
				HANDLE fenceEvent;
			};
		}
	}
}

#endif //#if DKGL_USE_DIRECT3D
