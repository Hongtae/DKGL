//
//  File: CommandBuffer.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKCommandBuffer.h"
#include "../../DKCommandQueue.h"

namespace DKFramework::Private::Metal
{
	class CommandEncoder
	{
    public:
		enum { InitialNumberOfCommands = 128 };
		virtual ~CommandEncoder() {}
		virtual bool Encode(id<MTLCommandBuffer>) = 0;
	};

	class CommandBuffer : public DKCommandBuffer
	{
	public:
		CommandBuffer(DKCommandQueue*);
		~CommandBuffer();

		DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(const DKRenderPassDescriptor&) override;
		DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder() override;
		DKObject<DKCopyCommandEncoder> CreateCopyCommandEncoder() override;

		bool Commit() override;
		DKCommandQueue* Queue() override { return queue; };

		void EndEncoder(DKCommandEncoder*, CommandEncoder*);

	private:
		DKObject<DKCommandQueue> queue;
		DKArray<DKObject<CommandEncoder>> encoders;
	};
}
#endif //#if DKGL_ENABLE_METAL
