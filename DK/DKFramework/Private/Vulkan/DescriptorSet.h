//
//  File: DescriptorSet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
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

        DKArray<BufferViewObject> bufferViews;
        DKArray<ImageViewObject> imageViews;
        DKArray<SamplerObject> samplers;

        VkDescriptorSet descriptorSet;
        DKObject<DescriptorPool> descriptorPool;
        DKObject<DKGraphicsDevice> device;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
