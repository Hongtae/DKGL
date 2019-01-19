//
//  File: Extensions.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#define DEF_VK_PFN(name)	PFN_##name name
#define GET_INSTANCE_PROC(inst, name)	name = reinterpret_cast<decltype(name)>(vkGetInstanceProcAddr(inst, #name))
#define GET_DEVICE_PROC(dev, name)	name = reinterpret_cast<decltype(name)>(vkGetDeviceProcAddr(dev, #name))

namespace DKFramework::Private::Vulkan
{
	struct InstanceProc
	{
		// VK_EXT_debug_report
		DEF_VK_PFN(vkCreateDebugReportCallbackEXT) = nullptr;
		DEF_VK_PFN(vkDestroyDebugReportCallbackEXT) = nullptr;
		DEF_VK_PFN(vkDebugReportMessageEXT) = nullptr;

		// VK_KHR_surface
		DEF_VK_PFN(vkGetPhysicalDeviceSurfaceSupportKHR) = nullptr;
		DEF_VK_PFN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) = nullptr;
		DEF_VK_PFN(vkGetPhysicalDeviceSurfaceFormatsKHR) = nullptr;
		DEF_VK_PFN(vkGetPhysicalDeviceSurfacePresentModesKHR) = nullptr;

#ifdef VK_USE_PLATFORM_XLIB_KHR
		// VK_KHR_xlib_surface
		DEF_VK_PFN(vkCreateXlibSurfaceKHR) = nullptr;
		DEF_VK_PFN(vkGetPhysicalDeviceXlibPresentationSupportKHR) = nullptr;
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
		// VK_KHR_xcb_surface
		DEF_VK_PFN(vkCreateXcbSurfaceKHR) = nullptr;
		DEF_VK_PFN(vkGetPhysicalDeviceXcbPresentationSupportKHR) = nullptr;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
		// VK_KHR_wayland_surface
		DEF_VK_PFN(vkCreateWaylandSurfaceKHR) = nullptr;
		DEF_VK_PFN(vkGetPhysicalDeviceWaylandPresentationSupportKHR) = nullptr;
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
		// VK_KHR_mir_surface
		DEF_VK_PFN(vkCreateMirSurfaceKHR) = nullptr;
		DEF_VK_PFN(vkGetPhysicalDeviceMirPresentationSupportKHR) = nullptr;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
		// VK_KHR_android_surface
		DEF_VK_PFN(vkCreateAndroidSurfaceKHR) = nullptr;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
		// VK_KHR_win32_surface
		DEF_VK_PFN(vkCreateWin32SurfaceKHR) = nullptr;
		DEF_VK_PFN(vkGetPhysicalDeviceWin32PresentationSupportKHR) = nullptr;
#endif

		void Load(VkInstance instance)
		{
			GET_INSTANCE_PROC(instance, vkCreateDebugReportCallbackEXT);
			GET_INSTANCE_PROC(instance, vkDestroyDebugReportCallbackEXT);
			GET_INSTANCE_PROC(instance, vkDebugReportMessageEXT);

			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);

#ifdef VK_USE_PLATFORM_XLIB_KHR
			GET_INSTANCE_PROC(instance, vkCreateXlibSurfaceKHR);
			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceXlibPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
			GET_INSTANCE_PROC(instance, vkCreateXcbSurfaceKHR);
			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceXcbPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
			GET_INSTANCE_PROC(instance, vkCreateWaylandSurfaceKHR);
			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceWaylandPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
			GET_INSTANCE_PROC(instance, vkCreateMirSurfaceKHR);
			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceMirPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
			GET_INSTANCE_PROC(instance, vkCreateAndroidSurfaceKHR);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
			GET_INSTANCE_PROC(instance, vkCreateWin32SurfaceKHR);
			GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceWin32PresentationSupportKHR);
#endif
		}
	};

	struct DeviceProc
	{
		// VK_EXT_debug_marker
		DEF_VK_PFN(vkDebugMarkerSetObjectTagEXT) = nullptr;
		DEF_VK_PFN(vkDebugMarkerSetObjectNameEXT) = nullptr;
		DEF_VK_PFN(vkCmdDebugMarkerBeginEXT) = nullptr;
		DEF_VK_PFN(vkCmdDebugMarkerEndEXT) = nullptr;
		DEF_VK_PFN(vkCmdDebugMarkerInsertEXT) = nullptr;

		// VK_KHR_swapchain
		DEF_VK_PFN(vkCreateSwapchainKHR) = nullptr;
		DEF_VK_PFN(vkDestroySwapchainKHR) = nullptr;
		DEF_VK_PFN(vkGetSwapchainImagesKHR) = nullptr;
		DEF_VK_PFN(vkAcquireNextImageKHR) = nullptr;
		DEF_VK_PFN(vkQueuePresentKHR) = nullptr;

		void Load(VkDevice device)
		{
			GET_DEVICE_PROC(device, vkDebugMarkerSetObjectTagEXT);
			GET_DEVICE_PROC(device, vkDebugMarkerSetObjectNameEXT);
			GET_DEVICE_PROC(device, vkCmdDebugMarkerBeginEXT);
			GET_DEVICE_PROC(device, vkCmdDebugMarkerEndEXT);
			GET_DEVICE_PROC(device, vkCmdDebugMarkerInsertEXT);

			GET_DEVICE_PROC(device, vkCreateSwapchainKHR);
			GET_DEVICE_PROC(device, vkDestroySwapchainKHR);
			GET_DEVICE_PROC(device, vkGetSwapchainImagesKHR);
			GET_DEVICE_PROC(device, vkAcquireNextImageKHR);
			GET_DEVICE_PROC(device, vkQueuePresentKHR);
		}
	};

	inline const char* VkResultCStr(VkResult r)
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
#undef DEF_VK_PFN
#undef GET_INSTANCE_PROC
#undef GET_DEVICE_PROC
#endif //#if DKGL_ENABLE_VULKAN
