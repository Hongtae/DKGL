//
//  File: ShaderBindingSet.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "GraphicsDevice.h"
#include "ShaderBindingSet.h"
#include "DescriptorPool.h"
#include "Buffer.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ShaderBindingSet::ShaderBindingSet(DKGraphicsDevice* dev, VkDescriptorSetLayout layout, VkDescriptorSet set, DescriptorPool* pool)
    : device(dev)
    , descriptorSet(set)
    , descriptorSetLayout(layout)
    , descriptorPool(pool)    
{
    DKASSERT_DEBUG(descriptorSet != VK_NULL_HANDLE);
    DKASSERT_DEBUG(descriptorSetLayout != VK_NULL_HANDLE);
    DKASSERT_DEBUG(descriptorPool);
}


ShaderBindingSet::~ShaderBindingSet()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    dev->DestroyDescriptorSet(descriptorSet, descriptorPool);
    vkDestroyDescriptorSetLayout(dev->device, descriptorSetLayout, dev->allocationCallbacks);
}

void ShaderBindingSet::UpdateDescriptorSet()
{
    if (descriptorWrites.Count() > 0 || descriptorCopies.Count() > 0)
    {
        GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
        vkUpdateDescriptorSets(dev->device,
                               descriptorWrites.Count(),
                               descriptorWrites,
                               descriptorCopies.Count(),
                               descriptorCopies);
        descriptorWrites.Clear();
        descriptorCopies.Clear();

        imageInfos.Clear();
        bufferInfos.Clear();
        texelBufferViews.Clear();
    }
    
    DKASSERT_DEBUG(descriptorWrites.IsEmpty());
    DKASSERT_DEBUG(descriptorCopies.IsEmpty());
    DKASSERT_DEBUG(imageInfos.IsEmpty());
    DKASSERT_DEBUG(bufferInfos.IsEmpty());
    DKASSERT_DEBUG(texelBufferViews.IsEmpty());
}

bool ShaderBindingSet::FindDescriptorBinding(uint32_t binding, VkDescriptorSetLayoutBinding* descriptor) const
{
    for (const VkDescriptorSetLayoutBinding& b : bindings)
    {
        if (b.binding == binding)
        {
            *descriptor = b;
            return true;
        }
    }
    return false;
}

void ShaderBindingSet::SetBuffer(uint32_t binding, DKGpuBuffer* bufferObject, uint64_t offset, uint64_t length)
{
    VkDescriptorSetLayoutBinding descriptor = {};
    if (FindDescriptorBinding(binding, &descriptor))
    {
        DKASSERT_DEBUG(dynamic_cast<Buffer*>(bufferObject) != nullptr);
        Buffer* buffer = static_cast<Buffer*>(bufferObject);

        for (uint32_t i = 0; i < descriptorWrites.Count(); ++i)
        {
            if (descriptorWrites.Value(i).dstBinding == binding)
            {
                descriptorWrites.Remove(i);
                break;
            }
        }

        VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet = descriptorSet;
        write.dstBinding = descriptor.binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;  // number of descriptors to update.
        write.descriptorType = descriptor.descriptorType;

        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            // bufferView (pTexelBufferView)
            if (1)
            {
                DKASSERT_DEBUG(buffer->bufferView);
                write.pTexelBufferView = &buffer->bufferView;
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            // buffer (pBufferInfo)
            if (1)
            {
                DKASSERT_DEBUG(buffer->buffer);
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer->buffer;
                bufferInfo.offset = offset;
                bufferInfo.range = length;

                // keep VkDescriptorBufferInfo instance until the descriptorSet is updated.
                auto index = bufferInfos.Add(bufferInfo);
                write.pBufferInfo = &bufferInfos.Value(index);
            }
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        buffers.Update(binding, buffer);

        descriptorWrites.Add(write);
    }
}

void ShaderBindingSet::SetTexture(uint32_t binding, DKTexture* textureObject)
{

}

void ShaderBindingSet::SetSamplerState(uint32_t binding, DKSamplerState*)
{
 
}

void ShaderBindingSet::SetTextureSampler(uint32_t binding, DKTexture*, DKSamplerState*)
{

}

#endif //#if DKGL_ENABLE_VULKAN
