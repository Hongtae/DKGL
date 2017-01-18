//
//  File: QueueFamily.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "QueueFamily.h"
#include "CommandQueue.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

QueueFamily::QueueFamily(VkDevice device, uint32_t index, uint32_t count)
	: familyIndex(index)
{
	freeQueues.Reserve(count);
	for (uint32_t queueIndex = 0; queueIndex < count; ++queueIndex)
	{
		VkQueue queue = nullptr;
		vkGetDeviceQueue(device, familyIndex, queueIndex, &queue);
		if (queue)
			freeQueues.Add(queue);
	}
}

QueueFamily::~QueueFamily(void)
{
}

DKObject<DKCommandQueue> QueueFamily::CreateCommandQueue(DKGraphicsDevice* dev)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	if (freeQueues.Count() > 0)
	{
		VkQueue queue = freeQueues.Value(freeQueues.Count() - 1);
		freeQueues.Remove(freeQueues.Count() - 1);
		DKObject<CommandQueue> commandQueue = DKOBJECT_NEW CommandQueue(dev, this, queue);
		return commandQueue.SafeCast<DKCommandQueue>();
	}
	return NULL;
}

void QueueFamily::RecycleQueue(VkQueue queue)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	freeQueues.Add(queue);
}

#endif //#if DKGL_USE_VULKAN
