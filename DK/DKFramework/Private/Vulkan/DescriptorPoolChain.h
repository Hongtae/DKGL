//
//  File: DescriptorPoolChain.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "DescriptorPool.h"

namespace DKFramework::Private::Vulkan
{
    class GraphicsDevice;
    class DescriptorPoolChain
    {
    public:
        struct AllocationInfo
        {
            VkDescriptorSet descriptorSet;
            DKObject<DescriptorPool> descriptorPool;
        };

        bool AllocateDescriptorSet(VkDescriptorSetLayout, AllocationInfo&);

        size_t Cleanup();

        DescriptorPoolChain(GraphicsDevice*, const DescriptorPoolId&);
        ~DescriptorPoolChain();

        GraphicsDevice* device;
        const DescriptorPoolId poolId;

    private:
        DescriptorPool* AddNewPool(VkDescriptorPoolCreateFlags flags = 0);
        
        DKArray<DKObject<DescriptorPool>> descriptorPoolArray;
        uint32_t maxSets;
    };
}
#endif //#if DKGL_ENABLE_VULKAN
