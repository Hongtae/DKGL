//
//  File: GraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#include "../../Interface/DKGraphicsDeviceInterface.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			DKGraphicsDeviceInterface* CreateInterface(void);

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

				PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
				PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
				PFN_vkDebugReportMessageEXT DebugReportMessage;

				DKString deviceName;
				struct DeviceQueue
				{
					uint32_t queueFamilyIndex;
					uint32_t queueIndex;
					VkQueue queue;
				};
				DKArray<DeviceQueue> deviceQueues;

				bool enableValidation;
				VkDebugReportCallbackEXT msgCallback;

				static const char* ErrorString(VkResult);
			};
		}
	}
}
#endif //#if DKGL_USE_VULKAN
