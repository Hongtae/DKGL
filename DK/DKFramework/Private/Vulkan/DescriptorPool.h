//
//  File: DescriptorPool.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "Types.h"

namespace DKFramework::Private::Vulkan
{
    constexpr auto descriptorTypes = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT
    };
    static_assert(std::is_sorted(std::cbegin(descriptorTypes), std::cend(descriptorTypes)));
    constexpr size_t numDescriptorTypes = std::size(descriptorTypes);

    inline auto IndexOfDescriptorType(VkDescriptorType t)
    {
        auto it = std::lower_bound(std::begin(descriptorTypes),
                                   std::end(descriptorTypes),
                                   t);
        DKASSERT_DEBUG(it != std::end(descriptorTypes));
        return std::distance(std::begin(descriptorTypes), it);
    };
    template <typename Index> inline
    VkDescriptorType DescriptorTypeAtIndex(Index index)
    {
        DKASSERT_DEBUG(index >= 0 && index < numDescriptorTypes);
        auto it = std::begin(descriptorTypes);
        std::advance(it, index);
        return *it;
    }

    struct DescriptorPoolId
    {
        uint32_t mask = 0;
        uint32_t typeSize[numDescriptorTypes] = { 0 };

        DescriptorPoolId() = default;
        DescriptorPoolId(uint32_t poolSizeCount, const VkDescriptorPoolSize* poolSizes)
            : mask(0)
            , typeSize{ 0 }
        {
            for (uint32_t i = 0; i < poolSizeCount; ++i)
            {
                const VkDescriptorPoolSize& poolSize = poolSizes[i];
                uint32_t typeIndex = IndexOfDescriptorType(poolSize.type);
                typeSize[typeIndex] += poolSize.descriptorCount;
            }
            uint32_t dpTypeKey = 0;
            for (uint32_t i = 0; i < numDescriptorTypes; ++i)
            {
                if (typeSize[i])
                    mask = mask | (1 << i);
            }
        }
        DescriptorPoolId(const DKShaderBindingSetLayout& layout)
            : mask(0)
            , typeSize{ 0 }
        {
            for (const DKShaderBinding& binding : layout.bindings)
            {
                VkDescriptorType type = DescriptorType(binding.type);
                uint32_t typeIndex = IndexOfDescriptorType(type);
                typeSize[typeIndex] += binding.arrayLength;
            }
            uint32_t dpTypeKey = 0;
            for (uint32_t i = 0; i < numDescriptorTypes; ++i)
            {
                if (typeSize[i])
                    mask = mask | (1 << i);
            }
        }
        bool operator < (const DescriptorPoolId& rhs) const
        {
            if (this->mask == rhs.mask)
            {
                for (uint32_t i = 0; i < numDescriptorTypes; ++i)
                {
                    if (this->typeSize[i] != rhs.typeSize[i])
                        return this->typeSize[i] < rhs.typeSize[i];
                }
            }
            return this->mask < rhs.mask;
        }
        bool operator > (const DescriptorPoolId& rhs) const
        {
            if (this->mask == rhs.mask)
            {
                for (uint32_t i = 0; i < numDescriptorTypes; ++i)
                {
                    if (this->typeSize[i] != rhs.typeSize[i])
                        return this->typeSize[i] > rhs.typeSize[i];
                }
            }
            return this->mask > rhs.mask;
        }
        bool operator == (const DescriptorPoolId& rhs) const
        {
            if (this->mask == rhs.mask)
            {
                for (uint32_t i = 0; i < numDescriptorTypes; ++i)
                {
                    if (this->typeSize[i] != rhs.typeSize[i])
                        return false;
                }
                return true;
            }
            return false;
        }
    };

    class GraphicsDevice;
    class DescriptorPool final
    {
    public:
        const DescriptorPoolId& poolId; // key for container(DescriptorPoolChain)
        const uint32_t maxSets;
        const VkDescriptorPoolCreateFlags createFlags;

        VkDescriptorPool pool;
        GraphicsDevice* device;
        uint32_t numAllocatedSets;


        DescriptorPool(GraphicsDevice*, VkDescriptorPool, const VkDescriptorPoolCreateInfo& ci, const DescriptorPoolId&);
        ~DescriptorPool();

        VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout);

        void ReleaseDescriptorSets(VkDescriptorSet*, size_t);

    };
}
#endif //#if DKGL_ENABLE_VULKAN
