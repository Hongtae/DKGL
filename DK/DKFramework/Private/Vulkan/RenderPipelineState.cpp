//
//  File: RenderPipelineState.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "RenderPipelineState.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

RenderPipelineState::RenderPipelineState(DKGraphicsDevice* d, VkPipeline p, VkPipelineLayout l, VkRenderPass r)
	: device(d)
	, pipeline(p)
	, layout(l)
	, renderPass(r)
{
}

RenderPipelineState::~RenderPipelineState()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	vkDestroyPipeline(dev->device, pipeline, nullptr);
	vkDestroyPipelineLayout(dev->device, layout, nullptr);
	vkDestroyRenderPass(dev->device, renderPass, nullptr);
}

#endif //#if DKGL_ENABLE_VULKAN
