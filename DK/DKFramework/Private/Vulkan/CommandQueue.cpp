//
//  File: CommandQueue.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN

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
	vkQueueWaitIdle(queue);
	family->RecycleQueue(queue);
}

DKObject<DKCommandBuffer> CommandQueue::CreateCommandBuffer(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
	VkDevice device = dev->device;

	VkCommandPoolCreateInfo cmdPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	cmdPoolCreateInfo.queueFamilyIndex = this->family->FamilyIndex();
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkResult err = vkCreateCommandPool(device, &cmdPoolCreateInfo, nullptr, &commandPool);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateCommandPool failed: %s", VkResultCStr(err));
		return NULL;
	}

	DKObject<CommandBuffer> buffer = DKOBJECT_NEW CommandBuffer(commandPool, this);
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
				DKLogE("ERROR: vkGetPhysicalDeviceSurfaceSupportKHR failed: %s", VkResultCStr(err));
				return NULL;
			}
			if (!supported)
			{
				DKLogE("ERROR: Vulkan WSI not supported with this queue family. Try to use other queue family!");
				return NULL;
			}
		}
		return swapChain.SafeCast<DKSwapChain>();
	}
	return NULL;
}

bool CommandQueue::Submit(const VkSubmitInfo* submits, uint32_t submitCount, DKOperation* callback)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);

	VkFence fence = VK_NULL_HANDLE;
	if (callback)
		fence = dev->GetFence();

	VkResult err = vkQueueSubmit(queue, submitCount, submits, fence);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkQueueSubmit failed: %s", VkResultCStr(err));
		DKASSERT(err == VK_SUCCESS);
	}
	if (fence)
		dev->AddFenceCompletionHandler(fence, callback);

	return err == VK_SUCCESS;
}

bool CommandQueue::WaitIdle(void)
{
	return vkQueueWaitIdle(queue) == VK_SUCCESS;
}

#endif //#if DKGL_ENABLE_VULKAN
