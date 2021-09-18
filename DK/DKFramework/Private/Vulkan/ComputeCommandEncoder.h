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
        class Encoder;
        struct EncodingState
        {
            Encoder* encoder;
            ComputePipelineState* pipelineState;
            ShaderBindingSet::ImageLayoutMap imageLayoutMap;
            ShaderBindingSet::ImageViewLayoutMap imageViewLayoutMap;
            DKMap<ShaderBindingSet*, DescriptorSet*> bindingSetMap;
        };
        using EncoderCommand = DKFunctionSignature<void(VkCommandBuffer, EncodingState&)>;
        class Encoder : public CommandEncoder
        {
        public:
            Encoder(class CommandBuffer*);
            ~Encoder();

            bool Encode(VkCommandBuffer) override;

            // Retain ownership of all encoded objects
            DKArray<DKObject<ComputePipelineState>> pipelineStateObjects;
            DKArray<DKObject<ShaderBindingSet>> shaderBindingSets;
            DKArray<DKObject<DescriptorSet>> descriptorSets;
            DKArray<DKObject<DKGpuEvent>> events;
            DKArray<DKObject<DKGpuSemaphore>> semaphores;

            class CommandBuffer* commandBuffer;
            DKArray<DKObject<EncoderCommand>> commands;
            DKArray<DKObject<EncoderCommand>> setupCommands;
            DKArray<DKObject<EncoderCommand>> cleanupCommands;
        };
        DKObject<Encoder> encoder;

	public:
		ComputeCommandEncoder(class CommandBuffer*);

        void EndEncoding() override;
        bool IsCompleted() const override { return encoder == nullptr; }
        DKCommandBuffer* CommandBuffer() override { return commandBuffer; }

        void WaitEvent(const DKGpuEvent*) override;
        void SignalEvent(const DKGpuEvent*) override;
        void WaitSemaphoreValue(const DKGpuSemaphore*, uint64_t) override;
        void SignalSemaphoreValue(const DKGpuSemaphore*, uint64_t) override;

        void SetResources(uint32_t set, const DKShaderBindingSet*) override;
        void SetComputePipelineState(const DKComputePipelineState*) override;

        void PushConstant(uint32_t stages, uint32_t offset, uint32_t size, const void*) override;

        void Dispatch(uint32_t, uint32_t, uint32_t) override;

		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
