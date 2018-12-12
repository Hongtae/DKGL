//
//  File: ComputePipelineState.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2018 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "ComputePipelineState.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ComputePipelineState::ComputePipelineState(DKGraphicsDevice* d, VkPipeline p, VkPipelineLayout l)
	: device(d)
	, pipeline(p)
	, layout(l)
{
}

ComputePipelineState::~ComputePipelineState()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	vkDestroyPipeline(dev->device, pipeline, dev->allocationCallbacks);
	vkDestroyPipelineLayout(dev->device, layout, dev->allocationCallbacks);
}

#endif //#if DKGL_ENABLE_VULKAN
