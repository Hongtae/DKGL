//
//  File: QueueFamily.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKCommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class QueueFamily
			{
			public:
				QueueFamily(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t familyIndex, uint32_t queueCount, const VkQueueFamilyProperties& prop);
				~QueueFamily(void);

				DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*);
				void RecycleQueue(VkQueue);
				
				// if presentation not supported, should check on the fly!
				bool IsSupportPresentation(void) const { return supportPresentation; }
				uint32_t FamilyIndex(void) const { return familyIndex; }

				bool supportPresentation;
				VkQueueFamilyProperties properties;
				DKSpinLock lock;
				uint32_t familyIndex;
				DKArray<VkQueue> freeQueues;
			};
		}
	}
}
#endif //#if DKGL_ENABLE_VULKAN
