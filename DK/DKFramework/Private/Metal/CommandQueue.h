//
//  File: CommandQueue.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKCommandQueue.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class CommandQueue : public DKCommandQueue
			{
			public:
				CommandQueue(id<MTLCommandQueue>, DKGraphicsDevice*);
				~CommandQueue(void);

				DKObject<DKCommandBuffer> CreateCommandBuffer(void) override;
				DKObject<DKSwapChain> CreateSwapChain(DKWindow*) override;
				
				DKGraphicsDevice* Device(void) override { return device; }

				DKObject<DKGraphicsDevice> device;
				id<MTLCommandQueue> queue;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
