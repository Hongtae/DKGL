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

#define DEF_VK_PFN(name)	PFN_##name name = nullptr
#define GET_INSTANCE_PROC(inst, name)	name = reinterpret_cast<decltype(name)>(vkGetInstanceProcAddr(inst, #name))
#define GET_DEVICE_PROC(dev, name)	name = reinterpret_cast<decltype(name)>(vkGetDeviceProcAddr(dev, #name))

namespace DKFramework::Private::Vulkan
{
	struct InstanceProc
	{
#if 0
        // VK_EXT_debug_report
        DEF_VK_PFN(vkCreateDebugReportCallbackEXT);
        DEF_VK_PFN(vkDestroyDebugReportCallbackEXT);
        DEF_VK_PFN(vkDebugReportMessageEXT);
#endif
        // VK_EXT_debug_utils
        DEF_VK_PFN(vkSetDebugUtilsObjectNameEXT);
        DEF_VK_PFN(vkSetDebugUtilsObjectTagEXT);
        DEF_VK_PFN(vkQueueBeginDebugUtilsLabelEXT);
        DEF_VK_PFN(vkQueueEndDebugUtilsLabelEXT);
        DEF_VK_PFN(vkQueueInsertDebugUtilsLabelEXT);
        DEF_VK_PFN(vkCmdBeginDebugUtilsLabelEXT);
        DEF_VK_PFN(vkCmdEndDebugUtilsLabelEXT);
        DEF_VK_PFN(vkCmdInsertDebugUtilsLabelEXT);
        DEF_VK_PFN(vkCreateDebugUtilsMessengerEXT);
        DEF_VK_PFN(vkDestroyDebugUtilsMessengerEXT);
        DEF_VK_PFN(vkSubmitDebugUtilsMessageEXT);

		// VK_KHR_surface
		DEF_VK_PFN(vkGetPhysicalDeviceSurfaceSupportKHR);
		DEF_VK_PFN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
		DEF_VK_PFN(vkGetPhysicalDeviceSurfaceFormatsKHR);
		DEF_VK_PFN(vkGetPhysicalDeviceSurfacePresentModesKHR);

#ifdef VK_USE_PLATFORM_XLIB_KHR
		// VK_KHR_xlib_surface
		DEF_VK_PFN(vkCreateXlibSurfaceKHR);
		DEF_VK_PFN(vkGetPhysicalDeviceXlibPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
		// VK_KHR_xcb_surface
		DEF_VK_PFN(vkCreateXcbSurfaceKHR);
		DEF_VK_PFN(vkGetPhysicalDeviceXcbPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
		// VK_KHR_wayland_surface
		DEF_VK_PFN(vkCreateWaylandSurfaceKHR);
		DEF_VK_PFN(vkGetPhysicalDeviceWaylandPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
		// VK_KHR_mir_surface
		DEF_VK_PFN(vkCreateMirSurfaceKHR);
		DEF_VK_PFN(vkGetPhysicalDeviceMirPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
		// VK_KHR_android_surface
		DEF_VK_PFN(vkCreateAndroidSurfaceKHR);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
		// VK_KHR_win32_surface
		DEF_VK_PFN(vkCreateWin32SurfaceKHR);
		DEF_VK_PFN(vkGetPhysicalDeviceWin32PresentationSupportKHR);
#endif

		void Load(VkInstance instance)
		{
#if 0
            // VK_EXT_debug_report
            GET_INSTANCE_PROC(instance, vkCreateDebugReportCallbackEXT);
            GET_INSTANCE_PROC(instance, vkDestroyDebugReportCallbackEXT);
            GET_INSTANCE_PROC(instance, vkDebugReportMessageEXT);
#endif
            // VK_EXT_debug_utils
            GET_INSTANCE_PROC(instance, vkSetDebugUtilsObjectNameEXT);
            GET_INSTANCE_PROC(instance, vkSetDebugUtilsObjectTagEXT);
            GET_INSTANCE_PROC(instance, vkQueueBeginDebugUtilsLabelEXT);
            GET_INSTANCE_PROC(instance, vkQueueEndDebugUtilsLabelEXT);
            GET_INSTANCE_PROC(instance, vkQueueInsertDebugUtilsLabelEXT);
            GET_INSTANCE_PROC(instance, vkCmdBeginDebugUtilsLabelEXT);
            GET_INSTANCE_PROC(instance, vkCmdEndDebugUtilsLabelEXT);
            GET_INSTANCE_PROC(instance, vkCmdInsertDebugUtilsLabelEXT);
            GET_INSTANCE_PROC(instance, vkCreateDebugUtilsMessengerEXT);
            GET_INSTANCE_PROC(instance, vkDestroyDebugUtilsMessengerEXT);
            GET_INSTANCE_PROC(instance, vkSubmitDebugUtilsMessageEXT);

            // VK_KHR_surface
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
#if 0
		// VK_EXT_debug_marker
		DEF_VK_PFN(vkDebugMarkerSetObjectTagEXT);
		DEF_VK_PFN(vkDebugMarkerSetObjectNameEXT);
		DEF_VK_PFN(vkCmdDebugMarkerBeginEXT);
		DEF_VK_PFN(vkCmdDebugMarkerEndEXT);
		DEF_VK_PFN(vkCmdDebugMarkerInsertEXT);
#endif
#if 0
		// VK_KHR_swapchain
		DEF_VK_PFN(vkCreateSwapchainKHR);
		DEF_VK_PFN(vkDestroySwapchainKHR);
		DEF_VK_PFN(vkGetSwapchainImagesKHR);
		DEF_VK_PFN(vkAcquireNextImageKHR);
		DEF_VK_PFN(vkQueuePresentKHR);
#endif
#if 0
		// VK_EXT_extended_dynamic_state
        DEF_VK_PFN(vkCmdSetCullModeEXT);
        DEF_VK_PFN(vkCmdSetFrontFaceEXT);
        DEF_VK_PFN(vkCmdSetPrimitiveTopologyEXT);
        DEF_VK_PFN(vkCmdSetViewportWithCountEXT);
        DEF_VK_PFN(vkCmdSetScissorWithCountEXT);
        DEF_VK_PFN(vkCmdBindVertexBuffers2EXT);
        DEF_VK_PFN(vkCmdSetDepthTestEnableEXT);
        DEF_VK_PFN(vkCmdSetDepthWriteEnableEXT);
        DEF_VK_PFN(vkCmdSetDepthCompareOpEXT);
        DEF_VK_PFN(vkCmdSetDepthBoundsTestEnableEXT);
        DEF_VK_PFN(vkCmdSetStencilTestEnableEXT);
        DEF_VK_PFN(vkCmdSetStencilOpEXT);
#endif
		void Load(VkDevice device)
		{
#if 0
            // VK_EXT_debug_marker
			GET_DEVICE_PROC(device, vkDebugMarkerSetObjectTagEXT);
			GET_DEVICE_PROC(device, vkDebugMarkerSetObjectNameEXT);
			GET_DEVICE_PROC(device, vkCmdDebugMarkerBeginEXT);
			GET_DEVICE_PROC(device, vkCmdDebugMarkerEndEXT);
			GET_DEVICE_PROC(device, vkCmdDebugMarkerInsertEXT);
#endif
#if 0
            // VK_KHR_swapchain
			GET_DEVICE_PROC(device, vkCreateSwapchainKHR);
			GET_DEVICE_PROC(device, vkDestroySwapchainKHR);
			GET_DEVICE_PROC(device, vkGetSwapchainImagesKHR);
			GET_DEVICE_PROC(device, vkAcquireNextImageKHR);
			GET_DEVICE_PROC(device, vkQueuePresentKHR);
#endif
#if 0
            // VK_EXT_extended_dynamic_state
            GET_DEVICE_PROC(device, vkCmdSetCullModeEXT);
            GET_DEVICE_PROC(device, vkCmdSetFrontFaceEXT);
            GET_DEVICE_PROC(device, vkCmdSetPrimitiveTopologyEXT);
            GET_DEVICE_PROC(device, vkCmdSetViewportWithCountEXT);
            GET_DEVICE_PROC(device, vkCmdSetScissorWithCountEXT);
            GET_DEVICE_PROC(device, vkCmdBindVertexBuffers2EXT);
            GET_DEVICE_PROC(device, vkCmdSetDepthTestEnableEXT);
            GET_DEVICE_PROC(device, vkCmdSetDepthWriteEnableEXT);
            GET_DEVICE_PROC(device, vkCmdSetDepthCompareOpEXT);
            GET_DEVICE_PROC(device, vkCmdSetDepthBoundsTestEnableEXT);
            GET_DEVICE_PROC(device, vkCmdSetStencilTestEnableEXT);
            GET_DEVICE_PROC(device, vkCmdSetStencilOpEXT);
#endif
		}
	};

	inline const char* VkResultCStr(VkResult r)
	{
		switch (r)
		{
#define CASE_STR(c) case c: return #c
            CASE_STR(VK_SUCCESS);
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
            CASE_STR(VK_ERROR_FRAGMENTED_POOL);
            CASE_STR(VK_ERROR_OUT_OF_POOL_MEMORY);
            CASE_STR(VK_ERROR_INVALID_EXTERNAL_HANDLE);
            CASE_STR(VK_ERROR_SURFACE_LOST_KHR);
            CASE_STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
            CASE_STR(VK_SUBOPTIMAL_KHR);
            CASE_STR(VK_ERROR_OUT_OF_DATE_KHR);
            CASE_STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
            CASE_STR(VK_ERROR_VALIDATION_FAILED_EXT);
            CASE_STR(VK_ERROR_INVALID_SHADER_NV);
            CASE_STR(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
            CASE_STR(VK_ERROR_FRAGMENTATION_EXT);
            CASE_STR(VK_ERROR_NOT_PERMITTED_EXT);
#undef CASE_STR
		}
		return "";
	}
}
#undef DEF_VK_PFN
#undef GET_INSTANCE_PROC
#undef GET_DEVICE_PROC
#endif //#if DKGL_ENABLE_VULKAN
