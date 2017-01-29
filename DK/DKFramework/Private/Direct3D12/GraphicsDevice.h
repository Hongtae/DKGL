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

				void PushReusableCommandAllocator(CommandAllocator* allocator);
				CommandAllocator* RetrieveReusableCommandAllocator(D3D12_COMMAND_LIST_TYPE);
				void PurgeAllReusableCommandAllocators(void);
				
				void PushReusableCommandList(ID3D12CommandList* list);
				ComPtr<ID3D12CommandList> RetrieveReusableCommandList(D3D12_COMMAND_LIST_TYPE);
				void PurgeAllReusableCommandLists(void);

				ComPtr<IDXGIFactory5> factory;
				ComPtr<ID3D12Device1> device;

			private:
				DKString deviceName;
				ComPtr<ID3D12CommandAllocator> dummyAllocator;

				DKSpinLock					reusableItemsLock;
				DKArray<CommandAllocator*>	reusableCommandAllocators;				
				DKArray<ComPtr<ID3D12CommandList>>	reusableCommandLists;
			};
		}
	}
}
#endif //#if DKGL_USE_DIRECT3D
