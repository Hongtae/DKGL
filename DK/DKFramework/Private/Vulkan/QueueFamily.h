//
//  File: QueueFamily.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKCommandQueue.h"
#include "../../Interface/DKBackendInterface.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class QueueFamily : public DKBackendInterface
			{
			public:
				QueueFamily(VkDevice, uint32_t familyIndex, uint32_t count);
				~QueueFamily(void);

				DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*);
				void RecycleQueue(VkQueue);

			private:
				DKSpinLock lock;
				uint32_t familyIndex;
				DKArray<VkQueue> freeQueues;
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
