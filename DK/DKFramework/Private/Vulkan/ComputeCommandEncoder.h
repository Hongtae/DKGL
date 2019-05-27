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

        void WaitEvent(DKGpuEvent*) override;
        void SignalEvent(DKGpuEvent*) override;

        void SetResources(uint32_t set, DKShaderBindingSet*) override;
        void SetComputePipelineState(DKComputePipelineState*) override;
        void Dispatch(uint32_t, uint32_t, uint32_t) override;

		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
