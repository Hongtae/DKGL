//
//  File: ShaderBindingSet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKShaderBindingSet.h"
#include "BufferView.h"
#include "ImageView.h"
#include "Sampler.h"
#include "DescriptorSet.h"
#include "DescriptorPool.h"

namespace DKFramework::Private::Vulkan
{
    class ShaderBindingSet : public DKShaderBindingSet
    {
        const DescriptorPoolId poolId;
    public:
        ShaderBindingSet(DKGraphicsDevice*,
                         VkDescriptorSetLayout,                         
                         const DescriptorPoolId&,
                         const VkDescriptorSetLayoutCreateInfo&);
        ~ShaderBindingSet();

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSetLayoutCreateFlags layoutFlags;

        DKObject<DKGraphicsDevice> device;

        using DescriptorBinding = DescriptorSet::Binding;
        DKArray<DescriptorBinding> bindings;

        void SetBuffer(uint32_t binding, DKGpuBuffer*, uint64_t, uint64_t) override;
        void SetBufferArray(uint32_t binding, uint32_t numBuffers, BufferInfo*) override;
        void SetTexture(uint32_t binding, DKTexture*) override;
        void SetTextureArray(uint32_t binding, uint32_t numTextures, DKTexture**) override;
        void SetSamplerState(uint32_t binding, DKSamplerState*) override;
        void SetSamplerStateArray(uint32_t binding, uint32_t numSamplers, DKSamplerState**) override;

        DKObject<DescriptorSet> CreateDescriptorSet() const;

        DescriptorBinding* FindDescriptorBinding(uint32_t binding);
    };
}
#endif //#if DKGL_ENABLE_VULKAN
