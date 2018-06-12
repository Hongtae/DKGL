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

namespace DKFramework::Private::Metal
{
	struct ReusableCommandEncoder
	{
		enum { InitialNumberOfCommands = 128 };
		virtual ~ReusableCommandEncoder(void) {}
		virtual bool EncodeBuffer(id<MTLCommandBuffer>) = 0;
		virtual void CompleteBuffer(void) = 0;
	};

	class CommandBuffer : public DKCommandBuffer
	{
	public:
		CommandBuffer(DKCommandQueue*);
		~CommandBuffer(void);

		DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(const DKRenderPassDescriptor&) override;
		DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder(void) override;
		DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder(void) override;

		bool Commit(void) override;
		DKCommandQueue* Queue(void) override { return queue; };

		void EndEncoder(DKCommandEncoder*);

	private:
		DKCommandEncoder * activeEncoder;
		DKObject<DKCommandQueue> queue;
		DKArray<DKObject<ReusableCommandEncoder>> completedEncoders;
	};
}
#endif //#if DKGL_ENABLE_METAL
