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
				DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*, uint32_t) override;
				DKObject<DKShaderModule> CreateShaderModule(DKGraphicsDevice*, DKShader*) override;
				DKObject<DKRenderPipelineState> CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKPipelineReflection*) override;
				DKObject<DKComputePipelineState> CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*) override;
				DKObject<DKGpuBuffer> CreateBuffer(size_t, DKGpuStorageMode, DKCpuCacheMode);
				DKObject<DKTexture> CreateTexture(const DKTextureDescriptor&);

				VkFence GetFence(void);
				void AddFenceCompletionHandler(VkFence, DKOperation*, bool useEventLoop = false);

				VkInstance instance;
				VkDevice device;
				VkPhysicalDevice physicalDevice;

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
