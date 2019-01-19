//
//  File: DescriptorPool.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "GraphicsDevice.h"
#include "DescriptorPool.h"
#include "ShaderBindingSet.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

DescriptorPool::DescriptorPool(GraphicsDevice* dev, VkDescriptorPool p, const VkDescriptorPoolCreateInfo& ci, const DescriptorPoolId& pid)
    : device(dev)
    , pool(p)
    , maxSets(ci.maxSets)
    , createFlags(ci.flags)
    , numAllocatedSets(0)
    , poolId(pid)
{
    DKASSERT_DEBUG(pool);
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(device->device, pool, device->allocationCallbacks);
}

VkDescriptorSet DescriptorPool::AllocateDescriptorSet(VkDescriptorSetLayout layout)
{
    DKASSERT_DEBUG(layout != VK_NULL_HANDLE);

    VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocateInfo.descriptorPool = pool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &layout;

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    VkResult result = vkAllocateDescriptorSets(device->device, &allocateInfo, &descriptorSet);
    if (result == VK_SUCCESS)
    {
        DKASSERT_DEBUG(descriptorSet != VK_NULL_HANDLE);
        numAllocatedSets++;
    }
    else
    {
        DKLogE("ERROR: vkAllocateDescriptorSets failed: %s", VkResultCStr(result));
    }

    return descriptorSet;
}

void DescriptorPool::ReleaseDescriptorSet(VkDescriptorSet set)
{
    DKASSERT_DEBUG(set != VK_NULL_HANDLE);
        
    DKASSERT_DEBUG(numAllocatedSets > 0);
    numAllocatedSets -= 1;

    if (numAllocatedSets == 0)
    {
        VkResult result = vkResetDescriptorPool(device->device, pool, 0);
        if (result != VK_SUCCESS)
        {
            DKLogE("ERROR: vkResetDescriptorPool failed: %s", VkResultCStr(result));
        }
    }
    else if (createFlags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
    {
        VkResult result = vkFreeDescriptorSets(device->device, pool, 1, &set);
        DKASSERT_DEBUG(result == VK_SUCCESS);
        if (result != VK_SUCCESS)
        {
            DKLogE("ERROR: vkFreeDescriptorSets failed: %s", VkResultCStr(result));
        }
    }
}

#endif //#if DKGL_ENABLE_VULKAN
