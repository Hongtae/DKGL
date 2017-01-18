//
//  File: GraphicsDevice.h
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

#include "../../Interface/DKGraphicsDeviceInterface.h"
#include "CommandAllocator.h"
#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			DKGraphicsDeviceInterface* CreateInterface(void);

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

			private:
				DKString deviceName;
				ComPtr<ID3D12Device1> device;
				ComPtr<ID3D12CommandAllocator> dummyAllocator;

				DKSpinLock					reusableItemsLock;
				DKArray<CommandAllocator*>	reusableCommandAllocators;				
				DKArray<ComPtr<ID3D12CommandList>>	reusableCommandLists;
			};
		}
	}
}
#endif //#if DKGL_USE_DIRECT3D
