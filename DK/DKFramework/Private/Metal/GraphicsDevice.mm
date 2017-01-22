//
//  File: GraphicsDevice.mm
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_METAL

#include "GraphicsDevice.h"
#include "CommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			DKGraphicsDeviceInterface* CreateInterface(void)
			{
				return DKRawPtrNew<GraphicsDevice>();
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private::Metal;


GraphicsDevice::GraphicsDevice(void)
{
	device = MTLCreateSystemDefaultDevice();
}

GraphicsDevice::~GraphicsDevice(void)
{
	[device autorelease];
}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice* ctxt)
{
	DKObject<CommandQueue> queue = DKOBJECT_NEW CommandQueue();
	queue->queue = [device newCommandQueue];
	queue->device = ctxt;
	return queue.SafeCast<DKCommandQueue>();
}

#endif //#if DKGL_USE_METAL
