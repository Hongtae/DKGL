//
//  File: GraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
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

				VkFence GetFence(void);
				void AddFenceCompletionHandler(VkFence, DKOperation*, bool useEventLoop = false);

				VkInstance instance;
				VkDevice device;
				VkPhysicalDevice physicalDevice;

			private:
				DKAtomicNumber64 numberOfFences;
				DKArray<QueueFamily*> queueFamilies;

				// device properties
				VkPhysicalDeviceProperties properties;
				VkPhysicalDeviceFeatures features;
				DKArray<VkExtensionProperties> extensionProperties;

				bool enableValidation;
				VkDebugReportCallbackEXT msgCallback;

				struct FenceCallback
				{
					VkFence fence;
					DKObject<DKOperation> operation;
					DKThread::ThreadId threadId;
				};
				DKArray<FenceCallback> pendingFenceCallbacks;
				DKArray<VkFence> reusableFences;
				DKCondition fenceCompletionCond;
				bool fenceCompletionThreadRunning;
				DKObject<DKThread> fenceCompletionThread;
				void FenceCompletionCallbackThreadProc(void);
			};
		}
	}
}
#endif //#if DKGL_ENABLE_VULKAN
