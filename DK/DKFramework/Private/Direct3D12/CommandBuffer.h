//
//  File: CommandBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
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
				~CommandBuffer(void);

				DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(DKRenderPassDescriptor*) override;
				DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder(void) override;
				DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder(void) override;

				bool Commit(void) override;
				bool WaitUntilCompleted(void) override;

				DKCommandQueue* Queue(void) override { return queue; };
				
				DKObject<DKCommandQueue> queue;
				CommandAllocator* commandAllocator;

				DKArray<ComPtr<ID3D12CommandList>> commandLists;
			};
		}
	}
}

#endif //#if DKGL_USE_DIRECT3D
