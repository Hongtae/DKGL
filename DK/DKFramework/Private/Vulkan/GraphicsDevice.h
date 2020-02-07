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
#include "DescriptorPool.h"
#include "DescriptorSet.h"

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
        DKObject<DKGpuEvent> CreateEvent(DKGraphicsDevice*) override;
        DKObject<DKGpuSemaphore> CreateSemaphore(DKGraphicsDevice*) override;

        DKString DeviceName() const override;

        DKObject<DescriptorSet> CreateDescriptorSet(DKGraphicsDevice*, VkDescriptorSetLayout, const DescriptorPoolId&);
        void DestroyDescriptorSets(DescriptorPool*, VkDescriptorSet*, size_t);

        void SetQueueCompletionHandler(VkQueue, DKOperation*, VkSemaphore&, uint64_t&);

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

        uint32_t IndexOfMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties)
        {
            for (uint32_t i = 0; i < deviceMemoryTypes.Count(); ++i)
            {
                if ((typeBits & (1U << i)) && (deviceMemoryTypes.Value(i).propertyFlags & properties) == properties)
                    return i;
            }
            DKASSERT_DEBUG(0);
            return uint32_t(-1);
        };

        VkPipelineCache pipelineCache;

        // timeline semaphore completion handlers
        struct TimelineSemaphoreCounter
        {
            VkSemaphore semaphore;
            uint64_t value; // signal value (from GPU)
        };
        struct TimelineSemaphoreCompletionHandler
        {
            uint64_t value;
            DKObject<DKOperation> operation;
        };
        struct QueueSubmissionSemaphore
        {
            VkQueue queue;
            TimelineSemaphoreCounter semaphore;
            uint64_t waitValue;
            DKArray<TimelineSemaphoreCompletionHandler> handlers;
        };
        bool queueCompletionThreadRunning;
        TimelineSemaphoreCounter deviceEventSemaphore;
        DKArray<QueueSubmissionSemaphore> queueCompletionSemaphoreHandlers;

        DKObject<DKThread> queueCompletionThread;
        DKSpinLock queueCompletionHandlerLock;
        void QueueCompletionThreadProc();

        // VK_EXT_debug_utils
        VkDebugUtilsMessengerEXT debugMessenger;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
