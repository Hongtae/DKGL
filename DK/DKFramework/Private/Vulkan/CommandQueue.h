//
//  File: CommandQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "../../DKCommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class CommandQueue : public DKCommandQueue
			{
			public:
				~CommandQueue(void);

				DKObject<DKCommandBuffer> CreateCommandBuffer(void) override;
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
