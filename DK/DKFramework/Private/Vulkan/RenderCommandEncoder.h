//
//  File: RenderCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKRenderCommandEncoder.h"

#include "CommandBuffer.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class RenderCommandEncoder : public DKRenderCommandEncoder
			{
			public:
				RenderCommandEncoder(CommandBuffer*, const DKRenderPassDescriptor&);
				~RenderCommandEncoder(void);

				void EndEncoding(void) override;
				DKCommandBuffer* Buffer(void) override;

				DKObject<CommandBuffer> commandBuffer;
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
