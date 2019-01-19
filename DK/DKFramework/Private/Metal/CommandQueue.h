//
//  File: CommandQueue.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKCommandQueue.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Metal
{
	class CommandQueue : public DKCommandQueue
	{
	public:
		CommandQueue(DKGraphicsDevice*, id<MTLCommandQueue>);
		~CommandQueue();

		DKObject<DKCommandBuffer> CreateCommandBuffer() override;
		DKObject<DKSwapChain> CreateSwapChain(DKWindow*) override;

		uint32_t Type() const override { return Graphics | Compute; }
		DKGraphicsDevice* Device() override { return device; }

		DKObject<DKGraphicsDevice> device;
		id<MTLCommandQueue> queue;
	};
}
#endif //#if DKGL_ENABLE_METAL
