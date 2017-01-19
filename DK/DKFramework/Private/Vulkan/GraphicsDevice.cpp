//
//  File: GraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "GraphicsDevice.h"
#include "CommandQueue.h"
#include "../../DKPropertySet.h"

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
	, msgCallback(NULL)
	, enableValidation(false)
{
#ifdef DKGL_DEBUG_ENABLED
	this->enableValidation = true;
#endif

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "DKGL";
	appInfo.pEngineName = "DKGL";
	appInfo.apiVersion = VK_API_VERSION_1_0;

	DKArray<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

	// Enable surface extensions depending on OS
#ifdef VK_USE_PLATFORM_XLIB_KHR
	enabledExtensions.Add(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
	enabledExtensions.Add(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	enabledExtensions.Add(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
	enabledExtensions.Add(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	enabledExtensions.Add(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
	enabledExtensions.Add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
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
		throw std::exception((const char*)DKStringU8::Format("vkCreateInstance failed: %s", VkResultCStr(err)));
	}
	// initialize instance extensions
	instanceProc.Init(instance);

	if (enableValidation)
	{
		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugMessageCallback;
		dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		
		err = instanceProc.vkCreateDebugReportCallbackEXT(instance, &dbgCreateInfo, nullptr, &msgCallback);
		if (err != VK_SUCCESS)
		{
			throw std::exception((const char*)DKStringU8::Format("CreateDebugReportCallback failed: %s", VkResultCStr(err)));
		}
	}

	struct PhysicalDeviceDesc
	{
		VkPhysicalDevice physicalDevice;
		int deviceTypePriority;
		size_t deviceMemory;
		size_t numQueues;	// graphics | compute queue count.
		DKString deviceName;
	};
	DKArray<PhysicalDeviceDesc> physicalDeviceList;

	// Extract physical devices that can create graphics & compute queues.
	if (true)
	{
		// Physical device
		uint32_t gpuCount = 0;
		// Get number of available physical devices
		err = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
		if (err != VK_SUCCESS)
		{
			throw std::exception((const char*)DKStringU8::Format("vkEnumeratePhysicalDevices failed: %s", VkResultCStr(err)));
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
			throw std::exception((const char*)DKStringU8::Format("vkEnumeratePhysicalDevices failed: %s", VkResultCStr(err)));
		}
		physicalDeviceList.Reserve(gpuCount);

		for (size_t i = 0; i < physicalDevices.Count(); ++i)
		{
			VkPhysicalDevice physicalDevice = physicalDevices.Value(i);
			DKArray<VkQueueFamilyProperties> queueFamilyProperties;

			uint32_t queueFamilyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
			queueFamilyProperties.Resize(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties);

			size_t numGCQueues = 0; // graphics | compute queue
			// calculate num available queues. (Graphics & Compute)
			for (VkQueueFamilyProperties& qfp : queueFamilyProperties)
			{
				if (qfp.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
				{
					numGCQueues += qfp.queueCount;
				}
			}

			if (numGCQueues > 0)
			{
				VkPhysicalDeviceProperties properties;
				VkPhysicalDeviceMemoryProperties memoryProperties;

				vkGetPhysicalDeviceProperties(physicalDevice, &properties);
				vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

				PhysicalDeviceDesc desc;
				desc.physicalDevice = physicalDevice;
				desc.deviceTypePriority = 0;
				desc.numQueues = numGCQueues;
				desc.deviceMemory = 0;
				desc.deviceName = properties.deviceName;

				switch (properties.deviceType)
				{
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:		desc.deviceTypePriority++;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:	desc.deviceTypePriority++;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:		desc.deviceTypePriority++;
				case VK_PHYSICAL_DEVICE_TYPE_CPU:				desc.deviceTypePriority++;
				}

				// calcualte device memory.
				for (uint32_t k = 0; k < memoryProperties.memoryHeapCount; ++k)
				{
					VkMemoryHeap& heap = memoryProperties.memoryHeaps[k];
					if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					{
						desc.deviceMemory += heap.size;
					}
				}

				physicalDeviceList.Add(desc);
			}
		}
		// sort deviceList order by Type/NumQueues/Memory
		physicalDeviceList.Sort([](const PhysicalDeviceDesc& lhs, const PhysicalDeviceDesc& rhs)->bool
		{
			if (lhs.deviceTypePriority == rhs.deviceTypePriority)
			{
				if (lhs.numQueues == rhs.numQueues)
				{
					return lhs.deviceMemory > rhs.deviceMemory;
				}
				return lhs.numQueues > rhs.numQueues;
			}
			return lhs.deviceTypePriority > rhs.deviceTypePriority;
		});
	}

	if (1)	// write device list to DKPropertySet::SystemConfig
	{
		DKVariant deviceList = DKVariant::TypeArray;

		for (size_t i = 0; i < physicalDeviceList.Count(); ++i)
		{
			PhysicalDeviceDesc& desc = physicalDeviceList.Value(i);

			VkPhysicalDevice physicalDevice = desc.physicalDevice;
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

			deviceList.Array().Add(properties.deviceName);
		}
		DKPropertySet::SystemConfig().SetValue("GraphicsDeviceList", deviceList);
	}

	// make preferred device first.
	const char* preferredDeviceNameKey = "PreferredGraphicsDeviceName";
	if (DKPropertySet::SystemConfig().HasValue(preferredDeviceNameKey))
	{
		if (DKPropertySet::SystemConfig().Value(preferredDeviceNameKey).ValueType() == DKVariant::TypeString)
		{
			DKString prefDevName = DKPropertySet::SystemConfig().Value(preferredDeviceNameKey).String();

			if (prefDevName.Length() > 0)
			{
				for (size_t i = 0; i < physicalDeviceList.Count(); ++i)
				{
					if (prefDevName.CompareNoCase(physicalDeviceList.Value(i).deviceName) == 0)
					{
						PhysicalDeviceDesc desc = physicalDeviceList.Value(i);
						physicalDeviceList.Remove(i);
						physicalDeviceList.Insert(desc, 0);
						break;
					}
				}
			}
		}
	}

	// create logical device
	const float defaultQueuePriority(0.0f);
	for (PhysicalDeviceDesc& desc : physicalDeviceList)
	{
		VkPhysicalDevice pdevice = desc.physicalDevice;
		DKArray<VkQueueFamilyProperties> queueFamilyProperties;

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		queueFamilyProperties.Resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties);

		DKArray<VkDeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.Reserve(queueFamilyCount);
		for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex)
		{
			VkQueueFamilyProperties& queueFamily = queueFamilyProperties.Value(queueFamilyIndex);
			if (queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
			{
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndex;
				queueInfo.queueCount = queueFamily.queueCount;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.Add(queueInfo);
			}
		}
		DKASSERT_DEBUG(queueCreateInfos.Count() > 0);

		VkPhysicalDeviceFeatures enabledFeatures{};
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.Count());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
		VkDevice logicalDevice;

		err = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
		if (err == VK_SUCCESS)
		{
			// initialize device extensions
			deviceProc.Init(logicalDevice);

			this->device = logicalDevice;
			this->physicalDevice = physicalDevice;
			this->deviceName = desc.deviceName;

			// get queues
			this->queueFamilies.Reserve(desc.numQueues);

			for (VkDeviceQueueCreateInfo& queueInfo : queueCreateInfos)
			{
				QueueFamily* qf = new QueueFamily(logicalDevice, queueInfo.queueFamilyIndex, queueInfo.queueCount);
				this->queueFamilies.Add(qf);
			}
			DKLog("Vulkan device created with \"%s\"", desc.deviceName);
			break;
		}
	}

	if (this->device == nullptr)
		throw std::exception("Failed to create device!");

	queueFamilies.ShrinkToFit();
}

GraphicsDevice::~GraphicsDevice(void)
{
	for (QueueFamily* family : queueFamilies)
	{
		delete family;
	}
	queueFamilies.Clear();

	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, nullptr);
	if (msgCallback)
		instanceProc.vkDestroyDebugReportCallbackEXT(instance, msgCallback, nullptr);
}

DKString GraphicsDevice::DeviceName(void) const
{
	return deviceName;
}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice* dev)
{
	for (QueueFamily* family : queueFamilies)
	{
		DKObject<DKCommandQueue> queue = family->CreateCommandQueue(dev);
		if (queue)
			return queue;
	}
	return NULL;
}

#endif //#if DKGL_USE_VULKAN
