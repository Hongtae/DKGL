//
//  File: GraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "GraphicsDevice.h"
#include "CommandQueue.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			DKGraphicsDeviceInterface* CreateInterface(void)
			{
				return new GraphicsDevice();
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

GraphicsDevice::GraphicsDevice(void)
{
}

GraphicsDevice::~GraphicsDevice(void)
{
}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice*)
{
	return NULL;
}

#endif //#if DKGL_USE_VULKAN
