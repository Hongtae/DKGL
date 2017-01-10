//
//  File: CommandBuffer.h
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

#include "../../DKCommandBuffer.h"
#include "../../DKCommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class CommandBuffer : public DKCommandBuffer
			{
			public:
				~CommandBuffer(void);

				DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(DKRenderPassDescriptor*) override;
				DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder(void) override;
				DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder(void) override;

				void Commit(void) override;

				DKObject<DKCommandQueue> queue;

				DKArray<ID3D12CommandList*> closedCommandLists;	// finished encoded, not executed yet.
				ComPtr<ID3D12CommandAllocator> commandAllocator;
				D3D12_COMMAND_LIST_TYPE type;
			};
		}
	}
}

#endif //#if DKGL_USE_DIRECT3D
