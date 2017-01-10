//
//  File: CommandBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "../../DKCommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class CommandBuffer : public DKCommandBuffer
			{
			public:
				~CommandBuffer(void);

				DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(DKRenderPassDescriptor*) override;
				DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder(void) override;
				DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder(void) override;
			};
		}
	}
}

#endif //#if DKGL_USE_VULKAN
