//
//  File: ComputeCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKComputeCommandEncoder.h"
#include "CommandBuffer.h"
#include "ComputePipelineState.h"
#include "ShaderBindingSet.h"

namespace DKFramework::Private::Vulkan
{
	class ComputeCommandEncoder : public DKComputeCommandEncoder
	{
	public:
		ComputeCommandEncoder(VkCommandBuffer, class CommandBuffer*);
		~ComputeCommandEncoder();

		void EndEncoding() override;
		bool IsCompleted() const override { return false; }
		DKCommandBuffer* CommandBuffer() override;

        void SetResources(uint32_t set, DKShaderBindingSet*) override;
        void SetComputePipelineState(DKComputePipelineState*) override;

        struct Resources
        {
            ComputePipelineState* pipelineState;
            DKMap<uint32_t, ShaderBindingSet*> updateResources; // have flag that 'update after bind'
            DKMap<uint32_t, ShaderBindingSet*> unboundResources;

            DKArray<VkSemaphore>			waitSemaphores;
            DKArray<VkPipelineStageFlags>	waitStageMasks;
            DKArray<VkSemaphore>			signalSemaphores;

            class CommandBuffer* cb;
            VkCommandBuffer commandBuffer;

            Resources(class CommandBuffer*);
            ~Resources();

            // Retain ownership of all encoded objects
            DKArray<DKObject<ComputePipelineState>> pipelineStateObjects;
            DKArray<DKObject<ShaderBindingSet>> shaderBindingSets;
        };

        void AddWaitSemaphore(VkSemaphore, VkPipelineStageFlags);
        void AddSignalSemaphore(VkSemaphore);

        DKMap<VkSemaphore, VkPipelineStageFlags> semaphorePipelineStageMasks;
        DKSet<VkSemaphore> signalSemaphores;

        DKObject<Resources> resources;
		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
