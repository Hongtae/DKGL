//
//  File: CommandBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKCommandBuffer.h"
#include "../../DKCommandQueue.h"
#include "CommandAllocator.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class CommandBuffer : public DKCommandBuffer
			{
			public:
				CommandBuffer(CommandAllocator*, DKCommandQueue*);
				~CommandBuffer();

				DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(const DKRenderPassDescriptor&) override;
				DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder() override;
				DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder() override;

				bool Commit() override;
				bool WaitUntilCompleted() override;

				void FinishCommandList(ID3D12GraphicsCommandList*);

				DKCommandQueue* Queue() override { return queue; };
				
				DKObject<DKCommandQueue> queue;
				CommandAllocator* commandAllocator;

				DKArray<ComPtr<ID3D12GraphicsCommandList>> commandLists;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
