//
//  File: GraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include "d3d12_headers.h"

#include "../../Interface/DKGraphicsDeviceInterface.h"
#include "CommandAllocator.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class GraphicsDevice : public DKGraphicsDeviceInterface
			{
			public:
				GraphicsDevice(void);
				~GraphicsDevice(void);

				DKString DeviceName(void) const override;
				DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*) override;

				CommandAllocator* GetCommandAllocator(D3D12_COMMAND_LIST_TYPE);
				void ReleaseCommandAllocator(CommandAllocator* allocator);
				void PurgeCachedCommandAllocators(void);
				
				ComPtr<ID3D12GraphicsCommandList> GetCommandList(D3D12_COMMAND_LIST_TYPE);
				void ReleaseCommandList(ID3D12GraphicsCommandList* list);
				void PurgeCachedCommandLists(void);

				D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE);
				void ReleaseDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE, D3D12_CPU_DESCRIPTOR_HANDLE);

				void AddFenceCompletionHandler(ID3D12Fence*, UINT64, DKObject<DKOperation>, bool useEventLoop);

				ComPtr<IDXGIFactory5> factory;
				ComPtr<ID3D12Device1> device;

			private:
				DKString deviceName;
				ComPtr<ID3D12CommandAllocator> dummyAllocator;

				// command lists / command allocators
				DKSpinLock					reusableItemsLock;
				DKArray<CommandAllocator*>	reusableCommandAllocators;				
				DKArray<ComPtr<ID3D12GraphicsCommandList>>	reusableCommandLists;

				// descriptor heaps
				struct DescriptorHeap
				{
					ComPtr<ID3D12DescriptorHeap> heap;
					DKBitArray<> inUseBits;
				};
				DKSpinLock descriptorHeapLock;
				DKArray<DescriptorHeap> descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
				UINT descriptorHandleIncrementSizes[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
				UINT descriptorHeapInitialCapacities[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];


				struct PendingFenceCallback
				{
					ComPtr<ID3D12Fence> fence;
					UINT64 fenceValue;
					DKObject<DKOperation> callback;
					DKThread::ThreadId threadId;
				};
				DKArray<PendingFenceCallback> pendingFenceCallbacks;
				DKCondition fenceCompletionCond;
				bool fenceCompletionThreadRunning;
				DKObject<DKThread> fenceCompletionThread;
				void FenceCompletionCallbackThreadProc(void);
				HANDLE fenceCompletionEvent;
			};
		}
	}
}
#endif //#if DKGL_USE_DIRECT3D
