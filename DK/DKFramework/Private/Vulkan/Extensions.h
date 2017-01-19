//
//  File: Extensions.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN
#include <vulkan/vulkan.h>

#define DEF_VK_PFN(name)	PFN_##name name
#define GET_INSTANCE_PROC(inst, name)	name = reinterpret_cast<decltype(name)>(vkGetInstanceProcAddr(inst, #name))
#define GET_DEVICE_PROC(dev, name)	name = reinterpret_cast<decltype(name)>(vkGetDeviceProcAddr(dev, #name))

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			struct InstanceProc
			{
				DEF_VK_PFN(vkCreateDebugReportCallbackEXT) = VK_NULL_HANDLE;
				DEF_VK_PFN(vkDestroyDebugReportCallbackEXT) = VK_NULL_HANDLE;
				DEF_VK_PFN(vkDebugReportMessageEXT) = VK_NULL_HANDLE;

				void Init(VkInstance instance)
				{
					GET_INSTANCE_PROC(instance, vkCreateDebugReportCallbackEXT);
					GET_INSTANCE_PROC(instance, vkDestroyDebugReportCallbackEXT);
					GET_INSTANCE_PROC(instance, vkDebugReportMessageEXT);
				}
			};

			struct DeviceProc
			{
				DEF_VK_PFN(vkDebugMarkerSetObjectTagEXT) = VK_NULL_HANDLE;
				DEF_VK_PFN(vkDebugMarkerSetObjectNameEXT) = VK_NULL_HANDLE;
				DEF_VK_PFN(vkCmdDebugMarkerBeginEXT) = VK_NULL_HANDLE;
				DEF_VK_PFN(vkCmdDebugMarkerEndEXT) = VK_NULL_HANDLE;
				DEF_VK_PFN(vkCmdDebugMarkerInsertEXT) = VK_NULL_HANDLE;

				void Init(VkDevice device)
				{
					GET_DEVICE_PROC(device, vkDebugMarkerSetObjectTagEXT);
					GET_DEVICE_PROC(device, vkDebugMarkerSetObjectNameEXT);
					GET_DEVICE_PROC(device, vkCmdDebugMarkerBeginEXT);
					GET_DEVICE_PROC(device, vkCmdDebugMarkerEndEXT);
					GET_DEVICE_PROC(device, vkCmdDebugMarkerInsertEXT);
				}
			};

			const char* VkResultCStr(VkResult r)
			{
				switch (r)
				{
#define CASE_STR(c) case c: return #c
					CASE_STR(VK_NOT_READY);
					CASE_STR(VK_TIMEOUT);
					CASE_STR(VK_EVENT_SET);
					CASE_STR(VK_EVENT_RESET);
					CASE_STR(VK_INCOMPLETE);
					CASE_STR(VK_ERROR_OUT_OF_HOST_MEMORY);
					CASE_STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
					CASE_STR(VK_ERROR_INITIALIZATION_FAILED);
					CASE_STR(VK_ERROR_DEVICE_LOST);
					CASE_STR(VK_ERROR_MEMORY_MAP_FAILED);
					CASE_STR(VK_ERROR_LAYER_NOT_PRESENT);
					CASE_STR(VK_ERROR_EXTENSION_NOT_PRESENT);
					CASE_STR(VK_ERROR_FEATURE_NOT_PRESENT);
					CASE_STR(VK_ERROR_INCOMPATIBLE_DRIVER);
					CASE_STR(VK_ERROR_TOO_MANY_OBJECTS);
					CASE_STR(VK_ERROR_FORMAT_NOT_SUPPORTED);
					CASE_STR(VK_ERROR_SURFACE_LOST_KHR);
					CASE_STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
					CASE_STR(VK_SUBOPTIMAL_KHR);
					CASE_STR(VK_ERROR_OUT_OF_DATE_KHR);
					CASE_STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
					CASE_STR(VK_ERROR_VALIDATION_FAILED_EXT);
					CASE_STR(VK_ERROR_INVALID_SHADER_NV);
#undef CASE_STR
				}
				return "";
			}
		}
	}
}
#undef DEF_VK_PFN
#undef GET_INSTANCE_PROC
#undef GET_DEVICE_PROC
#endif //#if DKGL_USE_VULKAN
