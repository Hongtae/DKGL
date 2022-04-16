//
//  File: ShaderBindingSet.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "GraphicsDevice.h"
#include "ShaderBindingSet.h"
#include "DescriptorPool.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ShaderBindingSet::ShaderBindingSet(DKGraphicsDevice* dev,
                                   VkDescriptorSetLayout layout,
                                   const DescriptorPoolId& poolId,
                                   const VkDescriptorSetLayoutCreateInfo& createInfo)
    : device(dev)
    , descriptorSetLayout(layout)
    , poolId(poolId)
    , layoutFlags(createInfo.flags)
{
    DKASSERT_DEBUG(descriptorSetLayout != VK_NULL_HANDLE);

    bindings.Reserve(createInfo.bindingCount);
    for (uint32_t i = 0; i < createInfo.bindingCount; ++i)
    {
        const VkDescriptorSetLayoutBinding& binding = createInfo.pBindings[i];

        DescriptorBinding ds = { binding };
        ds.valueSet = false;
        bindings.Add(ds);
    }
}

ShaderBindingSet::~ShaderBindingSet()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    vkDestroyDescriptorSetLayout(dev->device, descriptorSetLayout, dev->allocationCallbacks);
}

DKObject<DescriptorSet> ShaderBindingSet::CreateDescriptorSet() const
{
    DKGraphicsDevice* device = const_cast<DKGraphicsDevice*>(this->device.Ptr());
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance((DKGraphicsDevice*)device);
    DKObject<DescriptorSet> descriptorSet = dev->CreateDescriptorSet(device, descriptorSetLayout, poolId);
    DKASSERT_DEBUG(descriptorSet);

    descriptorSet->bindings = this->bindings;

    DKArray<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.Reserve(descriptorSet->bindings.Count());

    for (DescriptorBinding& binding : descriptorSet->bindings)
    {
        if (!binding.valueSet)
            continue;

        VkWriteDescriptorSet& write = binding.write;
        write.dstSet = descriptorSet->descriptorSet;
        if (write.pImageInfo)
            write.pImageInfo = binding.imageInfos;
        if (write.pBufferInfo)
            write.pBufferInfo = binding.bufferInfos;
        if (write.pTexelBufferView)
            write.pTexelBufferView = binding.texelBufferViews;

        descriptorWrites.Add(write);
    }

    DKASSERT_DEBUG(descriptorWrites.Count() > 0);

    vkUpdateDescriptorSets(dev->device,
                           descriptorWrites.Count(),
                           descriptorWrites,
                           0,
                           nullptr);

    return descriptorSet;
}

ShaderBindingSet::DescriptorBinding* ShaderBindingSet::FindDescriptorBinding(uint32_t binding)
{
    for (DescriptorBinding& b : bindings)
    {
        if (b.layoutBinding.binding == binding)
        {
            return &b;
        }
    }
    return nullptr;
}

void ShaderBindingSet::SetBuffer(uint32_t binding, DKGpuBuffer* bufferObject, uint64_t offset, uint64_t length)
{
    BufferInfo bufferInfo = { bufferObject, offset, length };
    return SetBufferArray(binding, 1, &bufferInfo);
#if 0
    DescriptorBinding* descriptorBinding = FindDescriptorBinding(binding);
    if (descriptorBinding)
    {
        descriptorBinding->valueSet = false;
        descriptorBinding->bufferInfos.Clear();
        descriptorBinding->imageInfos.Clear();
        descriptorBinding->texelBufferViews.Clear();
        descriptorBinding->bufferViews.Clear();
        descriptorBinding->imageViews.Clear();
        descriptorBinding->samplers.Clear();

        const VkDescriptorSetLayoutBinding& descriptor = descriptorBinding->layoutBinding;

        DKASSERT_DEBUG(dynamic_cast<BufferView*>(bufferObject) != nullptr);
        BufferView* bufferView = static_cast<BufferView*>(bufferObject);

        VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet = VK_NULL_HANDLE;
        write.dstBinding = descriptor.binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;  // number of descriptors to update.
        write.descriptorType = descriptor.descriptorType;

        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            // bufferView (pTexelBufferView)
            DKASSERT_DEBUG(bufferView->bufferView);
            if (bufferView->bufferView)
            {
                write.pTexelBufferView = &bufferView->bufferView;
            }
            else
            {
                DKLogE("DKShaderBindingSet::SetBuffer failed: Invalid buffer view");
                return;
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            // buffer (pBufferInfo)
            DKASSERT_DEBUG(bufferView->buffer);
            if (Buffer* buffer = bufferView->buffer; buffer)
            {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer->buffer;
                bufferInfo.offset = offset;
                bufferInfo.range = length;

                // keep VkDescriptorBufferInfo instance until the descriptorSet is updated.
                auto index = descriptorBinding->bufferInfos.Add(bufferInfo);
                write.pBufferInfo = &descriptorBinding->bufferInfos.Value(index);
            }
            else
            {
                DKLogE("DKShaderBindingSet::SetBuffer failed: Invalid buffer");
                return;
            }
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        descriptorBinding->bufferViews.Add(bufferView);
        descriptorBinding->write = write;
        descriptorBinding->valueSet = true;
    }
#endif
}

void ShaderBindingSet::SetBufferArray(uint32_t binding, uint32_t numBuffers, BufferInfo* bufferArray)
{
    DescriptorBinding* descriptorBinding = FindDescriptorBinding(binding);
    if (descriptorBinding)
    {
        descriptorBinding->valueSet = false;
        descriptorBinding->bufferInfos.Clear();
        descriptorBinding->imageInfos.Clear();
        descriptorBinding->texelBufferViews.Clear();
        descriptorBinding->bufferViews.Clear();
        descriptorBinding->imageViews.Clear();
        descriptorBinding->samplers.Clear();

        const VkDescriptorSetLayoutBinding& descriptor = descriptorBinding->layoutBinding;

        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numBuffers, descriptor.descriptorCount - startingIndex);
        DKASSERT_DEBUG(availableItems <= numBuffers);

        VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet = VK_NULL_HANDLE;
        write.dstBinding = descriptor.binding;
        write.dstArrayElement = startingIndex;
        write.descriptorCount = availableItems;
        write.descriptorType = descriptor.descriptorType;

        DKASSERT_DEBUG(descriptorBinding->bufferInfos.IsEmpty());
        DKASSERT_DEBUG(descriptorBinding->texelBufferViews.IsEmpty());

        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            // bufferView (pTexelBufferView)
            descriptorBinding->texelBufferViews.Reserve(availableItems);
            for (uint32_t i = 0; i < availableItems; ++i)
            {
                DKASSERT_DEBUG(dynamic_cast<BufferView*>(bufferArray[i].buffer));
                BufferView* bufferView = static_cast<BufferView*>(bufferArray[i].buffer);
                DKASSERT_DEBUG(bufferView->bufferView);
                descriptorBinding->texelBufferViews.Add(bufferView->bufferView);
            }
            write.pTexelBufferView = descriptorBinding->texelBufferViews;
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            // buffer (pBufferInfo)
            descriptorBinding->bufferInfos.Reserve(availableItems);
            for (uint32_t i = 0; i < availableItems; ++i)
            {
                DKASSERT_DEBUG(dynamic_cast<BufferView*>(bufferArray[i].buffer));
                Buffer* buffer = static_cast<BufferView*>(bufferArray[i].buffer)->buffer;
                DKASSERT_DEBUG(buffer->buffer);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer->buffer;
                bufferInfo.offset = bufferArray[i].offset;
                bufferInfo.range = bufferArray[i].length;

                descriptorBinding->bufferInfos.Add(bufferInfo);
            }
            write.pBufferInfo = descriptorBinding->bufferInfos;
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        descriptorBinding->bufferViews.Reserve(availableItems);

        for (uint32_t i = 0; i < availableItems; ++i)
        {
            BufferView* bufferView = static_cast<BufferView*>(bufferArray[i].buffer);
            descriptorBinding->bufferViews.Add(bufferView);
        }
        descriptorBinding->write = write;
        descriptorBinding->valueSet = true;
    }
}

void ShaderBindingSet::SetTexture(uint32_t binding, DKTexture* textureObject)
{
    return SetTextureArray(binding, 1, &textureObject);
#if 0
    DescriptorBinding* descriptorBinding = FindDescriptorBinding(binding);
    if (descriptorBinding)
    {
        //descriptorBinding->descriptorWrites.Clear();
        descriptorBinding->bufferInfos.Clear();
        //descriptorBinding->imageInfos.Clear();
        descriptorBinding->texelBufferViews.Clear();
        descriptorBinding->bufferViews.Clear();
        descriptorBinding->imageViews.Clear();
        //descriptorBinding->samplers.Clear();

        const VkDescriptorSetLayoutBinding& descriptor = descriptorBinding->layoutBinding;

        DKASSERT_DEBUG(dynamic_cast<ImageView*>(textureObject) != nullptr);
        ImageView* imageView = static_cast<ImageView*>(textureObject);

        if (!descriptorBinding->valueSet)
        {
            VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = VK_NULL_HANDLE;
            write.dstBinding = descriptor.binding;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;  // number of descriptors to update.
            write.descriptorType = descriptor.descriptorType;
            descriptorBinding->write = write;
            descriptorBinding->valueSet = true;
        }
        VkWriteDescriptorSet& write = descriptorBinding->write;
        if (write.pImageInfo == nullptr)
        {
            descriptorBinding->samplers.Clear();
            descriptorBinding->imageInfos.Clear();
            write.descriptorCount = 1;
        }

        DKPixelFormat pixelFormat = imageView->PixelFormat();
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED; /* imageView->LayerLayout(0);*/
        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            break;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            if (DKPixelFormatIsColorFormat(pixelFormat))
                imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            else if (DKPixelFormatIsDepthFormat(pixelFormat))
                imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            else if (DKPixelFormatIsStencilFormat(pixelFormat))
                imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        switch (descriptor.descriptorType)
        {
            // pImageInfo
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            DKASSERT_DEBUG(imageView->imageView);
            for (uint32_t i = 0; i < write.descriptorCount; ++i)
            {
                if (i >= descriptorBinding->imageInfos.Count())
                {
                    VkDescriptorImageInfo imageInfo = {};
                    auto index = descriptorBinding->imageInfos.Add(imageInfo);
                    DKASSERT_DEBUG(index == i);
                }

                VkDescriptorImageInfo& imageInfo = descriptorBinding->imageInfos.Value(i);
                imageInfo.imageView = imageView->imageView;
                imageInfo.imageLayout = imageLayout;
            }
            write.pImageInfo = descriptorBinding->imageInfos;
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        descriptorBinding->imageViews.Add(imageView);
    }
#endif
}

void ShaderBindingSet::SetTextureArray(uint32_t binding, uint32_t numTextures, DKTexture** textureArray)
{
    DescriptorBinding* descriptorBinding = FindDescriptorBinding(binding);
    if (descriptorBinding)
    {
        //descriptorBinding->descriptorWrites.Clear();
        descriptorBinding->bufferInfos.Clear();
        //descriptorBinding->imageInfos.Clear();
        descriptorBinding->texelBufferViews.Clear();
        descriptorBinding->bufferViews.Clear();
        descriptorBinding->imageViews.Clear();
        //descriptorBinding->samplers.Clear();

        const VkDescriptorSetLayoutBinding& descriptor = descriptorBinding->layoutBinding;

        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numTextures, descriptor.descriptorCount - startingIndex);
        DKASSERT_DEBUG(availableItems <= numTextures);

        if (!descriptorBinding->valueSet)
        {
            VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = VK_NULL_HANDLE;
            write.dstBinding = descriptor.binding;
            write.dstArrayElement = startingIndex;
            write.descriptorCount = availableItems;  // number of descriptors to update.
            write.descriptorType = descriptor.descriptorType;
            descriptorBinding->write = write;
            descriptorBinding->valueSet = true;
        }
        VkWriteDescriptorSet& write = descriptorBinding->write;
        if (write.pImageInfo == nullptr)
        {
            descriptorBinding->samplers.Clear();
            descriptorBinding->imageInfos.Clear();
        }
        write.dstArrayElement = startingIndex;
        write.descriptorCount = availableItems;

        auto getImageLayout = [](VkDescriptorType type, DKPixelFormat pixelFormat)
        {
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED; /* imageView->LayerLayout(0);*/
            switch (type)
            {
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                break;
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                break;
            default:
                imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            }
            return imageLayout;
        };

        switch (descriptor.descriptorType)
        {
            // pImageInfo
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            descriptorBinding->imageViews.Reserve(availableItems);
            for (uint32_t i = 0; i < availableItems; ++i)
            {
                if (i >= descriptorBinding->imageInfos.Count())
                {
                    VkDescriptorImageInfo info = {};
                    auto index = descriptorBinding->imageInfos.Add(info);
                    DKASSERT_DEBUG(index == i);
                }

                DKASSERT_DEBUG(dynamic_cast<ImageView*>(textureArray[i]) != nullptr);
                ImageView* imageView = static_cast<ImageView*>(textureArray[i]);
                DKASSERT_DEBUG(imageView->imageView);

                if (descriptor.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
                {
                    if (!(imageView->image->usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
                    {
                        DKLogE("ImageView image does not have usage flag:VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT");
                    }
                }

                VkDescriptorImageInfo& imageInfo = descriptorBinding->imageInfos.Value(i);
                imageInfo.imageView = imageView->imageView;
                imageInfo.imageLayout = getImageLayout(descriptor.descriptorType, imageView->image->PixelFormat());

                descriptorBinding->imageViews.Add(imageView);
            }
            write.pImageInfo = descriptorBinding->imageInfos;
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }
    }
}

void ShaderBindingSet::SetSamplerState(uint32_t binding, DKSamplerState* samplerState)
{
    return SetSamplerStateArray(binding, 1, &samplerState);
#if 0
    DescriptorBinding* descriptorBinding = FindDescriptorBinding(binding);
    if (descriptorBinding)
    {
        //descriptorBinding->descriptorWrites.Clear();
        descriptorBinding->bufferInfos.Clear();
        //descriptorBinding->imageInfos.Clear();
        descriptorBinding->texelBufferViews.Clear();
        descriptorBinding->bufferViews.Clear();
        //descriptorBinding->imageViews.Clear();
        descriptorBinding->samplers.Clear();

        const VkDescriptorSetLayoutBinding& descriptor = descriptorBinding->layoutBinding;

        DKASSERT_DEBUG(dynamic_cast<Sampler*>(samplerState) != nullptr);
        Sampler* sampler = static_cast<Sampler*>(samplerState);

        if (!descriptorBinding->valueSet)
        {
            VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = VK_NULL_HANDLE;
            write.dstBinding = descriptor.binding;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;  // number of descriptors to update.
            write.descriptorType = descriptor.descriptorType;
            descriptorBinding->write = write;
            descriptorBinding->valueSet = true;
        }
        VkWriteDescriptorSet& write = descriptorBinding->write;
        if (write.pImageInfo == nullptr)
        {
            descriptorBinding->imageViews.Clear();
            descriptorBinding->imageInfos.Clear();
            write.descriptorCount = 1;
        }

        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            DKASSERT_DEBUG(sampler->sampler);
            for (uint32_t i = 0; i < write.descriptorCount; ++i)
            {
                if (i >= descriptorBinding->imageInfos.Count())
                {
                    VkDescriptorImageInfo imageInfo = {};
                    auto index = descriptorBinding->imageInfos.Add(imageInfo);
                    DKASSERT_DEBUG(index == i);
                }
                VkDescriptorImageInfo& imageInfo = descriptorBinding->imageInfos.Value(i);
                imageInfo.sampler = sampler->sampler;
            }
            write.pImageInfo = descriptorBinding->imageInfos;
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }

        // take ownership of resource.
        descriptorBinding->samplers.Add(sampler);
    }
#endif
}

void ShaderBindingSet::SetSamplerStateArray(uint32_t binding, uint32_t numSamplers, DKSamplerState** samplerArray) 
{
    DescriptorBinding* descriptorBinding = FindDescriptorBinding(binding);
    if (descriptorBinding)
    {
        //descriptorBinding->descriptorWrites.Clear();
        descriptorBinding->bufferInfos.Clear();
        //descriptorBinding->imageInfos.Clear();
        descriptorBinding->texelBufferViews.Clear();
        descriptorBinding->bufferViews.Clear();
        //descriptorBinding->imageViews.Clear();
        descriptorBinding->samplers.Clear();

        const VkDescriptorSetLayoutBinding& descriptor = descriptorBinding->layoutBinding;

        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numSamplers, descriptor.descriptorCount - startingIndex);
        DKASSERT_DEBUG(availableItems <= numSamplers);

        if (!descriptorBinding->valueSet)
        {
            VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = VK_NULL_HANDLE;
            write.dstBinding = descriptor.binding;
            write.dstArrayElement = startingIndex;
            write.descriptorCount = availableItems;  // number of descriptors to update.
            write.descriptorType = descriptor.descriptorType;
            descriptorBinding->write = write;
            descriptorBinding->valueSet = true;
        }

        VkWriteDescriptorSet& write = descriptorBinding->write;
        if (write.pImageInfo == nullptr)
        {
            descriptorBinding->imageViews.Clear();
            descriptorBinding->imageInfos.Clear();
        }
        write.dstArrayElement = startingIndex;
        write.descriptorCount = availableItems;

        switch (descriptor.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            descriptorBinding->samplers.Reserve(availableItems);
            for (uint32_t i = 0; i < availableItems; ++i)
            {
                VkDescriptorImageInfo* imageInfo = nullptr;
                if (i < descriptorBinding->imageInfos.Count())
                {
                    imageInfo = &descriptorBinding->imageInfos.Value(i);
                }
                else
                {
                    VkDescriptorImageInfo info = {};
                    auto index = descriptorBinding->imageInfos.Add(info);
                    imageInfo = &descriptorBinding->imageInfos.Value(index);
                    DKASSERT_DEBUG(index == i);
                }

                DKASSERT_DEBUG(dynamic_cast<Sampler*>(samplerArray[i]) != nullptr);
                Sampler* sampler = static_cast<Sampler*>(samplerArray[i]);
                DKASSERT_DEBUG(sampler->sampler);

                imageInfo->sampler = sampler->sampler;

                descriptorBinding->samplers.Add(sampler);
            }
            write.pImageInfo = descriptorBinding->imageInfos;
            break;
        default:
            DKLogE("Invalid type!");
            DKASSERT_DESC_DEBUG(0, "Invalid descriptor type!");
            return;
        }
    }
}

#endif //#if DKGL_ENABLE_VULKAN
