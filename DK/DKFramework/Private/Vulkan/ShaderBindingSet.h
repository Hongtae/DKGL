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
#include "Buffer.h"
#include "Texture.h"
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

        // pending updates (vkUpdateDescriptorSets)
        DKArray<VkWriteDescriptorSet> descriptorWrites;
        DKArray<VkCopyDescriptorSet> descriptorCopies;

        // descriptor infos (for storage of VkWriteDescriptorSets)
        DKArray<VkDescriptorImageInfo> imageInfos;
        DKArray<VkDescriptorBufferInfo> bufferInfos;
        DKArray<VkBufferView> texelBufferViews;

        using BufferObject = DKObject<Buffer>;
        using TextureObject = DKObject<Texture>;
        using SamplerObject = DKObject<Sampler>;
        // take ownership of bound resources.
        DKMap<uint32_t, DKArray<BufferObject>> buffers;
        DKMap<uint32_t, DKArray<TextureObject>> textures;
        DKMap<uint32_t, DKArray<SamplerObject>> samplers;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
