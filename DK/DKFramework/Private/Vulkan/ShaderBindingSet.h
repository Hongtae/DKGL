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

namespace DKFramework::Private::Vulkan
{
    class DescriptorPool;
    class ShaderBindingSet : public DKShaderBindingSet
    {
    public:
        ShaderBindingSet(DKGraphicsDevice*, VkDescriptorSetLayout, VkDescriptorSet, DescriptorPool*);
        ~ShaderBindingSet();

        VkDescriptorSet descriptorSet;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSetLayoutCreateFlags layoutFlags;

        DKObject<DescriptorPool> descriptorPool;
        DKObject<DKGraphicsDevice> device;
        DKArray<VkDescriptorSetLayoutBinding> bindings;

        void SetBuffer(uint32_t binding, DKGpuBuffer*, uint64_t, uint64_t) override;
        void SetBufferArray(uint32_t binding, uint32_t numBuffers, BufferInfo*) override;
        void SetTexture(uint32_t binding, DKTexture*) override;
        void SetTextureArray(uint32_t binding, uint32_t numTextures, DKTexture**) override;
        void SetSamplerState(uint32_t binding, DKSamplerState*) override;
        void SetSamplerStateArray(uint32_t binding, uint32_t numSamplers, DKSamplerState**) override;

        void UpdateDescriptorSet();
        bool FindDescriptorBinding(uint32_t binding, VkDescriptorSetLayoutBinding*) const;

        struct ImageLayoutTransition
        {
            Image* image;
            VkImageLayout layout;
            VkShaderStageFlags stageFlags;
        };
        DKArray<ImageLayoutTransition> imageLayoutTransitions;

        // pending updates (vkUpdateDescriptorSets)
        DKArray<VkWriteDescriptorSet> descriptorWrites;
        DKArray<VkCopyDescriptorSet> descriptorCopies;

        // descriptor infos (for storage of VkWriteDescriptorSets)
        DKArray<VkDescriptorImageInfo> imageInfos;
        DKArray<VkDescriptorBufferInfo> bufferInfos;
        DKArray<VkBufferView> texelBufferViews;

        using BufferViewObject = DKObject<BufferView>;
        using ImageViewObject = DKObject<ImageView>;
        using SamplerObject = DKObject<Sampler>;

        // take ownership of bound resources.
        DKMap<uint32_t, DKArray<BufferViewObject>> bufferViews;
        DKMap<uint32_t, DKArray<ImageViewObject>> imageViews;
        DKMap<uint32_t, DKArray<SamplerObject>> samplers;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
