//
//  File: DescriptorSet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2022 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "BufferView.h"
#include "ImageView.h"
#include "Sampler.h"

namespace DKFramework::Private::Vulkan
{
    class DescriptorPool;
    class DescriptorSet
    {
    public:
        DescriptorSet(DKGraphicsDevice*, DescriptorPool*, VkDescriptorSet);
        virtual ~DescriptorSet();

        using BufferViewObject = DKObject<BufferView>;
        using ImageViewObject = DKObject<ImageView>;
        using SamplerObject = DKObject<Sampler>;

        using ImageLayoutMap = DKMap<Image*, VkImageLayout>;
        using ImageViewLayoutMap = DKMap<VkImageView, VkImageLayout>;

        struct Binding
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
            VkWriteDescriptorSet write;
            bool valueSet;
        };
        DKArray<Binding> bindings;

        using ImageLayoutMap = DKMap<Image*, VkImageLayout>;
        using ImageViewLayoutMap = DKMap<VkImageView, VkImageLayout>;

        void CollectImageViewLayouts(ImageLayoutMap&, ImageViewLayoutMap&);
        void UpdateImageViewLayouts(const ImageViewLayoutMap&);

        VkDescriptorSet descriptorSet;
        DKObject<DescriptorPool> descriptorPool;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
