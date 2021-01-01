//
//  File: DescriptorPoolChain.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "GraphicsDevice.h"
#include "DescriptorPool.h"
#include "DescriptorPoolChain.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

DescriptorPoolChain::DescriptorPoolChain(GraphicsDevice* dev, const DescriptorPoolId& pid)
    : device(dev)
    , poolId(pid)
    , maxSets(0)
{
    DKASSERT_DEBUG(device);
    DKASSERT_DEBUG(poolId.mask);
}

DescriptorPoolChain::~DescriptorPoolChain()
{

}

bool DescriptorPoolChain::AllocateDescriptorSet(VkDescriptorSetLayout layout, AllocationInfo& info)
{
    DKASSERT_DEBUG(device);

    for (size_t i = 0; i < descriptorPoolArray.Count(); ++i)
    {
        DescriptorPool* pool = descriptorPoolArray.Value(i);
        VkDescriptorSet ds = pool->AllocateDescriptorSet(layout);
        if (ds != VK_NULL_HANDLE)
        {
            info.descriptorSet = ds;
            info.descriptorPool = pool;

            if (i)
            {
                descriptorPoolArray.Remove(i);
                descriptorPoolArray.Insert(pool, 0);
            }
            return true;
        }
    }
    DescriptorPool* pool = AddNewPool(0);
    if (pool)
    {
        VkDescriptorSet ds = pool->AllocateDescriptorSet(layout);
        if (ds != VK_NULL_HANDLE)
        {
            info.descriptorSet = ds;
            info.descriptorPool = pool;
            return true;
        }
    }
    return false;
}

DescriptorPool* DescriptorPoolChain::AddNewPool(VkDescriptorPoolCreateFlags flags)
{
    DKASSERT_DEBUG(device);

    maxSets = maxSets * 2 + 1;

    DKArray<VkDescriptorPoolSize> poolSizes;
    poolSizes.Reserve(numDescriptorTypes);
    for (uint32_t i = 0; i < numDescriptorTypes; ++i)
    {
        if (poolId.typeSize[i] > 0)
        {
            VkDescriptorType type = DescriptorTypeAtIndex(i);
            VkDescriptorPoolSize poolSize = {
                type,
                poolId.typeSize[i] * maxSets 
            };
            poolSizes.Add(poolSize);
        }
    }

    VkDescriptorPoolCreateInfo ci = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    ci.flags = flags;
    ci.poolSizeCount = poolSizes.Count();
    ci.pPoolSizes = poolSizes;
    ci.maxSets = maxSets;

    // setup.
    DKASSERT_DEBUG(ci.maxSets);
    DKASSERT_DEBUG(ci.poolSizeCount);
    DKASSERT_DEBUG(ci.pPoolSizes);

    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkResult err = vkCreateDescriptorPool(device->device,
                                          &ci,
                                          device->allocationCallbacks,
                                          &pool);

    if (err == VK_SUCCESS)
    {
        DKASSERT_DEBUG(pool);

        DKObject<DescriptorPool> dp = DKOBJECT_NEW DescriptorPool(device, pool, ci, poolId);
        descriptorPoolArray.Insert(dp, 0);
        return dp;
    }
    DKLogE("ERROR: vkCreateDescriptorPool failed: %s", VkResultCStr(err));
    return nullptr;
}

size_t DescriptorPoolChain::Cleanup()
{
    DKArray<DKObject<DescriptorPool>> poolCopy = std::move(descriptorPoolArray);

    DKArray<DescriptorPool*> emptyPools;
    emptyPools.Reserve(poolCopy.Count());
    for (DescriptorPool* pool : poolCopy)
    {
        if (pool->numAllocatedSets)
            descriptorPoolArray.Add(pool);
        else
            emptyPools.Add(pool);
    }
    if (emptyPools.Count() > 1)
    {
        emptyPools.Sort([](const DescriptorPool* lhs, const DescriptorPool* rhs)->bool
        {
            return lhs->maxSets > rhs->maxSets;
        });
    }
    if (emptyPools.Count() > 0 && descriptorPoolArray.Count())
        descriptorPoolArray.Add(emptyPools.Value(0)); // add first (biggest) pool for reuse.

    return descriptorPoolArray.Count();
}

#endif //#if DKGL_ENABLE_VULKAN
