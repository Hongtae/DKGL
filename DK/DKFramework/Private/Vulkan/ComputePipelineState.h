//
//  File: ComputePipelineState.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2018 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKComputePipeline.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Vulkan
{
	class ComputePipelineState : public DKComputePipelineState
	{
	public:
        ComputePipelineState(DKGraphicsDevice*, VkPipeline, VkPipelineLayout);
		~ComputePipelineState();

		DKGraphicsDevice* Device() override { return device; }

		DKObject<DKGraphicsDevice> device;
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
