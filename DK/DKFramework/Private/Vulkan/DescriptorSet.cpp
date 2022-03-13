//
//  File: DescriptorSet.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "GraphicsDevice.h"
#include "DescriptorSet.h"
#include "DescriptorPool.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

DescriptorSet::DescriptorSet(DKGraphicsDevice* dev,
                             DescriptorPool* pool,
                             VkDescriptorSet ds)
    : device(dev)
    , descriptorSet(ds)
    , descriptorPool(pool)
{
}

DescriptorSet::~DescriptorSet()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
    dev->DestroyDescriptorSets(descriptorPool, &descriptorSet, 1);
}

void DescriptorSet::CollectImageViewLayouts(ImageLayoutMap& imageLayouts, ImageViewLayoutMap& viewLayouts)
{
    DKMap<VkImageView, ImageView*> imageViewMap;
    for (Binding& binding : bindings)
    {
        if (!binding.valueSet)
            continue;

        for (ImageView* view : binding.imageViews)
        {
            imageViewMap.Update(view->imageView, view);
        }
    }

    for (Binding& binding : bindings)
    {
        if (!binding.valueSet)
            continue;

        const VkWriteDescriptorSet& write = binding.write;
        const VkDescriptorImageInfo* imageInfo = write.pImageInfo;
        if (imageInfo && imageInfo->imageView != VK_NULL_HANDLE)
        {
            auto p = imageViewMap.Find(imageInfo->imageView);
            DKASSERT_DEBUG(p);
            if (p)
            {
                ImageView* imageView = p->value;
                DKASSERT_DEBUG(imageView->imageView == imageInfo->imageView);

                Image* image = imageView->image;
                VkImageLayout layout = imageInfo->imageLayout;

                if (auto p2 = imageLayouts.Find(image); p2)
                {
                    DKASSERT_DEBUG(p2->value != VK_IMAGE_LAYOUT_UNDEFINED);
                    DKASSERT_DEBUG(imageInfo->imageLayout != VK_IMAGE_LAYOUT_UNDEFINED);

                    if (p2->value != imageInfo->imageLayout)
                    {
                        layout = VK_IMAGE_LAYOUT_GENERAL;
                        p2->value = layout;
                    }
                }
                else
                {
                    imageLayouts.Update(image, imageInfo->imageLayout);
                }

                viewLayouts.Update(imageInfo->imageView, layout);
            }
        }
    }
}

void DescriptorSet::UpdateImageViewLayout(const ImageViewLayoutMap& imageLayouts)
{
    DKArray<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.Reserve(bindings.Count());

    for (Binding& binding : bindings)
    {
        if (!binding.valueSet)
            continue;

        VkWriteDescriptorSet& write = binding.write;
        DKASSERT_DEBUG(write.dstSet == descriptorSet);
        DKASSERT_DEBUG(write.dstBinding == binding.layoutBinding.binding);

        VkDescriptorImageInfo* imageInfo = (VkDescriptorImageInfo*)write.pImageInfo;
        if (imageInfo && imageInfo->imageView != VK_NULL_HANDLE)
        {
            if (auto p = imageLayouts.Find(imageInfo->imageView); p)
            {
                VkImageLayout layout = p->value;
                if (imageInfo->imageLayout != layout)
                {
                    // Update layout
                    imageInfo->imageLayout = layout;
                    descriptorWrites.Add(write);
                }
            }
            else
            {
                //imageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                DKLogE("ERROR! Cannot find proper image layout");
            }
        }
    }
    if (descriptorWrites.Count() > 0)
    {
        GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(device);
        vkUpdateDescriptorSets(dev->device,
                               descriptorWrites.Count(),
                               descriptorWrites,
                               0,
                               nullptr);
    }
}

#endif //#if DKGL_ENABLE_VULKAN
