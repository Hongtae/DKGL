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

        DKObject<DescriptorSet> descriptorSet;
        DKObject<DKGraphicsDevice> device;

        using BufferViewObject = DKObject<BufferView>;
        using ImageViewObject = DKObject<ImageView>;
        using SamplerObject = DKObject<Sampler>;

        struct DescriptorBinding
        {
            const VkDescriptorSetLayoutBinding layoutBinding;

            // hold resource object ownership
            DKArray<BufferViewObject> bufferViews;
            DKArray<ImageViewObject> imageViews;
            DKArray<SamplerObject> samplers;

            // descriptor infos (for storage of VkWriteDescriptorSets)
            DKArray<VkDescriptorImageInfo> imageInfos;
            DKArray<VkDescriptorBufferInfo> bufferInfos;
            DKArray<VkBufferView> texelBufferViews;

            // pending updates (vkUpdateDescriptorSets)
            DKArray<VkWriteDescriptorSet> descriptorWrites;

            bool dirty;
        };
        DKArray<DescriptorBinding> bindings;

        void SetBuffer(uint32_t binding, DKGpuBuffer*, uint64_t, uint64_t) override;
        void SetBufferArray(uint32_t binding, uint32_t numBuffers, BufferInfo*) override;
        void SetTexture(uint32_t binding, DKTexture*) override;
        void SetTextureArray(uint32_t binding, uint32_t numTextures, DKTexture**) override;
        void SetSamplerState(uint32_t binding, DKSamplerState*) override;
        void SetSamplerStateArray(uint32_t binding, uint32_t numSamplers, DKSamplerState**) override;

        using ImageLayoutMap = DKMap<Image*, VkImageLayout>;
        using ImageViewLayoutMap = DKMap<VkImageView, VkImageLayout>;

        void CollectImageViewLayouts(ImageLayoutMap&, ImageViewLayoutMap&);

        DKObject<DescriptorSet> CreateDescriptorSet(const ImageViewLayoutMap& imageLayouts);
        bool FindDescriptorBinding(uint32_t binding, DescriptorBinding**);
    };
}
#endif //#if DKGL_ENABLE_VULKAN
