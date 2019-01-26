//
//  File: CopyCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKCopyCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework::Private::Vulkan
{
	class CopyCommandEncoder : public DKCopyCommandEncoder
	{
	public:
        CopyCommandEncoder(VkCommandBuffer, class CommandBuffer*);
		~CopyCommandEncoder();

		void EndEncoding() override;
		bool IsCompleted() const override { return false; }
		DKCommandBuffer* CommandBuffer() override;

		VkCommandBuffer encodingBuffer;
		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
