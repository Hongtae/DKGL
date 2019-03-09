//
//  File: CopyCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "CopyCommandEncoder.h"
#include "Buffer.h"
#include "Texture.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

CopyCommandEncoder::Resources::Resources(class CommandBuffer* b)
    : cb(b)
    , commandBuffer(VK_NULL_HANDLE)
{
}

CopyCommandEncoder::Resources::~Resources()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(cb->Queue()->Device());
    VkDevice device = dev->device;

    if (commandBuffer)
        cb->ReleaseEncodingBuffer(commandBuffer);
}

CopyCommandEncoder::CopyCommandEncoder(VkCommandBuffer vcb, class CommandBuffer* cb)
    : commandBuffer(cb)
{
    resources = DKOBJECT_NEW Resources(cb);
    resources->commandBuffer = vcb;
    DKASSERT_DEBUG(resources->commandBuffer);
}

CopyCommandEncoder::~CopyCommandEncoder()
{
}


void CopyCommandEncoder::AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags flags)
{
    if (semaphore != VK_NULL_HANDLE)
    {
        if (!semaphorePipelineStageMasks.Insert(semaphore, flags))
            semaphorePipelineStageMasks.Value(semaphore) |= flags;
    }
}

void CopyCommandEncoder::AddSignalSemaphore(VkSemaphore semaphore)
{
    if (semaphore != VK_NULL_HANDLE)
        signalSemaphores.Insert(semaphore);
}

void CopyCommandEncoder::EndEncoding()
{
    VkResult err = vkEndCommandBuffer(resources->commandBuffer);
    if (err != VK_SUCCESS)
    {
        DKLogE("ERROR: vkEndCommandBuffer failed: %s", VkResultCStr(err));
        DKASSERT(err == VK_SUCCESS);
    }

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &resources->commandBuffer;

    resources->waitSemaphores.Reserve(semaphorePipelineStageMasks.Count());
    resources->waitStageMasks.Reserve(semaphorePipelineStageMasks.Count());

    semaphorePipelineStageMasks.EnumerateForward([&](decltype(semaphorePipelineStageMasks)::Pair& pair)
    {
        resources->waitSemaphores.Add(pair.key);
        resources->waitStageMasks.Add(pair.value);
    });

    resources->signalSemaphores.Reserve(signalSemaphores.Count());
    signalSemaphores.EnumerateForward([&](VkSemaphore semaphore)
    {
        resources->signalSemaphores.Add(semaphore);
    });

    DKASSERT_DEBUG(resources->waitSemaphores.Count() == resources->waitStageMasks.Count());

    submitInfo.waitSemaphoreCount = resources->waitSemaphores.Count();
    submitInfo.pWaitSemaphores = resources->waitSemaphores;
    submitInfo.pWaitDstStageMask = resources->waitStageMasks;
    submitInfo.signalSemaphoreCount = resources->signalSemaphores.Count();
    submitInfo.pSignalSemaphores = resources->signalSemaphores;

    DKObject<DKOperation> completedCallback = DKFunction([](DKObject<Resources> res)
    {
        res = NULL;
    })->Invocation(resources);

    commandBuffer->Submit(submitInfo, nullptr, completedCallback);

    resources = NULL;
    semaphorePipelineStageMasks.Clear();
    signalSemaphores.Clear();
}

DKCommandBuffer* CopyCommandEncoder::CommandBuffer()
{
    return commandBuffer;
}

void CopyCommandEncoder::CopyFromBufferToBuffer(DKGpuBuffer* src, size_t srcOffset,
                                                DKGpuBuffer* dst, size_t dstOffset,
                                                size_t size)
{
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(dst) != nullptr);

    resources->buffers.Add(src);
    resources->buffers.Add(dst);

    VkBufferCopy region = {srcOffset, dstOffset, size};

    vkCmdCopyBuffer(resources->commandBuffer,
                    static_cast<Buffer*>(src)->buffer,
                    static_cast<Buffer*>(dst)->buffer,
                    1, &region);
}

void CopyCommandEncoder::CopyFromBufferToTexture(DKGpuBuffer* src, const BufferImageOrigin& srcOffset,
                                                 DKTexture* dst, const TextureOrigin& dstOffset,
                                                 const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<Texture*>(dst) != nullptr);

    resources->buffers.Add(src);
    resources->textures.Add(dst);

    Buffer* buffer = static_cast<Buffer*>(src);
    Texture* texture = static_cast<Texture*>(dst);

    VkBufferImageCopy region = {};
    region.bufferOffset = srcOffset.offset;
    region.bufferRowLength = srcOffset.bytesPerRow;
    region.bufferImageHeight = srcOffset.imageHeight;
    region.imageOffset = { (int32_t)dstOffset.x, (int32_t)dstOffset.y,(int32_t)dstOffset.z };
    region.imageExtent = { size.width, size.height, size.depth };
    SetupSubresource(dstOffset, 1, texture->PixelFormat(), region.imageSubresource);

    for (int i = 0; i < region.imageSubresource.layerCount; ++i)
        texture->ChangeLayerLayout(region.imageSubresource.baseArrayLayer + i,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   resources->commandBuffer,
                                   VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                   VK_PIPELINE_STAGE_TRANSFER_BIT);

    vkCmdCopyBufferToImage(resources->commandBuffer,
                           buffer->buffer,
                           texture->image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &region);
}

void CopyCommandEncoder::CopyFromTextureToBuffer(DKTexture* src, const TextureOrigin& srcOffset,
                                                 DKGpuBuffer* dst, const BufferImageOrigin& dstOffset,
                                                 const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<Texture*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(dst) != nullptr);

    resources->textures.Add(src);
    resources->buffers.Add(dst);

    Texture* texture = static_cast<Texture*>(src);
    Buffer* buffer = static_cast<Buffer*>(dst);

    VkBufferImageCopy region = {};
    region.bufferOffset = dstOffset.offset;
    region.bufferRowLength = dstOffset.bytesPerRow;
    region.bufferImageHeight = dstOffset.imageHeight;
    region.imageOffset = { (int32_t)srcOffset.x,(int32_t)srcOffset.y, (int32_t)srcOffset.z };
    region.imageExtent = { size.width, size.height,size.depth };
    SetupSubresource(srcOffset, 1, texture->PixelFormat(), region.imageSubresource);

    for (int i = 0; i < region.imageSubresource.layerCount; ++i)
        texture->ChangeLayerLayout(region.imageSubresource.baseArrayLayer + i,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   resources->commandBuffer);

    vkCmdCopyImageToBuffer(resources->commandBuffer,
                           texture->image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           buffer->buffer,
                           1, &region);
}

void CopyCommandEncoder::CopyFromTextureToTexture(DKTexture* src, const TextureOrigin& srcOffset,
                                                  DKTexture* dst, const TextureOrigin& dstOffset,
                                                  const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<Texture*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<Texture*>(dst) != nullptr);

    resources->textures.Add(src);
    resources->textures.Add(dst);

    Texture* srcTexture = static_cast<Texture*>(src);
    Texture* dstTexture = static_cast<Texture*>(dst);

    VkImageCopy region;
    SetupSubresource(srcOffset, 1, srcTexture->PixelFormat(), region.srcSubresource);
    SetupSubresource(dstOffset, 1, dstTexture->PixelFormat(), region.dstSubresource);

    DKASSERT_DEBUG(region.srcSubresource.aspectMask);
    DKASSERT_DEBUG(region.dstSubresource.aspectMask);

    region.srcOffset = { (int32_t)srcOffset.x, (int32_t)srcOffset.y,(int32_t)srcOffset.z };
    region.dstOffset = { (int32_t)dstOffset.x, (int32_t)dstOffset.y,(int32_t)dstOffset.z };
    region.extent = { size.width, size.height, size.depth };

    for (int i = 0; i < region.srcSubresource.layerCount; ++i)
        srcTexture->ChangeLayerLayout(region.srcSubresource.baseArrayLayer + i,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      resources->commandBuffer);
    for (int i = 0; i < region.dstSubresource.layerCount; ++i)
        dstTexture->ChangeLayerLayout(region.dstSubresource.baseArrayLayer + i,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      resources->commandBuffer,
                                      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT);

    vkCmdCopyImage(resources->commandBuffer,
                   srcTexture->image,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dstTexture->image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &region);
}

void CopyCommandEncoder::FillBuffer(DKGpuBuffer* buffer, size_t offset, size_t length, uint8_t value)
{
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(buffer) != nullptr);
    resources->buffers.Add(buffer);

    uint32_t data = (uint32_t(value) << 24 |
                     uint32_t(value) << 16 |
                     uint32_t(value) << 8 |
                     uint32_t(value));

    vkCmdFillBuffer(resources->commandBuffer,
                    static_cast<Buffer*>(buffer)->buffer,
                    static_cast<VkDeviceSize>(offset),
                    static_cast<VkDeviceSize>(length),
                    data);
}

void CopyCommandEncoder::SetupSubresource(const TextureOrigin& origin,
                                          uint32_t layerCount,
                                          DKPixelFormat pixelFormat,
                                          VkImageSubresourceLayers& subresource)
{
    if (DKPixelFormatIsColorFormat(pixelFormat))
        subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    else
    {
        subresource.aspectMask = 0;
        if (DKPixelFormatIsDepthFormat(pixelFormat))
            subresource.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (DKPixelFormatIsStencilFormat(pixelFormat))
            subresource.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    subresource.mipLevel = origin.level;
    subresource.baseArrayLayer = origin.layer;
    subresource.layerCount = layerCount;
}

#endif //#if DKGL_ENABLE_VULKAN
