//
//  File: GraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "GraphicsDevice.h"
#include "CommandQueue.h"
#include "ShaderModule.h"
#include "ShaderFunction.h"
#include "ShaderBindingSet.h"
#include "RenderPipelineState.h"
#include "ComputePipelineState.h"
#include "Buffer.h"
#include "Texture.h"
#include "Types.h"
#include "../../DKPropertySet.h"

namespace DKFramework::Private::Vulkan
{
	DKGraphicsDeviceInterface* CreateInterface()
	{
		return new GraphicsDevice();
	}

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
		DKLogCategory cat = DKLogCategory::Info;

		// Error that may result in undefined behaviour
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			prefix += "ERROR:";
			cat = DKLogCategory::Error;
		};
		// Warnings may hint at unexpected / non-spec API usage
		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			prefix += "WARNING:";
			cat = DKLogCategory::Warning;
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
		DKLog(cat, "[Vulkan-Debug] %s [%s] Message: %s (0x%x)", (const char*)prefix, pLayerPrefix, pMsg, msgCode);

		// The return value of this callback controls wether the Vulkan call that caused
		// the validation message will be aborted or not
		// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message
		// (and return a VkResult) to abort
		// If you instead want to have calls abort, pass in VK_TRUE and the function will
		// return VK_ERROR_VALIDATION_FAILED_EXT

#ifdef DKGL_DEBUG_ENABLED
        if (cat == DKLogCategory::Error)
        {
            if (!DKIsDebuggerPresent())
                return VK_TRUE;
        }
#endif
		return VK_FALSE;
	}

    struct Cleanup
    {
        DKObject<DKOperation> op;
        ~Cleanup()
        {
            op->Perform();
        }
    };
}
using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;


#define DKGL_PRINT_VULKAN_LAYERS 1
#define DKGL_PRINT_VULKAN_EXTENSIONS 1
#define DKGL_PRINT_VULKAN_DEVICE_EXTENSIONS 1

GraphicsDevice::GraphicsDevice()
	: instance(NULL)
	, device(NULL)
	, physicalDevice(NULL)
	, msgCallback(NULL)
	, fenceCompletionThreadRunning(true)
	, numberOfFences(0)
	, pipelineCache(VK_NULL_HANDLE)
    , allocationCallbacks(nullptr)
{
    VkResult err = VK_SUCCESS;

    auto getBoolValueFromSystemConfig = [](const char* key, bool defaultValue)->bool
    {
        bool value = defaultValue;
        DKPropertySet::SystemConfig().LookUpValueForKeyPath(key, DKFunction([&](const DKVariant& var)
        {
            if (var.ValueType() == DKVariant::TypeInteger)
            {
                value = var.Integer() != 0;
                return true;
            }
            return false;
        }));
        return value;
    };

    bool enableAllExtensions =  getBoolValueFromSystemConfig(vulkanEnableAllExtensions, false);
    bool enableExtensionsForEnabledLayers = getBoolValueFromSystemConfig(vulkanEnableExtensionsForEnabledLayers, false);
    bool enableLayersForEnabledExtensions = getBoolValueFromSystemConfig(vulkanEnableLayersForEnabledExtensions, false);

    bool enableValidation = getBoolValueFromSystemConfig(vulkanEnableValidation, false);
    bool enableDebugMarker = getBoolValueFromSystemConfig(vulkanEnableDebugMarker, false);

#ifdef DKGL_DEBUG_ENABLED
    enableValidation = true;
    enableDebugMarker = true;
#endif

    auto getStringSetValueFromSystemConfig = [](const char* key, const DKSet<DKStringU8>& defaultValue = {})
    {
        DKSet<DKStringU8> set = defaultValue;
        DKPropertySet::SystemConfig().LookUpValueForKeyPath(key, DKFunction([&](const DKVariant& var)->bool
        {
            if (var.ValueType() == DKVariant::TypeArray)
            {
                for (const DKVariant& v : var.Array())
                {
                    if (v.ValueType() == DKVariant::TypeString)
                    {
                        set.Insert(DKStringU8(v.String()));
                    }
                }
                return true;
            }
            return false;
        }));
        return set;
    };

    DKSet<DKStringU8> configRequiredLayers = getStringSetValueFromSystemConfig(vulkanRequiredLayers);
    DKSet<DKStringU8> configOptionalLayers = getStringSetValueFromSystemConfig(vulkanOptionalLayers);
    DKSet<DKStringU8> configRequiredInstanceExtensions = getStringSetValueFromSystemConfig(vulkanRequiredInstanceExtensions);
    DKSet<DKStringU8> configOptionalInstanceExtensions = getStringSetValueFromSystemConfig(vulkanOptionalInstanceExtensions);
    DKSet<DKStringU8> configRequiredDeviceExtensions = getStringSetValueFromSystemConfig(vulkanRequiredDeviceExtensions);
    DKSet<DKStringU8> configOptionalDeviceExtensions = getStringSetValueFromSystemConfig(vulkanOptionalDeviceExtensions);

    if (enableValidation)
    {
        configRequiredLayers.Insert("VK_LAYER_LUNARG_standard_validation");
        configRequiredInstanceExtensions.Insert(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    if (1)
    {
        configRequiredInstanceExtensions.Insert(VK_KHR_SURFACE_EXTENSION_NAME);
        // Enable surface extensions depending on OS
#ifdef VK_USE_PLATFORM_XLIB_KHR
        configRequiredInstanceExtensions.Insert(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
        configRequiredInstanceExtensions.Insert(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        configRequiredInstanceExtensions.Insert(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
        configRequiredInstanceExtensions.Insert(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        configRequiredInstanceExtensions.Insert(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        configRequiredInstanceExtensions.Insert(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
    }

    configRequiredDeviceExtensions.Insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    configRequiredDeviceExtensions.Insert(VK_KHR_MAINTENANCE1_EXTENSION_NAME);

    configOptionalDeviceExtensions.Insert(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    configOptionalDeviceExtensions.Insert(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    if (enableDebugMarker)
        configOptionalDeviceExtensions.Insert(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);


	DKMap<DKStringU8, VkLayerProperties> supportingLayers;
    DKMap<DKStringU8, DKArray<VkExtensionProperties>> layerExtensions;
    DKMap<DKStringU8, DKArray<DKStringU8>> extensionSupportLayers;  // key: extension-name, value: layer-names(array)
    DKArray<VkExtensionProperties> instanceExtensions; // instance extensions provided by the Vulkan implementation
	// checking layers
	if (1)
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		DKArray<VkLayerProperties> availableLayers(VkLayerProperties(), layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

		DKLog("Vulkan available layers: %d", layerCount);
		for (const VkLayerProperties& prop : availableLayers)
		{
#if DKGL_PRINT_VULKAN_LAYERS
			DKLog(" -- Layer: %s (\"%s\", %d / %d)",
				  prop.layerName,
				  prop.description,
				  prop.specVersion, 
				  prop.implementationVersion);
#endif
			supportingLayers.Update(prop.layerName, prop);

            uint32_t extCount = 0;
            err = vkEnumerateInstanceExtensionProperties(prop.layerName, &extCount, nullptr);
            if (err != VK_SUCCESS)
            {
                throw std::runtime_error((const char*)DKStringU8::Format("vkEnumerateInstanceExtensionProperties failed: %s", VkResultCStr(err)));
            }
            DKArray<VkExtensionProperties> extensions;
            if (extCount > 0)
            {
                extensions.Resize(extCount);
                err = vkEnumerateInstanceExtensionProperties(prop.layerName, &extCount, extensions);
                DKASSERT_DEBUG(err == VK_SUCCESS);

                for (const VkExtensionProperties& ext : extensions)
                {
                    auto* p = extensionSupportLayers.Find(ext.extensionName);
                    if (p)
                        p->value.Add(prop.layerName);
                    else
                        extensionSupportLayers.Insert(ext.extensionName, { prop.layerName });

#if DKGL_PRINT_VULKAN_EXTENSIONS
                    DKLog(" ---- Layer extensions: %s (Version: %u)\n", ext.extensionName, ext.specVersion);
#endif
                }
            }
            layerExtensions.Update(prop.layerName, std::move(extensions));
        }

        // default ext
        if (1)
        {
            uint32_t extCount = 0;
            err = vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
            if (err != VK_SUCCESS)
            {
                throw std::runtime_error((const char*)DKStringU8::Format("vkEnumerateInstanceExtensionProperties failed: %s", VkResultCStr(err)));
            }
            DKArray<VkExtensionProperties> extensions;
            if (extCount > 0)
            {
                extensions.Resize(extCount);
                err = vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extensions);
                DKASSERT_DEBUG(err == VK_SUCCESS);

                for (const VkExtensionProperties& ext : extensions)
                {
                    auto* p = extensionSupportLayers.Find(ext.extensionName);
                    if (p == nullptr)
                        extensionSupportLayers.Insert(ext.extensionName, {});

#if DKGL_PRINT_VULKAN_EXTENSIONS
                    DKLog(" -- Instance extensions: %s (Version: %u)\n", ext.extensionName, ext.specVersion);
#endif
                }
            }
            instanceExtensions = std::move(extensions);
        }
	}

	VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
	appInfo.pApplicationName = "DKGL";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "DKGL";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1; // Vulkan-1.1

    // add layers for required extensions
    configRequiredInstanceExtensions.EnumerateForward([&](const DKStringU8& ext)
    {
        auto* p = extensionSupportLayers.Find(ext);
        if (p)
        {
            if (enableLayersForEnabledExtensions)
            {
                for (const DKStringU8& layer : p->value)
                    configRequiredLayers.Insert(layer);
            }
        }
        else
        {
            DKLogW("InstanceExtension(%s) not supported, but required.", (const char*)ext);
        }
    });
    // add layers for optional extensions,
    configOptionalInstanceExtensions.EnumerateForward([&](const DKStringU8& ext)
    {
        auto* p = extensionSupportLayers.Find(ext);
        if (p)
        {
            if (enableLayersForEnabledExtensions)
            {
                for (const DKStringU8& layer : p->value)
                    configOptionalLayers.Insert(layer);
            }
        }
        else
        {
            DKLogW("InstanceExtension(%s) not supported.", (const char*)ext);
        }
    });

    // setup layer!
    DKArray<const char*> enabledLayers = {};
    if (1)
    {
        // merge two sets.
        configOptionalLayers.EnumerateForward([&](const DKStringU8& layer)
        {
            if (supportingLayers.Find(layer))
                configRequiredLayers.Insert(layer);
            else
                DKLogW("Layer(%s) not supported.", (const char*)layer);
        });

        configRequiredLayers.EnumerateForward([&](const DKStringU8& layer)
        {
            enabledLayers.Add(layer);
            if (!supportingLayers.Find(layer))
            {
                DKLogW("Layer(%s) not supported, but required.", (const char*)layer);
            }
        });
    }

    // setup instance extensions!
    DKArray<const char*> enabledInstanceExtensions = {  };
    if (1)
    {
        if (enableAllExtensions)
        {
            for (const VkExtensionProperties& ext : instanceExtensions)
            {
                configOptionalInstanceExtensions.Insert(ext.extensionName);
            }
        }
        if (enableExtensionsForEnabledLayers)
        {
            for (const char* layer : enabledLayers)
            {
                auto* p = layerExtensions.Find(layer);
                if (p)
                {
                    for (const VkExtensionProperties& ext : p->value)
                        configOptionalInstanceExtensions.Insert(ext.extensionName);
                }
            }
        }

        // merge two sets.
        configOptionalInstanceExtensions.EnumerateForward([&](const DKStringU8& ext)
        {
            if (extensionSupportLayers.Find(ext))
                configRequiredInstanceExtensions.Insert(ext);
        });

        configRequiredInstanceExtensions.EnumerateForward([&](const DKStringU8& ext)
        {
            enabledInstanceExtensions.Add(ext);
        });
    }

    VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.pApplicationInfo = &appInfo;
    if (enabledLayers.Count() > 0)
    {
        instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayers.Count();
        instanceCreateInfo.ppEnabledLayerNames = enabledLayers;
    }
	if (enabledInstanceExtensions.Count() > 0)
	{
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledInstanceExtensions.Count();
		instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensions;
	}

	err = vkCreateInstance(&instanceCreateInfo, allocationCallbacks, &instance);
	if (err != VK_SUCCESS)
	{
		throw std::runtime_error((const char*)DKStringU8::Format("vkCreateInstance failed: %s", VkResultCStr(err)));
	}
    if (enabledLayers.IsEmpty())
        DKLogI("VkInstance enabled layers: None\n");
    else
    {
        for (int i = 0; i < enabledLayers.Count(); ++i)
            DKLogI("VkInstance enabled layers[%d]: %s\n", i, enabledLayers.Value(i));
    }
    if (enabledInstanceExtensions.IsEmpty())
        DKLogI("VkInstance enabled extensions: None\n");
    else
    {
        for (int i = 0; i < enabledInstanceExtensions.Count(); ++i)
            DKLogI("VkInstance enabled extensions[%d]: %s\n", i, enabledInstanceExtensions.Value(i));
    }

	// load instance extensions
	iproc.Load(instance);

    if (iproc.vkCreateDebugReportCallbackEXT)
	{
        VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
		dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugMessageCallback;
		dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		
		err = iproc.vkCreateDebugReportCallbackEXT(instance, &dbgCreateInfo, allocationCallbacks, &msgCallback);
		if (err != VK_SUCCESS)
		{
			throw std::runtime_error((const char*)DKStringU8::Format("CreateDebugReportCallback failed: %s", VkResultCStr(err)));
		}
	}

	struct PhysicalDeviceDesc
	{
		VkPhysicalDevice physicalDevice;
		int deviceTypePriority;
		size_t deviceMemory;
		size_t numQueues;	// graphics | compute queue count.

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		DKArray<VkQueueFamilyProperties> queueFamilyProperties;
		DKArray<VkExtensionProperties> extensionProperties;

		bool IsExtensionSupported(const char* ext) const
		{
			for (const VkExtensionProperties& ep : this->extensionProperties)
			{
				if (strcmp(ep.extensionName, ext) == 0)
					return true;
			}
			return false;
		}
	};
	DKArray<PhysicalDeviceDesc> physicalDeviceList;

	size_t maxQueueCount = 0;
	// Extract physical devices that can create graphics & compute queues.
	if (true)
	{
		// Physical device
		uint32_t gpuCount = 0;
		// Get number of available physical devices
		err = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
		if (err != VK_SUCCESS)
		{
			throw std::runtime_error((const char*)DKStringU8::Format("vkEnumeratePhysicalDevices failed: %s", VkResultCStr(err)));
		}
		if (gpuCount == 0)
		{
			throw std::runtime_error("No vulkan gpu found.");
		}
		DKLogI("Vulkan GPU Count: %lu", gpuCount);
		// Enumerate devices
		DKArray<VkPhysicalDevice> physicalDevices(VkPhysicalDevice(), gpuCount);
		err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices);
		if (err)
		{
			throw std::runtime_error((const char*)DKStringU8::Format("vkEnumeratePhysicalDevices failed: %s", VkResultCStr(err)));
		}
		physicalDeviceList.Reserve(gpuCount);

		for (size_t i = 0; i < physicalDevices.Count(); ++i)
		{
			PhysicalDeviceDesc desc;
			desc.physicalDevice = physicalDevices.Value(i);;
			desc.deviceTypePriority = 0;

			uint32_t queueFamilyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(desc.physicalDevice, &queueFamilyCount, nullptr);
			desc.queueFamilyProperties.Resize(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(desc.physicalDevice, &queueFamilyCount, desc.queueFamilyProperties);

			size_t numGCQueues = 0; // graphics | compute queue
			// calculate num available queues. (Graphics & Compute)
			for (VkQueueFamilyProperties& qfp : desc.queueFamilyProperties)
			{
				if (qfp.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
				{
					numGCQueues += qfp.queueCount;
				}
				maxQueueCount = Max(maxQueueCount, (size_t)qfp.queueCount);
			}

			if (numGCQueues > 0)
			{
				desc.numQueues = numGCQueues;
				desc.deviceMemory = 0;

				vkGetPhysicalDeviceProperties(desc.physicalDevice, &desc.properties);
				vkGetPhysicalDeviceMemoryProperties(desc.physicalDevice, &desc.memoryProperties);
				vkGetPhysicalDeviceFeatures(desc.physicalDevice, &desc.features);

				switch (desc.properties.deviceType)
				{
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:		desc.deviceTypePriority++;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:	desc.deviceTypePriority++;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:		desc.deviceTypePriority++;
				case VK_PHYSICAL_DEVICE_TYPE_CPU:				desc.deviceTypePriority++;
				}

				// calcualte device memory.
				for (uint32_t k = 0; k < desc.memoryProperties.memoryHeapCount; ++k)
				{
					VkMemoryHeap& heap = desc.memoryProperties.memoryHeaps[k];
					if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					{
						desc.deviceMemory += heap.size;
					}
				}

				// Get list of supported extensions
				uint32_t extCount = 0;
				vkEnumerateDeviceExtensionProperties(desc.physicalDevice, nullptr, &extCount, nullptr);
				if (extCount > 0)
				{
					desc.extensionProperties.Resize(extCount);
					vkEnumerateDeviceExtensionProperties(desc.physicalDevice, nullptr, &extCount, desc.extensionProperties);
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

			const char* deviceType = "Unknown";
			switch (desc.properties.deviceType)
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

			uint32_t apiVersionMajor = desc.properties.apiVersion >> 22;
			uint32_t apiVersionMinor = (desc.properties.apiVersion >> 12) & 0x3ff;
			uint32_t apiVersionPatch = desc.properties.apiVersion & 0xfff;
			DKLog(" VkPhysicalDevice[%lu]: \"%s\" Type:%s API:%d.%d.%d (QueueFamilies:%u)", i,
				  desc.properties.deviceName, deviceType,
				  apiVersionMajor, apiVersionMinor, apiVersionPatch,
				  (uint32_t)desc.queueFamilyProperties.Count());
			for (size_t j = 0; j < desc.queueFamilyProperties.Count(); ++j)
			{
                const VkQueueFamilyProperties& prop = desc.queueFamilyProperties.Value(j);
                DKLog(" -- Queue-Family[%llu] Flags:0x%04x (SparseBinding:%c, Transfer:%c, Compute:%c, Graphics:%c), Queues:%d",
                      j,
                      prop.queueFlags,
                      prop.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT ? 'Y' : 'N',
                      prop.queueFlags & VK_QUEUE_TRANSFER_BIT ? 'Y' : 'N', // this is optional for Graphics, Compute queue.
                      prop.queueFlags & VK_QUEUE_COMPUTE_BIT ? 'Y' : 'N',
                      prop.queueFlags & VK_QUEUE_GRAPHICS_BIT ? 'Y' : 'N',
                      prop.queueCount);
			}
#if DKGL_PRINT_VULKAN_DEVICE_EXTENSIONS
            for (size_t j = 0; j < desc.extensionProperties.Count(); ++j)
            {
                const VkExtensionProperties& prop = desc.extensionProperties.Value(j);
                DKLog(" -- Device-Extension[%llu]: %s (Version:%u)\n", j, prop.extensionName, prop.specVersion);
            }
#endif
			deviceList.Array().Add(desc.properties.deviceName);
		}
		DKPropertySet::SystemConfig().SetValue(graphicsDeviceListKey, deviceList);
	}

	// make preferred device first.
	DKPropertySet::SystemConfig().LookUpValueForKeyPath(preferredDeviceNameKey,
														DKFunction([&physicalDeviceList](const DKVariant& var)->bool
	{
		if (var.ValueType() == DKVariant::TypeString)
		{
			DKString preferredDeviceName = var.String();
			if (preferredDeviceName.Length() > 0)
			{
				for (size_t i = 0; i < physicalDeviceList.Count(); ++i)
				{
					if (preferredDeviceName.CompareNoCase(physicalDeviceList.Value(i).properties.deviceName) == 0)
					{
						PhysicalDeviceDesc desc = physicalDeviceList.Value(i);
						physicalDeviceList.Remove(i);
						physicalDeviceList.Insert(desc, 0);
						break;
					}
				}
			}
			return true;
		}
		return false;
	}));

	// create logical device
	DKArray<float> defaultQueuePriorities(0.0f, 16);
	if (maxQueueCount > defaultQueuePriorities.Count())
		defaultQueuePriorities.Add(0.0f, maxQueueCount - defaultQueuePriorities.Count());

	for (PhysicalDeviceDesc& desc : physicalDeviceList)
	{
		VkPhysicalDevice pdevice = desc.physicalDevice;

		DKArray<VkDeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.Reserve(desc.queueFamilyProperties.Count());
		for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < desc.queueFamilyProperties.Count(); ++queueFamilyIndex)
		{
			VkQueueFamilyProperties& queueFamily = desc.queueFamilyProperties.Value(queueFamilyIndex);
			if (queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))
			{
				VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = queueFamilyIndex;
				queueInfo.queueCount = queueFamily.queueCount;
				queueInfo.pQueuePriorities = (const float*)defaultQueuePriorities;
				queueCreateInfos.Add(queueInfo);
			}
		}
		DKASSERT_DEBUG(queueCreateInfos.Count() > 0);

		// setup device extensions
		DKArray<const char*> deviceExtensions;

		if (enableAllExtensions)
		{
			deviceExtensions.Reserve(desc.extensionProperties.Count());
			for (VkExtensionProperties& ep : desc.extensionProperties)
				deviceExtensions.Add(ep.extensionName);
		}
		else
		{
            configRequiredDeviceExtensions.EnumerateForward([&](const DKStringU8& ext)
            {
                deviceExtensions.Add(ext);
                if (!desc.IsExtensionSupported(ext))
                    DKLogW("DeviceExtension: %s not supported, but required.", ext);
            });
            configOptionalDeviceExtensions.EnumerateForward([&](const DKStringU8& ext)
            {
                if (desc.IsExtensionSupported(ext))
                    deviceExtensions.Add(ext);
                else
                    DKLogW("DeviceExtension: %s not supported!", ext);
            });
        }

		VkPhysicalDeviceFeatures enabledFeatures = desc.features; //enable all features supported by a device
		VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.Count());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		if (deviceExtensions.Count() > 0)
		{
			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.Count();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
		}

		VkDevice logicalDevice = nullptr;
		err = vkCreateDevice(desc.physicalDevice, &deviceCreateInfo, allocationCallbacks, &logicalDevice);
		if (err == VK_SUCCESS)
		{
			// initialize device extensions
			dproc.Load(logicalDevice);

			this->device = logicalDevice;
			this->physicalDevice = desc.physicalDevice;
			this->properties = desc.properties;
			this->features = desc.features;
			this->deviceMemoryTypes = DKArray<VkMemoryType>(desc.memoryProperties.memoryTypes, desc.memoryProperties.memoryTypeCount);
			this->deviceMemoryHeaps = DKArray<VkMemoryHeap>(desc.memoryProperties.memoryHeaps, desc.memoryProperties.memoryHeapCount);
			this->queueFamilyProperties = desc.queueFamilyProperties;
			this->extensionProperties = desc.extensionProperties;

			// get queues
			this->queueFamilies.Reserve(desc.numQueues);

			for (const VkDeviceQueueCreateInfo& queueInfo : queueCreateInfos)
			{
				bool supportPresentation = false;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
				supportPresentation = true;	// always true on Android
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
				supportPresentation = iproc.vkGetPhysicalDeviceWin32PresentationSupportKHR(desc.physicalDevice, queueInfo.queueFamilyIndex) != VK_FALSE;
#endif

				QueueFamily* qf = new QueueFamily(desc.physicalDevice, logicalDevice, queueInfo.queueFamilyIndex, queueInfo.queueCount, desc.queueFamilyProperties.Value(queueInfo.queueFamilyIndex), supportPresentation);
				this->queueFamilies.Add(qf);
			}
			DKLogI("Vulkan device created with \"%s\" (EnabledExtensionCount: %u)", desc.properties.deviceName, deviceCreateInfo.enabledExtensionCount);
            for (int i = 0; i < deviceExtensions.Count(); ++i)
            {
                DKLogI(" -- Enabled extensions[%d]: %s\n", i, deviceExtensions.Value(i));
            }
			break;
		}
	}

	if (this->device == nullptr)
		throw std::runtime_error("Failed to create device!");

	// sort queue family order by presentationSupport, index
	queueFamilies.Sort([](QueueFamily* lhs, QueueFamily* rhs)->bool
	{
		int lhs_ps = lhs->IsSupportPresentation() ? 1 : 0;
		int rhs_ps = rhs->IsSupportPresentation() ? 1 : 0;

		if (lhs_ps == rhs_ps)
		{
			return lhs->FamilyIndex() < rhs->FamilyIndex(); // smaller index first
		}
		return lhs_ps > rhs_ps;
	});
	queueFamilies.ShrinkToFit();

	LoadPipelineCache();

	fenceCompletionThread = DKThread::Create(
		DKFunction(this, &GraphicsDevice::FenceCompletionCallbackThreadProc)->Invocation());
}

GraphicsDevice::~GraphicsDevice()
{
    for (int i = 0; i < NumDescriptorPoolChainBuckets; ++i)
    {
        DKASSERT_DEBUG(descriptorPoolChainMaps[i].poolChainMap.IsEmpty());
    }

	vkDeviceWaitIdle(device);
	if (fenceCompletionThread && fenceCompletionThread->IsAlive())
	{
		fenceCompletionCond.Lock();
		fenceCompletionThreadRunning = false;
		fenceCompletionCond.Broadcast();
		fenceCompletionCond.Unlock();

		fenceCompletionThread->WaitTerminate();
	}

	DKASSERT_DEBUG(pendingFenceCallbacks.IsEmpty());
	for (VkFence fence : reusableFences)
		vkDestroyFence(device, fence, allocationCallbacks);
	reusableFences.Clear();

	for (QueueFamily* family : queueFamilies)
	{
		delete family;
	}
	queueFamilies.Clear();

	vkDeviceWaitIdle(device);

	// destroy pipeline cache
	if (pipelineCache != VK_NULL_HANDLE)
	{
		vkDestroyPipelineCache(device, pipelineCache, allocationCallbacks);
		this->pipelineCache = VK_NULL_HANDLE;
	}

	vkDestroyDevice(device, allocationCallbacks);
	if (msgCallback)
		iproc.vkDestroyDebugReportCallbackEXT(instance, msgCallback, allocationCallbacks);

    if (allocationCallbacks)
        delete allocationCallbacks;
}

DKString GraphicsDevice::DeviceName() const
{
	return DKString(properties.deviceName);
}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice* dev, uint32_t flags)
{
	uint32_t queueFlags = 0;
	if (flags & DKCommandQueue::Graphics)
		queueFlags = queueFlags | VK_QUEUE_GRAPHICS_BIT;
	if (flags & DKCommandQueue::Compute)
		queueFlags = queueFlags | VK_QUEUE_COMPUTE_BIT;
    uint32_t queueMask = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
	queueMask = queueMask ^ queueFlags;

	// find the exact matching queue
	for (QueueFamily* family : queueFamilies)
	{
		if ((family->properties.queueFlags & queueMask) == 0 &&
			(family->properties.queueFlags & queueFlags) == queueFlags)
		{
			DKObject<DKCommandQueue> queue = family->CreateCommandQueue(dev);
			if (queue)
				return queue;
		}
	}

	// find any queue that satisfies the condition.
	for (QueueFamily* family : queueFamilies)
	{
		if ((family->properties.queueFlags & queueFlags) == queueFlags)
		{
			DKObject<DKCommandQueue> queue = family->CreateCommandQueue(dev);
			if (queue)
				return queue;
		}
	}
	//TODO: share queue!
	return NULL;
}

DKObject<DKShaderModule> GraphicsDevice::CreateShaderModule(DKGraphicsDevice* dev, DKShader* shader)
{
	DKASSERT_DEBUG(shader);
	if (DKData* data = shader->Data(); data)
	{
        for (const DKShaderPushConstantLayout& layout : shader->PushConstantBufferLayouts())
        {
            if (layout.offset >= this->properties.limits.maxPushConstantsSize)
            {
                DKLogE("ERROR: PushConstant offset is out of range. (offset:%u, limit:%u)",
                       layout.offset,
                       this->properties.limits.maxPushConstantsSize);
                return NULL;
            }
            if (layout.offset + layout.size > this->properties.limits.maxPushConstantsSize)
            {
                DKLogE("ERROR: PushConstant range exceeded limit. (offset:%u, size:%u, limit:%u)",
                       layout.offset, layout.size,
                       this->properties.limits.maxPushConstantsSize);
                return NULL;
            }
        }

        if (auto threadWorkgroupSize = shader->ThreadgroupSize();
            (threadWorkgroupSize.x > this->properties.limits.maxComputeWorkGroupSize[0]) ||
            (threadWorkgroupSize.y > this->properties.limits.maxComputeWorkGroupSize[1]) ||
            (threadWorkgroupSize.z > this->properties.limits.maxComputeWorkGroupSize[2]))
        {
            DKLogE("ERROR: Thread-WorkGroup size exceeded limit. Size:(%u, %u, %u), Limit:(%u, %u, %u)",
                   threadWorkgroupSize.x,
                   threadWorkgroupSize.y,
                   threadWorkgroupSize.z,
                   this->properties.limits.maxComputeWorkGroupSize[0],
                   this->properties.limits.maxComputeWorkGroupSize[1],
                   this->properties.limits.maxComputeWorkGroupSize[2]);
            return NULL;
        }

		DKDataReader reader(data);
		if (reader.Length() > 0)
		{
			VkShaderModuleCreateInfo shaderModuleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			shaderModuleCreateInfo.codeSize = reader.Length();
			shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(reader.Bytes());

			VkShaderModule shaderModule;
			VkResult res = vkCreateShaderModule(device, &shaderModuleCreateInfo, allocationCallbacks, &shaderModule);
			if (res != VK_SUCCESS)
			{
				DKLogE("ERROR: vkCreateShaderModule failed: %s", VkResultCStr(res));
				return NULL;
			}

			switch (shader->Stage())
			{
			case DKShaderStage::Vertex:
			case DKShaderStage::Fragment:
			case DKShaderStage::Compute:
				break;
			default:
                DKLogW("Warning: Unsupported shader type!");
                break;
			}

			DKObject<ShaderModule> module = DKOBJECT_NEW ShaderModule(dev, shaderModule, shader);
			return module.SafeCast<DKShaderModule>();
		}
	}
	return NULL;
}

DKObject<DKShaderBindingSet> GraphicsDevice::CreateShaderBindingSet(DKGraphicsDevice* dev, const DKShaderBindingSetLayout& layout)
{
    DescriptorPoolId poolId(layout);
    if (poolId.mask)
    {
        DKHashResult32 hash = DKHashCRC32(&poolId, sizeof(poolId));
        uint32_t index = hash.digest[9] % NumDescriptorPoolChainBuckets;

        DescriptorPoolChainMap& dpChainMap = descriptorPoolChainMaps[index];

        DKCriticalSection<DKSpinLock> guard(dpChainMap.lock);

        // find matching pool.
        auto p = dpChainMap.poolChainMap.Find(poolId);
        if (!p)
        {
            // create new pool-chain.
            DescriptorPoolChain* chain = new DescriptorPoolChain(this, poolId);
            dpChainMap.poolChainMap.Update(poolId, chain);
        }        
        DescriptorPoolChain* chain = dpChainMap.poolChainMap.Value(poolId);
        DKASSERT_DEBUG(chain->device == this);
        DKASSERT_DEBUG(chain->poolId == poolId);

        // create layout!
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        DKArray<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.Reserve(layout.bindings.Count());
        for (const DKShaderBinding& binding : layout.bindings)
        {
            VkDescriptorSetLayoutBinding  layoutBinding = {};
            layoutBinding.binding = binding.binding;
            layoutBinding.descriptorType = DescriptorType(binding.type);
            layoutBinding.descriptorCount = binding.length;

            // input-attachment is for the fragment shader only! (framebuffer load operation)
            if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT &&
                layoutBinding.descriptorCount > 0)
                layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            else
                layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

            //TODO: setup immutable sampler!

            layoutBindings.Add(layoutBinding);
        }
        layoutCreateInfo.bindingCount = layoutBindings.Count();
        layoutCreateInfo.pBindings = layoutBindings;

        VkDescriptorSetLayoutSupport layoutSupport = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT };
        vkGetDescriptorSetLayoutSupport(device, &layoutCreateInfo, &layoutSupport);
        DKASSERT_DEBUG(layoutSupport.supported);

        VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
        VkResult result = vkCreateDescriptorSetLayout(device, &layoutCreateInfo, allocationCallbacks, &setLayout);
        if (result != VK_SUCCESS)
        {
            DKLogE("ERROR: vkCreateDescriptorSetLayout failed: %s", VkResultCStr(result));
            return NULL;
        }

        DescriptorPoolChain::AllocationInfo info;
        if (chain->AllocateDescriptorSet(setLayout, info))
        {
            DKASSERT_DEBUG(info.descriptorSet);
            DKASSERT_DEBUG(info.descriptorPool);

            DKObject<ShaderBindingSet> bindingSet = DKOBJECT_NEW ShaderBindingSet(dev, setLayout, info.descriptorSet, info.descriptorPool);
            bindingSet->bindings = std::move(layoutBindings);
            bindingSet->layoutFlags = layoutCreateInfo.flags; // copy layout creation flags! (for later use)
            return bindingSet.SafeCast<DKShaderBindingSet>();
        }

        vkDestroyDescriptorSetLayout(device, setLayout, allocationCallbacks);
    }
    return NULL;
}

void GraphicsDevice::DestroyDescriptorSet(VkDescriptorSet set, DescriptorPool* pool)
{
    DKASSERT_DEBUG(set);
    DKASSERT_DEBUG(pool);

    const DescriptorPoolId& poolId = pool->poolId;
    DKASSERT_DEBUG(poolId.mask);

    DKHashResult32 hash = DKHashCRC32(&poolId, sizeof(poolId));
    uint32_t index = hash.digest[9] % NumDescriptorPoolChainBuckets;

    DescriptorPoolChainMap& dpChainMap = descriptorPoolChainMaps[index];

    DKCriticalSection<DKSpinLock> guard(dpChainMap.lock);

    pool->ReleaseDescriptorSet(set);
    auto p = dpChainMap.poolChainMap.Find(poolId);
    DKASSERT_DEBUG(p);
    DescriptorPoolChain* chain = p->value;
    size_t numPools = chain->Cleanup();
    if (numPools == 0)
    {
        dpChainMap.poolChainMap.Remove(poolId);
        delete chain;
    }
}

DKObject<DKGpuBuffer> GraphicsDevice::CreateBuffer(DKGraphicsDevice* dev, size_t size, DKGpuBuffer::StorageMode storage, DKCpuCacheMode cache)
{
	if (size > 0)
	{
		VkBufferCreateInfo bufferCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCI.size = size;
		bufferCI.usage = 0x1ff;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer buffer = nullptr;
		VkResult result = vkCreateBuffer(device, &bufferCI, allocationCallbacks, &buffer);
		if (result == VK_SUCCESS)
		{
			VkBufferView view = nullptr;
			VkDeviceMemory memory = nullptr;
			VkMemoryRequirements memReqs;

			auto getMemoryTypeIndex = [this](uint32_t typeBits, VkMemoryPropertyFlags properties)
			{
				for (uint32_t i = 0; i < deviceMemoryTypes.Count(); ++i)
				{
					if ((typeBits & (1U << i)) && (deviceMemoryTypes.Value(i).propertyFlags & properties) == properties)
						return i;
				}
				DKASSERT_DEBUG(0);
				return uint32_t(-1);
			};

			VkMemoryPropertyFlags memProperties;
			switch (storage)
			{
			case DKGpuBuffer::StorageModeShared:
				memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				break;
			default:
				memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;
			}

			VkMemoryAllocateInfo memAllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
			vkGetBufferMemoryRequirements(device, buffer, &memReqs);
			memAllocInfo.allocationSize = memReqs.size;
			memAllocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, memProperties);

			result = vkAllocateMemory(device, &memAllocInfo, allocationCallbacks, &memory);
			if (result == VK_SUCCESS)
			{
				result = vkBindBufferMemory(device, buffer, memory, 0);
				if (result == VK_SUCCESS)
				{
					VkMemoryType memType = deviceMemoryTypes.Value(memAllocInfo.memoryTypeIndex);
					DKObject<Buffer> ret = DKOBJECT_NEW Buffer(dev, buffer, view, memory, memType, size);
					return ret.SafeCast<DKGpuBuffer>();
				}
				else
				{
					DKLogE("ERROR: vkBindBufferMemory failed: %s", VkResultCStr(result));
				}
			}
			else
			{
				DKLogE("ERROR: vkAllocateMemory failed: %s", VkResultCStr(result));
			}

			// clean up
			if (view)
				vkDestroyBufferView(device, view, allocationCallbacks);
			if (buffer)
				vkDestroyBuffer(device, buffer, allocationCallbacks);
			if (memory)
				vkFreeMemory(device, memory, allocationCallbacks);
		}
		else
		{
			DKLogE("ERROR: vkCreateBuffer failed: %s", VkResultCStr(result));
		}
	}
	return NULL;
}

DKObject<DKTexture> GraphicsDevice::CreateTexture(DKGraphicsDevice* dev, const DKTextureDescriptor& desc)
{
    VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    switch (desc.textureType)
    {
    case DKTexture::Type1D:
        imageCreateInfo.imageType = VK_IMAGE_TYPE_1D;
        break;
    case DKTexture::Type2D:
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        break;
    case DKTexture::Type3D:
        imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
        break;
    case DKTexture::TypeCube:
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        break;
    default:
        DKASSERT_DESC_DEBUG(0, "Invalid texture type!");
        DKLogE("Invalid texture type!");
        return NULL;
    }
    imageCreateInfo.arrayLayers = Max(desc.arrayLength, 1U);
    if (imageCreateInfo.arrayLayers > 1 && imageCreateInfo.imageType == VK_IMAGE_TYPE_2D)
    {
        imageCreateInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    }

    imageCreateInfo.format = PixelFormat(desc.pixelFormat);
    DKASSERT_DESC_DEBUG(imageCreateInfo.format != VK_FORMAT_UNDEFINED, "Unsupported format!");

    imageCreateInfo.extent = { desc.width, desc.height, desc.depth };
    imageCreateInfo.mipLevels = desc.mipmapLevels;

    DKASSERT_DESC_DEBUG(desc.sampleCount == 1, "Multisample is not implemented yet.");
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    
    if (desc.usage & DKTexture::UsageCopySource)
        imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (desc.usage & DKTexture::UsageCopyDestination)
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (desc.usage & (DKTexture::UsageShaderRead | DKTexture::UsageSampled))
        imageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (desc.usage & (DKTexture::UsageShaderWrite | DKTexture::UsageStorage))
        imageCreateInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (desc.usage & DKTexture::UsageRenderTarget)
    {
        if (DKPixelFormatIsDepthFormat(desc.pixelFormat))
            imageCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else
            imageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    // Set initial layout of the image to undefined
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkResult result = vkCreateImage(device, &imageCreateInfo, allocationCallbacks, &image);
    if (result != VK_SUCCESS)
    {
        DKLogE("ERROR: vkCreateImage failed: %s", VkResultCStr(result));
        return nullptr;
    }

    DKObject<Texture> texture = DKOBJECT_NEW Texture(dev, image, VK_NULL_HANDLE, &imageCreateInfo);

    if (imageCreateInfo.usage & (VK_IMAGE_USAGE_SAMPLED_BIT |
        VK_IMAGE_USAGE_STORAGE_BIT |
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        imageViewCreateInfo.image = image;

        switch (desc.textureType)
        {
        case DKTexture::Type1D:
            if (desc.arrayLength > 1)
                imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            else
                imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
            break;
        case DKTexture::Type2D:
            if (desc.arrayLength > 1)
                imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            else
                imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case DKTexture::Type3D:
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        case DKTexture::TypeCube:
            if (desc.arrayLength > 1)
                imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            else
                imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        }
        imageViewCreateInfo.format = imageCreateInfo.format;
        imageViewCreateInfo.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A 
        };

        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = imageCreateInfo.arrayLayers;
        imageViewCreateInfo.subresourceRange.levelCount = imageCreateInfo.mipLevels;

        result = vkCreateImageView(device, &imageViewCreateInfo, allocationCallbacks, &imageView);
        if (result != VK_SUCCESS)
        {
            DKLogE("ERROR: vkCreateImageView failed: %s", VkResultCStr(result));
            return nullptr;
        }

        texture->imageView = imageView;
    }

    return texture.SafeCast<DKTexture>();
}

DKObject<DKSamplerState> GraphicsDevice::CreateSamplerState(DKGraphicsDevice* dev, const DKSamplerDescriptor& desc)
{
    VkSamplerCreateInfo createInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

    auto filter = [](DKSamplerDescriptor::MinMagFilter f)->VkFilter
    {
        switch (f)
        {
        case DKSamplerDescriptor::MinMagFilterNearest:
            return VK_FILTER_NEAREST;
        case DKSamplerDescriptor::MinMagFilterLinear:
            return VK_FILTER_LINEAR;
        }
        return VK_FILTER_LINEAR;
    };
    auto mipmapMode = [](DKSamplerDescriptor::MipFilter f)->VkSamplerMipmapMode
    {
        switch (f)
        {
        case DKSamplerDescriptor::MipFilterNotMipmapped:
        case DKSamplerDescriptor::MipFilterNearest:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case DKSamplerDescriptor::MipFilterLinear:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    };
    auto addressMode = [](DKSamplerDescriptor::AddressMode m)->VkSamplerAddressMode
    {
        switch (m)
        {
        case DKSamplerDescriptor::AddressModeClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case DKSamplerDescriptor::AddressModeRepeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case DKSamplerDescriptor::AddressModeMirrorRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case DKSamplerDescriptor::AddressModeClampToZero:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    };
    auto compareOp = [](DKCompareFunction f)->VkCompareOp
    {
        switch (f)
        {
        case DKCompareFunctionNever:    return VK_COMPARE_OP_NEVER;
        case DKCompareFunctionLess:     return VK_COMPARE_OP_LESS;
        case DKCompareFunctionEqual:    return VK_COMPARE_OP_EQUAL;
        case DKCompareFunctionLessEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case DKCompareFunctionGreater:      return VK_COMPARE_OP_GREATER;
        case DKCompareFunctionNotEqual:     return VK_COMPARE_OP_NOT_EQUAL;
        case DKCompareFunctionGreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case DKCompareFunctionAlways:       return VK_COMPARE_OP_ALWAYS;
        }
        return VK_COMPARE_OP_NEVER;
    };

    createInfo.minFilter = filter(desc.minFilter);
    createInfo.magFilter = filter(desc.magFilter);
    createInfo.mipmapMode = mipmapMode(desc.mipFilter);
    createInfo.addressModeU = addressMode(desc.addressModeU);
    createInfo.addressModeV = addressMode(desc.addressModeV);
    createInfo.addressModeW = addressMode(desc.addressModeW);
    createInfo.mipLodBias = 0.0f;
    //createInfo.anisotropyEnable = desc.maxAnisotropy > 1 ? VK_TRUE : VK_FALSE;
    createInfo.anisotropyEnable = VK_TRUE;
    createInfo.maxAnisotropy = desc.maxAnisotropy;
    createInfo.compareOp = compareOp(desc.compareFunction);
    createInfo.compareEnable = createInfo.compareOp != VK_COMPARE_OP_NEVER;
    createInfo.minLod = desc.minLod;
    createInfo.maxLod = desc.maxLod;

    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

    createInfo.unnormalizedCoordinates = desc.normalizedCoordinates ? VK_FALSE : VK_TRUE;
    if (createInfo.unnormalizedCoordinates)
    {
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        createInfo.magFilter = createInfo.minFilter;
        createInfo.minLod = 0;
        createInfo.maxLod = 0;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.compareEnable = VK_FALSE;
    }

    VkSampler sampler = VK_NULL_HANDLE;
    VkResult result = vkCreateSampler(device, &createInfo, allocationCallbacks, &sampler);

    if (result != VK_SUCCESS)
    {
        DKLogE("ERROR: vkCreateSampler failed: %s", VkResultCStr(result));
        return nullptr;
    }

    DKObject<Sampler> s = DKOBJECT_NEW Sampler(dev, sampler);
    return s.SafeCast<DKSamplerState>();
}

DKObject<DKRenderPipelineState> GraphicsDevice::CreateRenderPipeline(DKGraphicsDevice* dev, const DKRenderPipelineDescriptor& desc, DKPipelineReflection* reflection)
{
	VkResult result = VK_SUCCESS;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;

    DKObject<RenderPipelineState> pipelineState = nullptr;

    Cleanup _cleanup = { DKFunction([&]() // cleanup operation, invoked if function failure.
    {
        if (!pipelineState)
        { 
            if (pipelineLayout != VK_NULL_HANDLE)
                vkDestroyPipelineLayout(device, pipelineLayout, allocationCallbacks);
            if (renderPass != VK_NULL_HANDLE)
                vkDestroyRenderPass(device, renderPass, allocationCallbacks);
            if (pipeline != VK_NULL_HANDLE)
                vkDestroyPipeline(device, pipeline, allocationCallbacks);
        }
    })->Invocation() };

	auto verifyShaderStage = [](const DKShaderFunction* fn, VkShaderStageFlagBits stage)->bool
	{
		if (fn)
		{
			DKASSERT_DEBUG(dynamic_cast<const ShaderFunction*>(fn) != nullptr);
			const ShaderFunction* func = static_cast<const ShaderFunction*>(fn);
			const ShaderModule* module = func->module.StaticCast<ShaderModule>();
			return module->stage == stage;
		}
		return false;
	};
	if (desc.vertexFunction)
	{
		DKASSERT_DEBUG(verifyShaderStage(desc.vertexFunction, VK_SHADER_STAGE_VERTEX_BIT));
	}
	if (desc.fragmentFunction)
	{
		DKASSERT_DEBUG(verifyShaderStage(desc.fragmentFunction, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	// shader stages
	DKArray<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;

	std::initializer_list<const DKShaderFunction*> shaderFunctions = { desc.vertexFunction, desc.fragmentFunction };
	shaderStageCreateInfos.Reserve(shaderFunctions.size());

	for (const DKShaderFunction* fn : shaderFunctions)
	{
		if (fn)
		{
			const ShaderFunction* func = static_cast<const ShaderFunction*>(fn);
			const ShaderModule* module = func->module.StaticCast<ShaderModule>();

			VkPipelineShaderStageCreateInfo shaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			shaderStageCreateInfo.stage = module->stage;
			shaderStageCreateInfo.module = module->module;
			shaderStageCreateInfo.pName = func->functionName;
			if (func->specializationInfo.mapEntryCount > 0)
				shaderStageCreateInfo.pSpecializationInfo = &func->specializationInfo;

			shaderStageCreateInfos.Add(shaderStageCreateInfo);
		}
	}
	pipelineCreateInfo.stageCount = (uint32_t)shaderStageCreateInfos.Count();
	pipelineCreateInfo.pStages = shaderStageCreateInfos;

    pipelineLayout = CreatePipelineLayout(shaderFunctions, VK_SHADER_STAGE_ALL);
    if (pipelineLayout == VK_NULL_HANDLE)
        return nullptr;
    pipelineCreateInfo.layout = pipelineLayout;

	// vertex input state
	DKArray<VkVertexInputBindingDescription> vertexBindingDescriptions;
	vertexBindingDescriptions.Reserve(desc.vertexDescriptor.layouts.Count());
	for (const DKVertexBufferLayoutDescriptor& bindingDesc : desc.vertexDescriptor.layouts)
	{
		VkVertexInputBindingDescription bind = {};
		bind.binding = bindingDesc.bufferIndex;
		bind.stride = bindingDesc.stride;
		switch (bindingDesc.step)
		{
		case DKVertexStepRate::Vertex:
			bind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; break;
		case DKVertexStepRate::Instance:
			bind.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE; break;
		default:
			DKASSERT_DEBUG(0);
		}
		vertexBindingDescriptions.Add(bind);
	}

	DKArray<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
	vertexAttributeDescriptions.Reserve(desc.vertexDescriptor.attributes.Count());
	for (const DKVertexAttributeDescriptor& attrDesc : desc.vertexDescriptor.attributes)
	{
		VkVertexInputAttributeDescription attr = {};
		attr.location = attrDesc.location;
		attr.binding = attrDesc.bufferIndex;
		attr.format = VertexFormat(attrDesc.format);
		attr.offset = attrDesc.offset;
		vertexAttributeDescriptions.Add(attr);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexBindingDescriptions.Count();
	vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptions;
	vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexAttributeDescriptions.Count();
	vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptions;
	pipelineCreateInfo.pVertexInputState = &vertexInputState;

	// input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	switch (desc.primitiveTopology)
	{
	case DKPrimitiveType::Point:
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;	break;
	case DKPrimitiveType::Line:
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;	break;
	case DKPrimitiveType::LineStrip:
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;	break;
	case DKPrimitiveType::Triangle:
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	break;
	case DKPrimitiveType::TriangleStrip:
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;	break;
	default:
		DKLogE("ERROR: DKGraphicsDevice::CreateRenderPipiline: Unknown PrimitiveTopology"); break;
	}
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;

	// setup viewport
	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = viewportState.scissorCount = 1;
	pipelineCreateInfo.pViewportState = &viewportState;

	// rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	if (desc.triangleFillMode == DKTriangleFillMode::Lines)
	{
		if (this->features.fillModeNonSolid)
			rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		else
			DKLogW("Warning: DKGraphicsDevice::CreateRenderPipiline: PolygonFillMode not supported for this hardware.");
	}

	switch (desc.cullMode)
	{
	case DKCullMode::None:	rasterizationState.cullMode = VK_CULL_MODE_NONE; break;
	case DKCullMode::Front:	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT; break;
	case DKCullMode::Back:	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT; break;
	default:
		DKLogE("ERROR: DKGraphicsDevice::CreateRenderPipiline: Unknown Cull-Mode"); break;
		rasterizationState.cullMode = VK_CULL_MODE_FRONT_AND_BACK; break;
	}

	switch (desc.frontFace)
	{
	case DKFrontFace::CW:		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE; break;
	case DKFrontFace::CCW:		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; break;
	default:
		DKLogE("ERROR: DKGraphicsDevice::CreateRenderPipiline: Unknown FrontFace-Mode"); break;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	rasterizationState.depthClampEnable = VK_FALSE;
	if (desc.depthClipMode == DKDepthClipMode::Clamp)
	{
		if (this->features.depthClamp)
			rasterizationState.depthClampEnable = VK_TRUE;
		else
			DKLogW("Warning: DKGraphicsDevice::CreateRenderPipiline: DepthClamp not supported for this hardware.");
	}
	rasterizationState.rasterizerDiscardEnable = desc.rasterizationEnabled ? VK_FALSE : VK_TRUE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;

	// setup multisampling
	VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.pSampleMask = nullptr;
	pipelineCreateInfo.pMultisampleState = &multisampleState;

	// setup depth-stencil
	VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;


	// dynamic states
	VkDynamicState dynamicStateEnables[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_DEPTH_BIAS,
		VK_DYNAMIC_STATE_BLEND_CONSTANTS,
		VK_DYNAMIC_STATE_DEPTH_BOUNDS,
		VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
		VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
		VK_DYNAMIC_STATE_STENCIL_REFERENCE,
	};
	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.pDynamicStates = dynamicStateEnables;
	dynamicState.dynamicStateCount = (uint32_t)std::size(dynamicStateEnables);
	pipelineCreateInfo.pDynamicState = &dynamicState;

	// render pass
	VkRenderPassCreateInfo  renderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	VkSubpassDescription subpassDesc = { 0, VK_PIPELINE_BIND_POINT_GRAPHICS };
	DKArray<VkAttachmentDescription> attachmentDescriptions;
	DKArray<VkAttachmentReference> subpassInputAttachmentRefs;
	DKArray<VkAttachmentReference> subpassColorAttachmentRefs;
	DKArray<VkAttachmentReference> subpassResolveAttachmentRefs;
	DKArray<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
	VkAttachmentReference subpassDepthStencilAttachment = { VK_ATTACHMENT_UNUSED };

	attachmentDescriptions.Reserve(desc.colorAttachments.Count() + 1);
	subpassColorAttachmentRefs.Reserve(desc.colorAttachments.Count());
	colorBlendAttachmentStates.Reserve(desc.colorAttachments.Count());


	auto GetBlendOperation = [](DKBlendOperation o)->VkBlendOp
	{
		switch (o)
		{
		case DKBlendOperation::Add:					return VK_BLEND_OP_ADD;
		case DKBlendOperation::Subtract:			return VK_BLEND_OP_SUBTRACT;
		case DKBlendOperation::ReverseSubtract:		return VK_BLEND_OP_REVERSE_SUBTRACT;
		case DKBlendOperation::Min:					return VK_BLEND_OP_MIN;
		case DKBlendOperation::Max:					return VK_BLEND_OP_MAX;
		}
		DKASSERT_DEBUG(0);
		return VkBlendOp(0);
	};
	auto GetBlendFactor = [](DKBlendFactor f)->VkBlendFactor
	{
		switch (f)
		{
		case DKBlendFactor::Zero:						return VK_BLEND_FACTOR_ZERO;
		case DKBlendFactor::One:						return VK_BLEND_FACTOR_ONE;
		case DKBlendFactor::SourceColor:				return VK_BLEND_FACTOR_SRC_COLOR;
		case DKBlendFactor::OneMinusSourceColor:		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case DKBlendFactor::SourceAlpha:				return VK_BLEND_FACTOR_SRC_ALPHA;
		case DKBlendFactor::OneMinusSourceAlpha:		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case DKBlendFactor::DestinationColor:			return VK_BLEND_FACTOR_DST_COLOR;
		case DKBlendFactor::OneMinusDestinationColor:	return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case DKBlendFactor::DestinationAlpha:			return VK_BLEND_FACTOR_DST_ALPHA;
		case DKBlendFactor::OneMinusDestinationAlpha:	return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case DKBlendFactor::SourceAlphaSaturated:		return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case DKBlendFactor::BlendColor:					return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case DKBlendFactor::OneMinusBlendColor:			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case DKBlendFactor::BlendAlpha:					return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case DKBlendFactor::OneMinusBlendAlpha:			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		}
		DKASSERT_DEBUG(0);
		return VkBlendFactor(0);
	};

	uint32_t colorAttachmentRefCount = 0;
	for (const DKRenderPipelineColorAttachmentDescriptor& attachment : desc.colorAttachments)
	{
		DKASSERT_DEBUG(DKPixelFormatIsColorFormat(attachment.pixelFormat));
		colorAttachmentRefCount = Max(colorAttachmentRefCount, attachment.index + 1);
	}
	if (colorAttachmentRefCount > this->properties.limits.maxColorAttachments)
	{
		DKLogE("ERROR: The number of colors attached exceeds the device limit. (%u > %u)",
			(uint32_t)colorAttachmentRefCount, (uint32_t)this->properties.limits.maxColorAttachments);
		return nullptr;
	}
	subpassColorAttachmentRefs.Add({ VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_GENERAL }, colorAttachmentRefCount);
	for (size_t index = 0; index < desc.colorAttachments.Count(); ++index)
	{
		const DKRenderPipelineColorAttachmentDescriptor& attachment = desc.colorAttachments.Value(index);

		VkAttachmentDescription attachmentDesc = {};
		attachmentDesc.format = PixelFormat(attachment.pixelFormat);
		attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDesc.loadOp = attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.storeOp = attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachmentDescriptions.Add(attachmentDesc);

		VkPipelineColorBlendAttachmentState blendState;
		blendState.blendEnable = attachment.blendingEnabled;
		blendState.srcColorBlendFactor = GetBlendFactor(attachment.sourceRGBBlendFactor);
		blendState.dstColorBlendFactor = GetBlendFactor(attachment.destinationRGBBlendFactor);
		blendState.colorBlendOp = GetBlendOperation(attachment.rgbBlendOperation);
		blendState.srcAlphaBlendFactor = GetBlendFactor(attachment.sourceAlphaBlendFactor);
		blendState.dstAlphaBlendFactor = GetBlendFactor(attachment.destinationAlphaBlendFactor);
		blendState.alphaBlendOp = GetBlendOperation(attachment.alphaBlendOperation);

		blendState.colorWriteMask = 0;
		if (attachment.writeMask & DKColorWriteMaskRed)		blendState.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
		if (attachment.writeMask & DKColorWriteMaskGreen)	blendState.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
		if (attachment.writeMask & DKColorWriteMaskBlue)	blendState.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
		if (attachment.writeMask & DKColorWriteMaskAlpha)	blendState.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentStates.Add(blendState);

		DKASSERT_DEBUG(subpassColorAttachmentRefs.Count() > attachment.index);
		VkAttachmentReference& attachmentRef = subpassColorAttachmentRefs.Value(attachment.index);
		attachmentRef.attachment = (uint32_t)index; // index of render-pass-attachment 
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	subpassDesc.colorAttachmentCount = (uint32_t)subpassColorAttachmentRefs.Count();
	subpassDesc.pColorAttachments = subpassColorAttachmentRefs;
	subpassDesc.pResolveAttachments = subpassResolveAttachmentRefs;
	subpassDesc.inputAttachmentCount = (uint32_t)subpassInputAttachmentRefs.Count();
	subpassDesc.pInputAttachments = subpassInputAttachmentRefs;
	if (DKPixelFormatIsDepthFormat(desc.depthStencilAttachmentPixelFormat))
	{
		subpassDesc.pDepthStencilAttachment = &subpassDepthStencilAttachment;
	}

	renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescriptions.Count();
	renderPassCreateInfo.pAttachments = attachmentDescriptions;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDesc;

	result = vkCreateRenderPass(device, &renderPassCreateInfo, allocationCallbacks, &renderPass);
	if (result != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateRenderPass failed: %s", VkResultCStr(result));
		return nullptr;
	}
	pipelineCreateInfo.renderPass = renderPass;

	// color blending
	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = (uint32_t)colorBlendAttachmentStates.Count();
	colorBlendState.pAttachments = colorBlendAttachmentStates;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;

	result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, allocationCallbacks, &pipeline);
	if (result == VK_SUCCESS)
		SavePipelineCache();

	if (result != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateGraphicsPipelines failed: %s", VkResultCStr(result));
		return nullptr;
	}
	if (reflection)
	{
		size_t maxResourceCount = 0;
		size_t maxPushConstantLayoutCount = 0;

		for (const DKShaderFunction* fn : shaderFunctions)
		{
			if (fn)
			{
				const ShaderFunction* func = static_cast<const ShaderFunction*>(fn);
				const ShaderModule* module = func->module.StaticCast<ShaderModule>();
				maxResourceCount += module->resources.Count();
				maxPushConstantLayoutCount += module->pushConstantLayouts.Count();

				if (module->stage == VK_SHADER_STAGE_VERTEX_BIT)
				{
                    reflection->inputAttributes.Reserve(module->inputAttributes.Count());
                    for (const DKShaderAttribute& attr : module->inputAttributes)
                    {
                        if (attr.enabled)
                            reflection->inputAttributes.Add(attr);
                    }
				}
			}
		}

		reflection->resources.Clear();
		reflection->pushConstantLayouts.Clear();

		reflection->resources.Reserve(maxResourceCount);
		reflection->pushConstantLayouts.Reserve(maxPushConstantLayoutCount);

		for (const DKShaderFunction* fn : shaderFunctions)
		{
			if (fn)
			{
				const ShaderFunction* func = static_cast<const ShaderFunction*>(fn);
				const ShaderModule* module = func->module.StaticCast<ShaderModule>();

				uint32_t stageMask = static_cast<uint32_t>(func->Stage());
                for (const DKShaderResource& res : module->resources)
                {
                    if (!res.enabled)
                        continue;

                    bool exist = false;
                    for (DKShaderResource& res2 : reflection->resources)
                    {
                        if (res.set == res2.set && res.binding == res2.binding)
                        {
                            DKASSERT(res.type == res2.type);
                            res2.stages |= stageMask;
                            exist = true;
                            break;
                        }
                    }
                    if (!exist)
                    {
                        DKShaderResource res2 = res;
                        res2.stages = stageMask;
                        reflection->resources.Add(res2);
                    }
                }
				for (const DKShaderPushConstantLayout& layout : module->pushConstantLayouts)
				{
					bool exist = false;
					for (DKShaderPushConstantLayout& l2 : reflection->pushConstantLayouts)
					{
						if (l2.offset == layout.offset && l2.size == layout.size)
						{
							l2.stages |= stageMask;
                            exist = true;
							break;
						}
					}
					if (!exist)
					{
						DKShaderPushConstantLayout l2 = layout;
						l2.stages = stageMask;
						reflection->pushConstantLayouts.Add(l2);
					}
				}
			}
		}

		reflection->inputAttributes.ShrinkToFit();
		reflection->resources.ShrinkToFit();
		reflection->pushConstantLayouts.ShrinkToFit();
	}

	pipelineState = DKOBJECT_NEW RenderPipelineState(dev, pipeline, pipelineLayout, renderPass);
	return pipelineState.SafeCast<DKRenderPipelineState>();
}

DKObject<DKComputePipelineState> GraphicsDevice::CreateComputePipeline(DKGraphicsDevice* dev, const DKComputePipelineDescriptor& desc, DKPipelineReflection* reflection)
{
    VkResult result = VK_SUCCESS;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    DKObject<ComputePipelineState> pipelineState = nullptr;

    Cleanup _cleanup = { DKFunction([&]() // cleanup operation, invoked if function failure.
    {
        if (!pipelineState)
        {
            if (pipelineLayout != VK_NULL_HANDLE)
                vkDestroyPipelineLayout(device, pipelineLayout, allocationCallbacks);
            if (pipeline != VK_NULL_HANDLE)
                vkDestroyPipeline(device, pipeline, allocationCallbacks);
        }
    })->Invocation() };

    VkComputePipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };

    if (desc.disableOptimization)
        pipelineCreateInfo.flags |= VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    //if (desc.deferCompile)
    //    pipelineCreateInfo.flags |= VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NVX;

    if (!desc.computeFunction)
        return nullptr; // compute function must be provided.

    DKASSERT_DEBUG(desc.computeFunction.SafeCast<ShaderFunction>() != nullptr);
    const ShaderFunction* func = desc.computeFunction.StaticCast<ShaderFunction>();
    const ShaderModule* module = func->module.StaticCast<ShaderModule>();
    DKASSERT_DEBUG(module->stage == VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    shaderStageCreateInfo.stage = module->stage;
    shaderStageCreateInfo.module = module->module;
    shaderStageCreateInfo.pName = func->functionName;
    if (func->specializationInfo.mapEntryCount > 0)
        shaderStageCreateInfo.pSpecializationInfo = &func->specializationInfo;

    pipelineCreateInfo.stage = shaderStageCreateInfo;

    pipelineLayout = CreatePipelineLayout({ func }, VK_SHADER_STAGE_ALL);
    if (pipelineLayout == VK_NULL_HANDLE)
        return nullptr;

    pipelineCreateInfo.layout = pipelineLayout;
    DKASSERT_DEBUG(pipelineCreateInfo.stage.stage == VK_SHADER_STAGE_COMPUTE_BIT);

    result = vkCreateComputePipelines(device, pipelineCache, 1, &pipelineCreateInfo, allocationCallbacks, &pipeline);
    if (result == VK_SUCCESS)
        SavePipelineCache();

    if (result != VK_SUCCESS)
    {
        DKLogE("ERROR: vkCreateComputePipelines failed: %s", VkResultCStr(result));
        return nullptr;
    }

    if (reflection)
    {
        reflection->resources.Clear();
        reflection->resources.Add(module->resources);
        reflection->resources.ShrinkToFit();
    }

    pipelineState = DKOBJECT_NEW ComputePipelineState(dev, pipeline, pipelineLayout);
	return pipelineState.SafeCast<DKComputePipelineState>();
}

VkFence GraphicsDevice::GetFence()
{
	VkFence fence = VK_NULL_HANDLE;

	if (fence == VK_NULL_HANDLE)
	{
		DKCriticalSection<DKCondition> guard(fenceCompletionCond);
		if (reusableFences.Count() > 0)
		{
			fence = reusableFences.Value(0);
			reusableFences.Remove(0);
		}
	}
	if (fence == VK_NULL_HANDLE)
	{
		VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		VkResult err = vkCreateFence(device, &fenceCreateInfo, allocationCallbacks, &fence);
		if (err != VK_SUCCESS)
		{
			DKLogE("ERROR: vkCreateFence failed: %s", VkResultCStr(err));
			DKASSERT(err == VK_SUCCESS);
		}
		numberOfFences.Increment();
		DKLogD("Queue Completion Helper: Num-Fences: %llu", (uint64_t)numberOfFences);
	}
	return fence;
}

void GraphicsDevice::AddFenceCompletionHandler(VkFence fence, DKOperation* op, bool useEventLoop)
{
	DKASSERT_DEBUG(fence != VK_NULL_HANDLE);

	if (op)
	{
		DKCriticalSection<DKCondition> guard(fenceCompletionCond);
		FenceCallback cb = { fence, op, 0 };
		if (useEventLoop)
			cb.threadId = DKThread::CurrentThreadId();
		pendingFenceCallbacks.Add(cb);
		fenceCompletionCond.Broadcast();
	}
}

void GraphicsDevice::FenceCompletionCallbackThreadProc()
{
	const double fenceWaitInterval = 0.002;

	VkResult err = VK_SUCCESS;

	DKArray<VkFence> fences;
	DKArray<FenceCallback> waitingFences;
	struct ThreadCallback
	{
		DKObject<DKOperation> operation;
		DKThread::ThreadId threadId;
	};
	DKArray<ThreadCallback> availableCallbacks;

	DKLogI("Vulkan Queue Completion Helper thread is started.");

	DKCriticalSection<DKCondition> guard(fenceCompletionCond);
	while (fenceCompletionThreadRunning)
	{
		waitingFences.Add(pendingFenceCallbacks);
		pendingFenceCallbacks.Clear();

		if (waitingFences.Count() > 0)
		{
			// condition is unlocked from here.
			fenceCompletionCond.Unlock();

			fences.Clear();
			fences.Reserve(waitingFences.Count());
			for (FenceCallback& cb : waitingFences)
				fences.Add(cb.fence);

			DKASSERT_DEBUG(fences.Count() > 0);
			err = vkWaitForFences(device,
								  static_cast<uint32_t>(fences.Count()),
								  fences,
								  VK_FALSE,
								  0);
			fences.Clear();
			if (err == VK_SUCCESS)
			{
				DKArray<FenceCallback> waitingFencesCopy;
				waitingFencesCopy.Reserve(waitingFences.Count());

				// check state for each fences
				for (FenceCallback& cb : waitingFences)
				{
					if (vkGetFenceStatus(device, cb.fence) == VK_SUCCESS)
					{
						fences.Add(cb.fence);
						availableCallbacks.Add({ cb.operation, cb.threadId });
					}
					else
						waitingFencesCopy.Add(cb); // fence is not ready (unsignaled)
				}
				// save unsignaled fences
				waitingFences.Clear();
				waitingFences = std::move(waitingFencesCopy);

				// reset signaled fences
				if (fences.Count() > 0)
				{
					err = vkResetFences(device, static_cast<uint32_t>(fences.Count()), fences);
					if (err != VK_SUCCESS)
					{
						DKLogE("ERROR: vkResetFences failed: %s", VkResultCStr(err));
						DKASSERT(err == VK_SUCCESS);
					}
				}
			}
			else if (err != VK_TIMEOUT)
			{
				DKLogE("ERROR: vkWaitForFences failed: %s", VkResultCStr(err));
				DKASSERT(0);
			}			

			if (availableCallbacks.Count() > 0)
			{
				for (ThreadCallback& tc : availableCallbacks)
				{
					DKObject<DKEventLoop> eventLoop = NULL;
					if (tc.threadId != 0)
						eventLoop = DKEventLoop::EventLoopForThreadId(tc.threadId);
					if (eventLoop)
						eventLoop->Post(tc.operation);
					else
						tc.operation->Perform();
				}
				availableCallbacks.Clear();
			}

			// lock condition (requires to reset fences mutually exclusive)
			fenceCompletionCond.Lock();
			if (fences.Count() > 0)
			{
				reusableFences.Add(fences);
				fences.Clear();
			}
			if (err == VK_TIMEOUT)
			{
				if (fenceWaitInterval > 0.0)
					fenceCompletionCond.WaitTimeout(fenceWaitInterval);
				else
					DKThread::Yield();
			}
		}
		else
		{
			fenceCompletionCond.Wait();
		}
	}

	DKLogI("Vulkan Queue Completion Helper thread is finished.");
}

void GraphicsDevice::LoadPipelineCache()
{
	if (this->pipelineCache != VK_NULL_HANDLE)
	{
		vkDestroyPipelineCache(this->device, this->pipelineCache, this->allocationCallbacks);
		this->pipelineCache = VK_NULL_HANDLE;
	}

	DKArray<unsigned char> buffer;
	VkPipelineCacheCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };

	DKPropertySet& defaultSet = DKPropertySet::DefaultSet();
	defaultSet.LookUpValueForKeyPath(DKString(savedSystemStates).Append(".").Append(savedPipelineCacheDataKey),
									 DKFunction([&](const DKVariant& var)->bool
	{
		if (var.ValueType() == DKVariant::TypeData)
		{
			const DKVariant::VData& data = var.Data();
			buffer.Clear();
			if (data.Length() > 0)
			{
				const void* p = data.LockShared();
				buffer.Resize(data.Length());
				memcpy((void*)buffer, p, data.Length());
				data.UnlockShared();

				pipelineCreateInfo.initialDataSize = buffer.Count();
				pipelineCreateInfo.pInitialData = buffer;
				return true;
			}
		}
		return false;
	}));
	
	VkResult result = vkCreatePipelineCache(this->device, &pipelineCreateInfo, allocationCallbacks, &this->pipelineCache);
	if (result != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreatePipelineCache failed: %s", VkResultCStr(result));
	}
}

void GraphicsDevice::SavePipelineCache()
{
	if (this->pipelineCache != VK_NULL_HANDLE)
	{
		DKArray<unsigned char> buffer;
		VkResult result = VK_SUCCESS;
		do
		{
			size_t dataLength = 0;
			result = vkGetPipelineCacheData(this->device, this->pipelineCache, &dataLength, 0);
			if (result != VK_SUCCESS)
				break;

			buffer.Resize(dataLength + 4);
			if (dataLength > 0)
			{
				result = vkGetPipelineCacheData(this->device, this->pipelineCache, &dataLength, (void*)buffer);
				if (result != VK_SUCCESS)
					break;
			}

			buffer.Resize(dataLength);
			DKASSERT_DEBUG(buffer.Count() == dataLength);

		} while (0);
		if (result == VK_SUCCESS)
		{
			DKPropertySet& defaultSet = DKPropertySet::DefaultSet();
			defaultSet.ReplaceValue(savedSystemStates, DKFunction([&](const DKVariant& v)->DKVariant
			{
				DKVariant var;
				if (v.ValueType() == DKVariant::TypePairs)
					var.SetValue(v.Pairs());
				else
					var.SetValueType(DKVariant::TypePairs);

				DKVariant cachedData(DKVariant::TypeData);
				cachedData.SetData((const void*)buffer, buffer.Count());
				DKVariant::VPairs& pairs = var.Pairs();
				pairs.Update(savedPipelineCacheDataKey, cachedData);
				return var;
			}));
		}
		else
		{
			DKLogE("ERROR: vkGetPipelineCacheData failed: %s", VkResultCStr(result));
		}
	}
	else
	{
		DKLogE("ERROR: VkPipelineCache is NULL");
	}
}

VkPipelineLayout GraphicsDevice::CreatePipelineLayout(std::initializer_list<const DKShaderFunction*> functions, VkShaderStageFlags layoutDefaultStageFlags) const
{
    DKArray<VkDescriptorSetLayout> descriptorSetLayouts;
    auto result = CreatePipelineLayout(functions, descriptorSetLayouts, layoutDefaultStageFlags);

    for (VkDescriptorSetLayout setLayout : descriptorSetLayouts)
    {
        DKASSERT_DEBUG(setLayout != VK_NULL_HANDLE);
        vkDestroyDescriptorSetLayout(device, setLayout, allocationCallbacks);
    }
    return result;
}

VkPipelineLayout GraphicsDevice::CreatePipelineLayout(std::initializer_list<const DKShaderFunction*> functions, DKArray<VkDescriptorSetLayout>& descriptorSetLayouts, VkShaderStageFlags layoutDefaultStageFlags) const
{
    VkResult result = VK_SUCCESS;

    size_t numPushConstantRanges = 0;
    for (const DKShaderFunction* fn : functions)
    {
        DKASSERT_DEBUG(fn);
        DKASSERT_DEBUG(dynamic_cast<const ShaderFunction*>(fn));

        const ShaderFunction* func = static_cast<const ShaderFunction*>(fn);
        const ShaderModule* module = func->module.StaticCast<ShaderModule>();

        numPushConstantRanges += module->pushConstantLayouts.Count();
    }

    DKArray<VkPushConstantRange> pushConstantRanges;
    pushConstantRanges.Reserve(numPushConstantRanges);

    size_t maxDescriptorBindings = 0;   // maximum number of descriptor
    uint32_t maxDescriptorSets = 0;     // maximum number of sets

    for (const DKShaderFunction* fn : functions)
    {
        const ShaderFunction* func = static_cast<const ShaderFunction*>(fn);
        const ShaderModule* module = func->module.StaticCast<ShaderModule>();

        for (const DKShaderPushConstantLayout& layout : module->pushConstantLayouts)
        {
            if (layout.size > 0)
            {
                VkPushConstantRange range = {};
                range.stageFlags = module->stage;
                range.offset = layout.offset;
                range.size = layout.size;
                pushConstantRanges.Add(range);
            }
        }

        // calculate max descriptor bindings a set
        if (module->descriptors.Count() > 0)
        {
            maxDescriptorSets = Max(maxDescriptorSets,
                                    module->descriptors.Value(module->descriptors.Count() - 1).set + 1);
            maxDescriptorBindings = Max(maxDescriptorBindings, module->descriptors.Count());
        }
    }

    // setup descriptor layout
    DKArray<VkDescriptorSetLayoutBinding> descriptorBindings;
    descriptorBindings.Reserve(maxDescriptorBindings);

    for (uint32_t setIndex = 0; setIndex < maxDescriptorSets; ++setIndex)
    {
        descriptorBindings.Clear();
        for (const DKShaderFunction* fn : functions)
        {
            if (fn)
            {
                const ShaderFunction* func = static_cast<const ShaderFunction*>(fn);
                const ShaderModule* module = func->module.StaticCast<ShaderModule>();

                for (const DKShader::Descriptor& desc : module->descriptors)
                {
                    if (desc.set > setIndex)
                        break;

                    if (desc.set == setIndex)
                    {
                        bool newBinding = true;

                        for (VkDescriptorSetLayoutBinding& b : descriptorBindings)
                        {
                            if (b.binding == desc.binding) // exist binding!! (conflict)
                            {
                                newBinding = false;
                                if (b.descriptorType == desc.type)
                                {
                                    b.descriptorCount = Max(b.descriptorCount, desc.count);
                                    b.stageFlags |= module->stage;
                                }
                                else
                                {
                                    DKLogE("ERROR: descriptor binding conflict! (set=%d, binding=%u)",
                                           setIndex, desc.binding);
                                    return nullptr;
                                }
                            }
                        }
                        if (newBinding)
                        {
                            VkDescriptorType type = DescriptorType(desc.type);
                            VkDescriptorSetLayoutBinding binding = {
                                desc.binding,
                                type,
                                desc.count,
                                layoutDefaultStageFlags | module->stage, 
                                nullptr  /* VkSampler* pImmutableSamplers */
                            };
                            descriptorBindings.Add(binding);
                        }
                    }
                }
            }
        }
        // create descriptor set (setIndex) layout
        VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        setLayoutCreateInfo.bindingCount = (uint32_t)descriptorBindings.Count();
        setLayoutCreateInfo.pBindings = descriptorBindings;

        VkDescriptorSetLayoutSupport layoutSupport = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT };
        vkGetDescriptorSetLayoutSupport(device, &setLayoutCreateInfo, &layoutSupport);
        DKASSERT_DEBUG(layoutSupport.supported);

        VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
        result = vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, allocationCallbacks, &setLayout);
        if (result != VK_SUCCESS)
        {
            DKLogE("ERROR: vkCreateDescriptorSetLayout failed: %s", VkResultCStr(result));
            return VK_NULL_HANDLE;
        }
        descriptorSetLayouts.Add(setLayout);
        descriptorBindings.Clear();
    }
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.Count();
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
    pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.Count();
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
     result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, allocationCallbacks, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        DKLogE("ERROR: vkCreatePipelineLayout failed: %s", VkResultCStr(result));
        return VK_NULL_HANDLE;
    }
    return pipelineLayout;
}

#endif //#if DKGL_ENABLE_VULKAN
