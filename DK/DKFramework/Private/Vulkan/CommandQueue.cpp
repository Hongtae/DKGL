//
//  File: CommandQueue.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "GraphicsDevice.h"
#include "SwapChain.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

CommandQueue::CommandQueue(DKGraphicsDevice* d, QueueFamily* f, VkQueue q)
	: device(d)
	, family(f)
	, queue(q)	
{
}

CommandQueue::~CommandQueue(void)
{
	family->RecycleQueue(queue);
}

DKObject<DKCommandBuffer> CommandQueue::CreateCommandBuffer(void)
{
	DKObject<CommandBuffer> buffer = DKOBJECT_NEW CommandBuffer();
	buffer->queue = this;

	return buffer.SafeCast<DKCommandBuffer>();
}

DKObject<DKSwapChain> CommandQueue::CreateSwapChain(DKWindow* window)
{
	DKObject<SwapChain> swapChain = DKOBJECT_NEW SwapChain(this, window);
	if (swapChain->Setup())
	{
		if (!this->family->IsSupportPresentation())
		{
			GraphicsDevice* dc = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
			VkPhysicalDevice physicalDevice = dc->physicalDevice;

			VkBool32 supported = VK_FALSE;
			VkResult err = iproc.vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, this->family->FamilyIndex(), swapChain->surface, &supported);
			if (err != VK_SUCCESS)
			{
				DKLog("ERROR: vkGetPhysicalDeviceSurfaceSupportKHR failed: %s", VkResultCStr(err));
				return NULL;
			}
			if (!supported)
			{
				DKLog("ERROR: Vulkan WSI not supported with this queue family. Try to use other queue family!");
				return NULL;
			}
		}
		return swapChain.SafeCast<DKSwapChain>();
	}
	return NULL;
}

#endif //#if DKGL_USE_VULKAN
