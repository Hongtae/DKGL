//
//  File: ShaderBindingSet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../DKGraphicsDevice.h"
#include "../../DKShaderBindingSet.h"

namespace DKFramework::Private::Vulkan
{
    class DescriptorPool;
    class ShaderBindingSet : public DKShaderBindingSet
    {
    public:
        ShaderBindingSet(DKGraphicsDevice*, VkDescriptorSet, DescriptorPool*);
        ~ShaderBindingSet();

        VkDescriptorSet descriptorSet;
        DKObject<DescriptorPool> descriptorPool;
        DKObject<DKGraphicsDevice> device;

        void SetBuffer(uint32_t binding, DKGpuBuffer*) override;
        void SetTexture(uint32_t binding, DKTexture*) override;
        void SetSamplerState(uint32_t binding, DKSamplerState*) override;
        void SetTextureSampler(uint32_t binding, DKTexture*, DKSamplerState*) override;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
