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

			const char *validationLayerNames[] =
			{
				"VK_LAYER_LUNARG_standard_validation"
			};

			VkBool32 DebugMessageCallback(
				VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objType,
				uint64_t srcObject,
				size_t location,
				int32_t msgCode,
				const char* pLayerPrefix,
				const char* pMsg,
				void* pUserData)
			{
				DKStringU8 prefix = "";

				// Error that may result in undefined behaviour
				if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
				{
					prefix += "ERROR:";
				};
				// Warnings may hint at unexpected / non-spec API usage
				if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
				{
					prefix += "WARNING:";
				};
				// May indicate sub-optimal usage of the API
				if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
				{
					prefix += "PERFORMANCE:";
				};
				// Informal messages that may become handy during debugging
				if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
				{
					prefix += "INFO:";
				}
				// Diagnostic info from the Vulkan loader and layers
				// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
				if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
				{
					prefix += "DEBUG:";
				}

				// Display message to default output (console if activated)
				DKLog("[VulkanDebug] %s [%s] Message: %s (0x%x)", (const char*)prefix, pLayerPrefix, pMsg, msgCode);

				// The return value of this callback controls wether the Vulkan call that caused
				// the validation message will be aborted or not
				// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
				// (and return a VkResult) to abort
				// If you instead want to have calls abort, pass in VK_TRUE and the function will 
				// return VK_ERROR_VALIDATION_FAILED_EXT 
				return VK_FALSE;
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;


GraphicsDevice::GraphicsDevice(void)
	: instance(NULL)
	, CreateDebugReportCallback(NULL)
	, DestroyDebugReportCallback(NULL)
	, DebugReportMessage(NULL)
	, msgCallback(NULL)
	, enableValidation(true)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "DKGL";
	appInfo.pEngineName = "DKGL";
	appInfo.apiVersion = VK_API_VERSION_1_0;

	DKArray<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

	// Enable surface extensions depending on os
#if defined(_WIN32)
	enabledExtensions.Add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
	enabledExtensions.Add(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	enabledExtensions.Add(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	if (enabledExtensions.Count() > 0)
	{
		if (enableValidation)
		{
			enabledExtensions.Add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.Count();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions;
	}
	if (enableValidation)
	{
		instanceCreateInfo.enabledLayerCount = 1;
		instanceCreateInfo.ppEnabledLayerNames = validationLayerNames;
	}
	
	VkResult err = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	if (err != VK_SUCCESS)
	{
		throw std::exception((const char*)DKStringU8::Format("vkCreateInstance failed: %s", ErrorString(err)));
	}

	if (enableValidation)
	{
		CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
		DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
		DebugReportMessage = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));

		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugMessageCallback;
		dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		
		err = CreateDebugReportCallback(instance, &dbgCreateInfo, nullptr, &msgCallback);
		if (err != VK_SUCCESS)
		{
			throw std::exception((const char*)DKStringU8::Format("CreateDebugReportCallback failed: %s", ErrorString(err)));
		}
	}

	// Physical device
	uint32_t gpuCount = 0;
	// Get number of available physical devices
	err = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
	if (err != VK_SUCCESS)
	{
		throw std::exception((const char*)DKStringU8::Format("vkEnumeratePhysicalDevices failed: %s", ErrorString(err)));
	}
	if (gpuCount == 0)
	{
		throw std::exception("No vulkan gpu found.");
	}

	// Enumerate devices
	DKArray<VkPhysicalDevice> physicalDevices(VkPhysicalDevice(), gpuCount);
	err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices);
	if (err)
	{
		throw std::exception((const char*)DKStringU8::Format("vkEnumeratePhysicalDevices failed: %s", ErrorString(err)));
	}

	for (size_t i = 0; i < physicalDevices.Count(); ++i)
	{
		VkPhysicalDevice physicalDevice = physicalDevices.Value(i);
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		DKArray<VkQueueFamilyProperties> queueFamilyProperties;
		DKArray<DKString> supportedExtensions;

		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		queueFamilyProperties.Resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties);

		// Get list of supported extensions
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			DKArray<VkExtensionProperties> extensions(VkExtensionProperties(), extCount);
			if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, extensions) == VK_SUCCESS)
			{
				for (auto ext : extensions)
				{
					supportedExtensions.Add(ext.extensionName);
				}
			}
		}

		const char* deviceType = "Unknown";
		switch (properties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			deviceType = "INTEGRATED_GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			deviceType = "DISCRETE_GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			deviceType = "VIRTUAL_GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			deviceType = "CPU";
			break;
		default:
			deviceType = "UNKNOWN";
			break;
		}

		DKLog(" VkPhysicalDevice[%lu]: \"%s\" Type:%s (QueueFamilies:%u)", i, properties.deviceName, deviceType, queueFamilyCount);
		for (size_t j = 0; j < queueFamilyProperties.Count(); ++j)
		{
			VkQueueFamilyProperties& prop = queueFamilyProperties.Value(j);
			DKLog(" -- Queue-Family[%llu] flag-bits:%c%c%c%c count:%d",
				j, 
				prop.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT ? '1' : '0',
				prop.queueFlags & VK_QUEUE_TRANSFER_BIT ? '1' : '0',
				prop.queueFlags & VK_QUEUE_COMPUTE_BIT ? '1' : '0',
				prop.queueFlags & VK_QUEUE_GRAPHICS_BIT ? '1' : '0',
				prop.queueCount);
		}
	}

	DKERROR_THROW("Not implemented yet!");
}

GraphicsDevice::~GraphicsDevice(void)
{
	if (msgCallback)
		DestroyDebugReportCallback(instance, msgCallback, nullptr);
}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice*)
{
	return NULL;
}

const char* GraphicsDevice::ErrorString(VkResult code)
{
	switch (code)
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
	return "UNKNOWN_ERROR";
}

#endif //#if DKGL_USE_VULKAN
