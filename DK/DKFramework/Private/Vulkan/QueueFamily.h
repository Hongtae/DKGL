//
//  File: QueueFamily.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKCommandQueue.h"

namespace DKFramework::Private::Vulkan
{
	class QueueFamily
	{
	public:
		QueueFamily(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t familyIndex, uint32_t queueCount, const VkQueueFamilyProperties& prop, bool supportPresentation);
		~QueueFamily();

		DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*);
		void RecycleQueue(VkQueue);

		// if presentation not supported, should check on the fly!
		bool IsSupportPresentation() const { return supportPresentation; }
		uint32_t FamilyIndex() const { return familyIndex; }

		bool supportPresentation;
		VkQueueFamilyProperties properties;
		DKSpinLock lock;
		uint32_t familyIndex;
		DKArray<VkQueue> freeQueues;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
