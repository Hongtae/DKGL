//
//  File: GraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"

#include "../../Interface/DKGraphicsDeviceInterface.h"
#include "QueueFamily.h"
#include "DescriptorPoolChain.h"

namespace DKFramework::Private::Vulkan
{
	class GraphicsDevice : public DKGraphicsDeviceInterface
	{
	public:
		GraphicsDevice();
		~GraphicsDevice();

		DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*, uint32_t) override;

		DKObject<DKRenderPipelineState> CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKPipelineReflection*) override;
		DKObject<DKComputePipelineState> CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*) override;

        DKObject<DKShaderModule> CreateShaderModule(DKGraphicsDevice*, DKShader*) override;
        DKObject<DKShaderBindingSet> CreateShaderBindingSet(DKGraphicsDevice*, const DKShaderBindingSetLayout&) override;

		DKObject<DKGpuBuffer> CreateBuffer(DKGraphicsDevice*, size_t, DKGpuBuffer::StorageMode, DKCpuCacheMode) override;
		DKObject<DKTexture> CreateTexture(DKGraphicsDevice*, const DKTextureDescriptor&) override;
        DKObject<DKSamplerState> CreateSamplerState(DKGraphicsDevice*, const DKSamplerDescriptor&) override;

        DKString DeviceName() const override;

        void DestroyDescriptorSet(VkDescriptorSet, DescriptorPool*);

		VkFence GetFence();
		void AddFenceCompletionHandler(VkFence, DKOperation*, bool useEventLoop = false);

		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice physicalDevice;

		DKAtomicNumber64 numberOfFences;
		DKArray<QueueFamily*> queueFamilies;

		// device properties
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		DKArray<VkMemoryType> deviceMemoryTypes;
		DKArray<VkMemoryHeap> deviceMemoryHeaps;
		DKArray<VkQueueFamilyProperties> queueFamilyProperties;
		DKArray<VkExtensionProperties> extensionProperties;

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
		void FenceCompletionCallbackThreadProc();

		VkPipelineCache pipelineCache;
		void LoadPipelineCache();
		void SavePipelineCache();

        VkAllocationCallbacks* allocationCallbacks;
		// extensions
		InstanceProc iproc; // instance procedure
		DeviceProc dproc; // device procedure

    private:
        VkPipelineLayout CreatePipelineLayout(std::initializer_list<const DKShaderFunction*>, VkShaderStageFlags) const;
        VkPipelineLayout CreatePipelineLayout(std::initializer_list<const DKShaderFunction*>, DKArray<VkDescriptorSetLayout>&, VkShaderStageFlags) const;

        enum { NumDescriptorPoolChainBuckets = 7 };
        struct DescriptorPoolChainMap
        {
            DKMap<DescriptorPoolId, DescriptorPoolChain*> poolChainMap;
            DKSpinLock lock;
        } descriptorPoolChainMaps[NumDescriptorPoolChainBuckets];
    };
}
#endif //#if DKGL_ENABLE_VULKAN
