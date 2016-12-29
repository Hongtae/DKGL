//
//  File: CommandQueue.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#import <Metal/Metal.h>

#include "../../DKCommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class CommandQueue : public DKCommandQueue
			{
			public:
				~CommandQueue(void);

				DKObject<DKCommandBuffer> CreateCommandBuffer(void) override;

				id<MTLCommandQueue> queue;
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
