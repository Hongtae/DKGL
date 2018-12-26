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
        DKArray<BufferObject>& bufferObjectArray = buffers.Value(binding);
        bufferObjectArray.Clear();
        bufferObjectArray.Add(buffer);

        descriptorWrites.Add(write);
    }
}

void ShaderBindingSet::SetBufferArray(uint32_t binding, uint32_t numBuffers, BufferInfo* bufferArray)
{
    VkDescriptorSetLayoutBinding descriptor = {};
    if (FindDescriptorBinding(binding, &descriptor))
    {
        for (uint32_t i = 0; i < descriptorWrites.Count(); ++i)
        {
            if (descriptorWrites.Value(i).dstBinding == binding)
            {
                descriptorWrites.Remove(i);
                break;
            }
        }

        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numBuffers, descriptor.descriptorCount - startingIndex);
        DKASSERT_DEBUG(availableItems <= numBuffers);

        VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet = descriptorSet;
        write.dstBinding = descriptor.binding;
        write.dstArrayElement = startingIndex;
        write.descriptorCount = availableItems;
        write.descriptorType = descriptor.descriptorType;


        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            // bufferView (pTexelBufferView)
            if (availableItems)
            {
                auto first = texelBufferViews.Count();
                texelBufferViews.Reserve(first + availableItems);
                for (uint32_t i = 0; i < availableItems; ++i)
                {
                    DKASSERT_DEBUG(dynamic_cast<Buffer*>(bufferArray[i].buffer));
                    Buffer* buffer = static_cast<Buffer*>(bufferArray[i].buffer);
                    DKASSERT_DEBUG(buffer->bufferView);
                    texelBufferViews.Add(buffer->bufferView);
                }
                write.pTexelBufferView = &texelBufferViews.Value(first);
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            // buffer (pBufferInfo)
            if (availableItems)
            {
                auto first = bufferInfos.Count();
                bufferInfos.Reserve(first + availableItems);
                for (uint32_t i = 0; i < availableItems; ++i)
                {
                    DKASSERT_DEBUG(dynamic_cast<Buffer*>(bufferArray[i].buffer));
                    Buffer* buffer = static_cast<Buffer*>(bufferArray[i].buffer);
                    DKASSERT_DEBUG(buffer->buffer);

                    VkDescriptorBufferInfo bufferInfo = {};
                    bufferInfo.buffer = buffer->buffer;
                    bufferInfo.offset = bufferArray[i].offset;
                    bufferInfo.range = bufferArray[i].length;

                    bufferInfos.Add(bufferInfo);
                }
                write.pBufferInfo = &bufferInfos.Value(first);
            }
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        DKArray<BufferObject>& bufferObjectArray = buffers.Value(binding);
        bufferObjectArray.Clear();
        bufferObjectArray.Reserve(availableItems);

        for (uint32_t i = 0; i < availableItems; ++i)
        {
            Buffer* buffer = static_cast<Buffer*>(bufferArray[i].buffer);
            bufferObjectArray.Add(buffer);
        }

        descriptorWrites.Add(write);
    }
}

void ShaderBindingSet::SetTexture(uint32_t binding, DKTexture* textureObject)
{
    VkDescriptorSetLayoutBinding descriptor = {};
    if (FindDescriptorBinding(binding, &descriptor))
    {
        DKASSERT_DEBUG(dynamic_cast<Texture*>(textureObject) != nullptr);
        Texture* texture = static_cast<Texture*>(textureObject);

        VkWriteDescriptorSet* prevWrite = nullptr;
        for (uint32_t i = 0; i < descriptorWrites.Count(); ++i)
        {
            if (descriptorWrites.Value(i).dstBinding == binding)
            {
                if (descriptorWrites.Value(i).descriptorCount > 0)
                {
                    prevWrite = &descriptorWrites.Value(i);
                    DKASSERT_DEBUG(prevWrite->dstSet == descriptorSet);
                    DKASSERT_DEBUG(prevWrite->descriptorType == descriptor.descriptorType);
                }
                else
                {
                    descriptorWrites.Remove(i);
                }
                break;
            }
        }

        switch (descriptor.descriptorType)
        {
            // pImageInfo
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            DKASSERT_DEBUG(texture->imageView);
            if (prevWrite)
            {
                auto first = imageInfos.Count();
                imageInfos.Reserve(first + prevWrite->descriptorCount);
                for (uint32_t i = 0; i < prevWrite->descriptorCount; ++i)
                {
                    VkDescriptorImageInfo imageInfo = prevWrite->pImageInfo[i];
                    imageInfo.imageView = texture->imageView;
                    auto index = imageInfos.Add(imageInfo);
                }
                prevWrite->pImageInfo = &imageInfos.Value(first);
            }
            else
            {
                VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                write.dstSet = descriptorSet;
                write.dstBinding = descriptor.binding;
                write.dstArrayElement = 0;
                write.descriptorCount = 1;  // number of descriptors to update.
                write.descriptorType = descriptor.descriptorType;

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageView = texture->imageView;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                auto index = imageInfos.Add(imageInfo);
                write.pImageInfo = &imageInfos.Value(index);

                descriptorWrites.Add(write);
            }
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        DKArray<TextureObject>& textureObjectArray = textures.Value(binding);
        textureObjectArray.Clear();
        textureObjectArray.Add(texture);
    }
}

void ShaderBindingSet::SetTextureArray(uint32_t binding, uint32_t numTextures, DKTexture** textureArray)
{
    VkDescriptorSetLayoutBinding descriptor = {};
    if (FindDescriptorBinding(binding, &descriptor))
    {
        VkWriteDescriptorSet* prevWrite = nullptr;
        for (uint32_t i = 0; i < descriptorWrites.Count(); ++i)
        {
            if (descriptorWrites.Value(i).dstBinding == binding)
            {
                if (descriptorWrites.Value(i).descriptorCount > 0)
                {
                    prevWrite = &descriptorWrites.Value(i);
                    DKASSERT_DEBUG(prevWrite->dstSet == descriptorSet);
                    DKASSERT_DEBUG(prevWrite->descriptorType == descriptor.descriptorType);
                }
                else
                {
                    descriptorWrites.Remove(i);
                }
                break;
            }
        }

        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numTextures, descriptor.descriptorCount - startingIndex);
        DKASSERT_DEBUG(availableItems <= numTextures);

        switch (descriptor.descriptorType)
        {
            // pImageInfo
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            if (prevWrite)
            {
                prevWrite->dstArrayElement = startingIndex;
                prevWrite->descriptorCount = availableItems;

                auto first = imageInfos.Count();
                imageInfos.Reserve(first + availableItems);
                for (uint32_t i = 0; i < availableItems; ++i)
                {
                    VkDescriptorImageInfo imageInfo = (prevWrite->descriptorCount < i) ?
                        prevWrite->pImageInfo[i] : prevWrite->pImageInfo[prevWrite->descriptorCount - 1];

                    DKASSERT_DEBUG(dynamic_cast<Texture*>(textureArray[i]) != nullptr);
                    Texture* texture = static_cast<Texture*>(textureArray[i]);
                    DKASSERT_DEBUG(texture->imageView);

                    imageInfo.imageView = texture->imageView;
                    auto index = imageInfos.Add(imageInfo);
                }
                prevWrite->pImageInfo = &imageInfos.Value(first);
            }
            else
            {
                VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                write.dstSet = descriptorSet;
                write.dstBinding = descriptor.binding;
                write.dstArrayElement = startingIndex;
                write.descriptorCount = availableItems;
                write.descriptorType = descriptor.descriptorType;

                auto first = imageInfos.Count();
                imageInfos.Reserve(first + availableItems);
                for (uint32_t i = 0; i < availableItems; ++i)
                {
                    VkDescriptorImageInfo imageInfo = {};

                    DKASSERT_DEBUG(dynamic_cast<Texture*>(textureArray[i]) != nullptr);
                    Texture* texture = static_cast<Texture*>(textureArray[i]);
                    DKASSERT_DEBUG(texture->imageView);

                    imageInfo.imageView = texture->imageView;
                    auto index = imageInfos.Add(imageInfo);
                }
                write.pImageInfo = &imageInfos.Value(first);

                descriptorWrites.Add(write);
            }
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        DKArray<TextureObject>& textureObjectArray = textures.Value(binding);
        textureObjectArray.Clear();
        textureObjectArray.Reserve(availableItems);

        for (uint32_t i = 0; i < availableItems; ++i)
        {
            Texture* texture = static_cast<Texture*>(textureArray[i]);
            textureObjectArray.Add(texture);
        }
    }
}

void ShaderBindingSet::SetSamplerState(uint32_t binding, DKSamplerState* samplerState)
{
    VkDescriptorSetLayoutBinding descriptor = {};
    if (FindDescriptorBinding(binding, &descriptor))
    {
        DKASSERT_DEBUG(dynamic_cast<Sampler*>(samplerState) != nullptr);
        Sampler* sampler = static_cast<Sampler*>(samplerState);

        VkWriteDescriptorSet* prevWrite = nullptr;
        for (uint32_t i = 0; i < descriptorWrites.Count(); ++i)
        {
            if (descriptorWrites.Value(i).dstBinding == binding)
            {
                if (descriptorWrites.Value(i).descriptorCount > 0)
                {
                    prevWrite = &descriptorWrites.Value(i);
                    DKASSERT_DEBUG(prevWrite->dstSet == descriptorSet);
                    DKASSERT_DEBUG(prevWrite->descriptorType == descriptor.descriptorType);
                }
                else
                {
                    descriptorWrites.Remove(i);
                }
                break;
            }
        }

        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            DKASSERT_DEBUG(sampler->sampler);
            if (prevWrite)
            {
                auto first = imageInfos.Count();
                imageInfos.Reserve(first + prevWrite->descriptorCount);
                for (uint32_t i = 0; i < prevWrite->descriptorCount; ++i)
                {
                    VkDescriptorImageInfo imageInfo = prevWrite->pImageInfo[i];
                    imageInfo.sampler = sampler->sampler;
                    auto index = imageInfos.Add(imageInfo);
                }
                prevWrite->pImageInfo = &imageInfos.Value(first);
            }
            else
            {
                VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                write.dstSet = descriptorSet;
                write.dstBinding = descriptor.binding;
                write.dstArrayElement = 0;
                write.descriptorCount = 1;  // number of descriptors to update.
                write.descriptorType = descriptor.descriptorType;

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.sampler = sampler->sampler;
                imageInfo.imageView = VK_NULL_HANDLE;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                auto index = imageInfos.Add(imageInfo);
                write.pImageInfo = &imageInfos.Value(index);

                descriptorWrites.Add(write);
            }
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        DKArray<SamplerObject>& samplerObjectArray = samplers.Value(binding);
        samplerObjectArray.Clear();
        samplerObjectArray.Add(sampler);
    }
}

void ShaderBindingSet::SetSamplerStateArray(uint32_t binding, uint32_t numSamplers, DKSamplerState** samplerArray) 
{
    VkDescriptorSetLayoutBinding descriptor = {};
    if (FindDescriptorBinding(binding, &descriptor))
    {
        VkWriteDescriptorSet* prevWrite = nullptr;
        for (uint32_t i = 0; i < descriptorWrites.Count(); ++i)
        {
            if (descriptorWrites.Value(i).dstBinding == binding)
            {
                if (descriptorWrites.Value(i).descriptorCount > 0)
                {
                    prevWrite = &descriptorWrites.Value(i);
                    DKASSERT_DEBUG(prevWrite->dstSet == descriptorSet);
                    DKASSERT_DEBUG(prevWrite->descriptorType == descriptor.descriptorType);
                }
                else
                {
                    descriptorWrites.Remove(i);
                }
                break;
            }
        }

        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numSamplers, descriptor.descriptorCount - startingIndex);
        DKASSERT_DEBUG(availableItems <= numSamplers);

        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            if (prevWrite)
            {
                prevWrite->dstArrayElement = startingIndex;
                prevWrite->descriptorCount = availableItems;

                auto first = imageInfos.Count();
                imageInfos.Reserve(first + availableItems);
                for (uint32_t i = 0; i < availableItems; ++i)
                {
                    VkDescriptorImageInfo imageInfo = (prevWrite->descriptorCount < i) ?
                        prevWrite->pImageInfo[i] : prevWrite->pImageInfo[prevWrite->descriptorCount - 1];

                    DKASSERT_DEBUG(dynamic_cast<Sampler*>(samplerArray[i]) != nullptr);
                    Sampler* sampler = static_cast<Sampler*>(samplerArray[i]);
                    DKASSERT_DEBUG(sampler->sampler);

                    imageInfo.sampler = sampler->sampler;
                    auto index = imageInfos.Add(imageInfo);
                }
                prevWrite->pImageInfo = &imageInfos.Value(first);
            }
            else
            {
                VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                write.dstSet = descriptorSet;
                write.dstBinding = descriptor.binding;
                write.dstArrayElement = startingIndex;
                write.descriptorCount = availableItems;
                write.descriptorType = descriptor.descriptorType;

                auto first = imageInfos.Count();
                imageInfos.Reserve(first + availableItems);
                for (uint32_t i = 0; i < availableItems; ++i)
                {
                    VkDescriptorImageInfo imageInfo = {};

                    DKASSERT_DEBUG(dynamic_cast<Sampler*>(samplerArray[i]) != nullptr);
                    Sampler* sampler = static_cast<Sampler*>(samplerArray[i]);
                    DKASSERT_DEBUG(sampler->sampler);

                    imageInfo.sampler = sampler->sampler;
                    imageInfo.imageView = VK_NULL_HANDLE;
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    auto index = imageInfos.Add(imageInfo);
                }
                write.pImageInfo = &imageInfos.Value(first);

                descriptorWrites.Add(write);
            }
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        DKArray<SamplerObject>& samplerObjectArray = samplers.Value(binding);
        samplerObjectArray.Clear();
        samplerObjectArray.Reserve(availableItems);

        for (uint32_t i = 0; i < availableItems; ++i)
        {
            Sampler* sampler = static_cast<Sampler*>(samplerArray[i]);
            samplerObjectArray.Add(sampler);
        }
    }
}

#endif //#if DKGL_ENABLE_VULKAN
