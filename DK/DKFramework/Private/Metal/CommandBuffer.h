//
//  File: CommandBuffer.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKCommandBuffer.h"
#include "../../DKCommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class CommandBuffer : public DKCommandBuffer
			{
			public:
				CommandBuffer(id<MTLCommandBuffer>, DKCommandQueue*);
				~CommandBuffer(void);

				DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(const DKRenderPassDescriptor&) override;
				DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder(void) override;
				DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder(void) override;

				bool Commit(void) override;
				bool WaitUntilCompleted(void) override;
				DKCommandQueue* Queue(void) override { return queue; };

				void EndEncoder(DKCommandEncoder*);

			private:
				DKCommandEncoder* activeEncoder;
				DKObject<DKCommandQueue> queue;
				id<MTLCommandBuffer> buffer;
				bool committed;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
