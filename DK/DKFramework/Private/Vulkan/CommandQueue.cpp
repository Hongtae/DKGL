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
	return NULL;
}

#endif //#if DKGL_USE_VULKAN
