//
//  File: GraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2020 Hongtae Kim. All rights reserved.
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

// Note: (2020-02-08)
// Using timeline semaphores to signal on completion causes an error in the validation layer.
// The Validation layer uses fences instead of semaphores, and fences are later than semaphores in synchronization order.
// see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap6.html#synchronization-signal-operation-order
//
// Set DKGL_QUEUE_COMPLETION_SYNC_TIMELINE_SEMAPHORE = 1 to use timeline semaphore on queue submission.
#define DKGL_QUEUE_COMPLETION_SYNC_TIMELINE_SEMAPHORE    0

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

#if DKGL_QUEUE_COMPLETION_SYNC_TIMELINE_SEMAPHORE
        void SetQueueCompletionHandler(VkQueue, DKOperation*, VkSemaphore&, uint64_t&);
#else
        void AddFenceCompletionHandler(VkFence, DKOperation*);
        VkFence GetFence();
#endif

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

#if DKGL_QUEUE_COMPLETION_SYNC_TIMELINE_SEMAPHORE
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
       
        TimelineSemaphoreCounter deviceEventSemaphore;
        DKArray<QueueSubmissionSemaphore> queueCompletionSemaphoreHandlers;

        DKSpinLock queueCompletionHandlerLock;
        bool queueCompletionThreadRunning;
        DKObject<DKThread> queueCompletionThread;
        void QueueCompletionThreadProc();
#else
        struct FenceCallback
        {
            VkFence fence;
            DKObject<DKOperation> operation;
        };
        DKArray<FenceCallback> pendingFenceCallbacks;
        DKArray<VkFence> reusableFences;
        DKCondition fenceCompletionCond;
        bool fenceCompletionThreadRunning;
        DKObject<DKThread> fenceCompletionThread;
        void FenceCompletionCallbackThreadProc();
#endif

        // VK_EXT_debug_utils
        VkDebugUtilsMessengerEXT debugMessenger;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
