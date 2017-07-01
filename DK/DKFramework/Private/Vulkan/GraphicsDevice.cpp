//
//  File: GraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN

#include "GraphicsDevice.h"
#include "CommandQueue.h"
#include "ShaderFunction.h"
#include "PixelFormat.h"
#include "RenderPipelineState.h"
#include "../../DKPropertySet.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			DKGraphicsDeviceInterface* CreateInterface(void)
			{
				return DKRawPtrNew<GraphicsDevice>();
			}

			const char *validationLayerNames[] =
			{
				"VK_LAYER_LUNARG_standard_validation"
			};

			// extensions
			InstanceProc iproc;
			DeviceProc dproc;

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
				if (!DKIsDebuggerPresent())
					return VK_TRUE;
#endif
				return VK_FALSE;
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;


GraphicsDevice::GraphicsDevice(void)
	: instance(NULL)
	, device(NULL)
	, physicalDevice(NULL)
	, msgCallback(NULL)
	, enableValidation(false)
	, fenceCompletionThreadRunning(true)
	, numberOfFences(0)
{
#ifdef DKGL_DEBUG_ENABLED
	this->enableValidation = true;
#endif

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
			DKLog(" -- Layer: %s (\"%s\", %d / %d)",
				  prop.layerName,
				  prop.description,
				  prop.specVersion, 
				  prop.implementationVersion);
		}
	}

	VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
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

	VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
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
		throw std::runtime_error((const char*)DKStringU8::Format("vkCreateInstance failed: %s", VkResultCStr(err)));
	}
	// load instance extensions
	iproc.Load(instance);

	if (enableValidation)
	{
		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
		dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugMessageCallback;
		dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		
		err = iproc.vkCreateDebugReportCallbackEXT(instance, &dbgCreateInfo, nullptr, &msgCallback);
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
				VkQueueFamilyProperties& prop = desc.queueFamilyProperties.Value(j);
				DKLog(" -- Queue-Family[%llu] flag-bits:%c%c%c%c count:%d",
					j,
					prop.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT ? '1' : '0',
					prop.queueFlags & VK_QUEUE_TRANSFER_BIT ? '1' : '0',
					prop.queueFlags & VK_QUEUE_COMPUTE_BIT ? '1' : '0',
					prop.queueFlags & VK_QUEUE_GRAPHICS_BIT ? '1' : '0',
					prop.queueCount);
			}

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
			if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
				(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				VkDeviceQueueCreateInfo queueInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
				queueInfo.queueFamilyIndex = queueFamilyIndex;
				queueInfo.queueCount = queueFamily.queueCount;
				queueInfo.pQueuePriorities = (const float*)defaultQueuePriorities;
				queueCreateInfos.Add(queueInfo);
			}
		}
		DKASSERT_DEBUG(queueCreateInfos.Count() > 0);

		// setup device extensions
		DKArray<const char*> deviceExtensions;

		bool enableAllExtensions = true;
		if (enableAllExtensions)
		{
			deviceExtensions.Reserve(desc.extensionProperties.Count());
			for (VkExtensionProperties& ep : desc.extensionProperties)
				deviceExtensions.Add(ep.extensionName);
		}
		else
		{
			deviceExtensions.Add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			if (desc.IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
				deviceExtensions.Add(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		}

		VkPhysicalDeviceFeatures enabledFeatures = {};
		VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.Count());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		if (deviceExtensions.Count() > 0)
		{
			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.Count();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
		}

		VkDevice logicalDevice;

		err = vkCreateDevice(desc.physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
		if (err == VK_SUCCESS)
		{
			// initialize device extensions
			dproc.Load(logicalDevice);

			this->device = logicalDevice;
			this->physicalDevice = desc.physicalDevice;
			this->properties = desc.properties;
			this->features = desc.features;
			this->extensionProperties = desc.extensionProperties;

			// get queues
			this->queueFamilies.Reserve(desc.numQueues);

			for (const VkDeviceQueueCreateInfo& queueInfo : queueCreateInfos)
			{
				QueueFamily* qf = DKRawPtrNew<QueueFamily>(desc.physicalDevice, logicalDevice, queueInfo.queueFamilyIndex, queueInfo.queueCount, desc.queueFamilyProperties.Value(queueInfo.queueFamilyIndex));
				this->queueFamilies.Add(qf);
			}
			DKLogI("Vulkan device created with \"%s\"", desc.properties.deviceName);
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

	fenceCompletionThread = DKThread::Create(
		DKFunction(this, &GraphicsDevice::FenceCompletionCallbackThreadProc)->Invocation());
}

GraphicsDevice::~GraphicsDevice(void)
{
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
		vkDestroyFence(device, fence, nullptr);
	reusableFences.Clear();

	for (QueueFamily* family : queueFamilies)
	{
		DKRawPtrDelete(family);
	}
	queueFamilies.Clear();

	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, nullptr);
	if (msgCallback)
		iproc.vkDestroyDebugReportCallbackEXT(instance, msgCallback, nullptr);
}

DKString GraphicsDevice::DeviceName(void) const
{
	return DKString(properties.deviceName);
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

DKObject<DKShaderFunction> GraphicsDevice::CreateShaderFunction(DKGraphicsDevice* dev, DKShader* shader)
{
	DKASSERT_DEBUG(shader);
	if (shader->codeData)
	{
		DKDataReader reader(shader->codeData);
		if (reader.Length() > 0)
		{
			VkShaderModuleCreateInfo shaderModuleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			shaderModuleCreateInfo.codeSize = reader.Length();
			shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(reader.Bytes());

			VkShaderModule shaderModule;
			VkResult res = vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &shaderModule);
			if (res == VK_SUCCESS)
			{
				DKObject<ShaderFunction> function = DKOBJECT_NEW ShaderFunction(dev, shaderModule, reader.Bytes(), reader.Length(), shader->stage, (const DKStringU8&)shader->entryPoint);
				return function.SafeCast<DKShaderFunction>();
			}
		}
	}
	return NULL;
}

DKObject<DKRenderPipelineState> GraphicsDevice::CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKPipelineReflection*)
{
	return NULL;
}

DKObject<DKComputePipelineState> GraphicsDevice::CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*)
{
	return NULL;
}

VkFence GraphicsDevice::GetFence(void)
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
		VkResult err = vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
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

void GraphicsDevice::FenceCompletionCallbackThreadProc(void)
{
	const double idleTimerValue = 0.1; // idle timer resolution

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
			else
			{
				DKThread::Yield();
			}

			// lock condition (requires to reset fences mutually exclusive)
			fenceCompletionCond.Lock();
			if (fences.Count() > 0)
			{
				reusableFences.Add(fences);
				fences.Clear();
			}
		}
		else
		{
			if (idleTimerValue > 0.0)
				fenceCompletionCond.WaitTimeout(idleTimerValue);
			else
				fenceCompletionCond.Wait();
		}
	}

	DKLogI("Vulkan Queue Completion Helper thread is finished.");
}

#endif //#if DKGL_ENABLE_VULKAN
