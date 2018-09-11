//
//  File: CommandBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKCommandBuffer.h"
#include "../../DKCommandQueue.h"

namespace DKFramework::Private::Vulkan
{
	class CommandBuffer : public DKCommandBuffer
	{
	public:
		CommandBuffer(VkCommandPool, DKCommandQueue*);
		~CommandBuffer();

		DKObject<DKRenderCommandEncoder> CreateRenderCommandEncoder(const DKRenderPassDescriptor&) override;
		DKObject<DKComputeCommandEncoder> CreateComputeCommandEncoder() override;
		DKObject<DKBlitCommandEncoder> CreateBlitCommandEncoder() override;

		bool Commit() override;

		DKCommandQueue* Queue() override { return queue; };

		void Submit(const VkSubmitInfo&, DKOperation*);
		void ReleaseEncodingBuffer(VkCommandBuffer cb);
		VkCommandBuffer GetEncodingBuffer();

		VkCommandPool commandPool;
		DKObject<DKCommandQueue> queue;

		DKArray<VkSubmitInfo> submitInfos;
		DKArray<DKObject<DKOperation>> callbacks;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
