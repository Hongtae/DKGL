//
//  File: GraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include "Extensions.h"

#include "../../Interface/DKGraphicsDeviceInterface.h"
#include "QueueFamily.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class GraphicsDevice : public DKGraphicsDeviceInterface
			{
			public:
				GraphicsDevice(void);
				~GraphicsDevice(void);

				DKString DeviceName(void) const override;
				DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*) override;

				VkInstance instance;
				VkDevice device;
				VkPhysicalDevice physicalDevice;

				DKArray<QueueFamily*> queueFamilies;
				

				bool enableValidation;
				VkDebugReportCallbackEXT msgCallback;

				// device properties
				VkPhysicalDeviceProperties properties;
				VkPhysicalDeviceFeatures features;
				DKArray<VkExtensionProperties> extensionProperties;
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
