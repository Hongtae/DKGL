//
//  File: CopyCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "CopyCommandEncoder.h"
#include "BufferView.h"
#include "ImageView.h"
#include "GraphicsDevice.h"
#include "Semaphore.h"
#include "TimelineSemaphore.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

CopyCommandEncoder::Encoder::Encoder(class CommandBuffer* cb)
    : commandBuffer(cb)
{
    commands.Reserve(InitialNumberOfCommands);
    setupCommands.Reserve(InitialNumberOfCommands);
    cleanupCommands.Reserve(InitialNumberOfCommands);
}

CopyCommandEncoder::Encoder::~Encoder()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(commandBuffer->Queue()->Device());
    VkDevice device = dev->device;
}

bool CopyCommandEncoder::Encoder::Encode(VkCommandBuffer commandBuffer)
{
    // recording commands
    EncodingState state = { this };
    for (EncoderCommand* c : setupCommands)
    {
        c->Invoke(commandBuffer, state);
    }
    for (EncoderCommand* c : commands)
    {
        c->Invoke(commandBuffer, state);
    }
    for (EncoderCommand* c : cleanupCommands)
    {
        c->Invoke(commandBuffer, state);
    }
    return true;
}

CopyCommandEncoder::CopyCommandEncoder(class CommandBuffer* cb)
    : commandBuffer(cb)
{
    encoder = DKOBJECT_NEW Encoder(cb);
}

void CopyCommandEncoder::EndEncoding()
{
    commandBuffer->EndEncoder(this, encoder);
    encoder = nullptr;
}

void CopyCommandEncoder::WaitEvent(DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<Semaphore*>(event));
    Semaphore* semaphore = static_cast<Semaphore*>(event);

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    encoder->AddWaitSemaphore(semaphore->semaphore, semaphore->NextWaitValue(), pipelineStages);
    encoder->events.Add(event);
}

void CopyCommandEncoder::SignalEvent(DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<Semaphore*>(event));
    Semaphore* semaphore = static_cast<Semaphore*>(event);

    encoder->AddSignalSemaphore(semaphore->semaphore, semaphore->NextSignalValue());
    encoder->events.Add(event);
}

void CopyCommandEncoder::WaitSemaphoreValue(DKGpuSemaphore* sema, uint64_t value)
{
    DKASSERT_DEBUG(dynamic_cast<TimelineSemaphore*>(sema));
    TimelineSemaphore* semaphore = static_cast<TimelineSemaphore*>(sema);

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    encoder->AddWaitSemaphore(semaphore->semaphore, value, pipelineStages);
    encoder->semaphores.Add(semaphore);
}

void CopyCommandEncoder::SignalSemaphoreValue(DKGpuSemaphore* sema, uint64_t value)
{
    DKASSERT_DEBUG(dynamic_cast<TimelineSemaphore*>(sema));
    TimelineSemaphore* semaphore = static_cast<TimelineSemaphore*>(sema);

    encoder->AddSignalSemaphore(semaphore->semaphore, value);
    encoder->semaphores.Add(semaphore);
}

void CopyCommandEncoder::CopyFromBufferToBuffer(DKGpuBuffer* src, size_t srcOffset,
                                                DKGpuBuffer* dst, size_t dstOffset,
                                                size_t size)
{
    DKASSERT_DEBUG(dynamic_cast<BufferView*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<BufferView*>(dst) != nullptr);

    Buffer* srcBuffer = static_cast<BufferView*>(src)->buffer;
    Buffer* dstBuffer = static_cast<BufferView*>(dst)->buffer;

    DKASSERT_DEBUG(srcBuffer &&srcBuffer->buffer);
    DKASSERT_DEBUG(dstBuffer && dstBuffer->buffer);

    size_t srcLength = srcBuffer->Length();
    size_t dstLength = dstBuffer->Length();

    if (srcOffset + size > srcLength || dstOffset + size > dstLength)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromBufferToBuffer failed: Invalid buffer region");
        return;
    }

    VkBufferCopy region = {srcOffset, dstOffset, size};

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        vkCmdCopyBuffer(commandBuffer,
                        srcBuffer->buffer,
                        dstBuffer->buffer,
                        1, &region);
    });
    encoder->commands.Add(command);
    encoder->buffers.Add(src);
    encoder->buffers.Add(dst);
}

void CopyCommandEncoder::CopyFromBufferToTexture(DKGpuBuffer* src, const BufferImageOrigin& srcOffset,
                                                 DKTexture* dst, const TextureOrigin& dstOffset,
                                                 const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<BufferView*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<ImageView*>(dst) != nullptr);

    DKASSERT_DEBUG((srcOffset.bufferOffset % 4) == 0);

    Buffer* buffer = static_cast<BufferView*>(src)->buffer;
    Image* image = static_cast<ImageView*>(dst)->image;

    DKASSERT_DEBUG(buffer && buffer->buffer);
    DKASSERT_DEBUG(image && image->image);

    const Size mipDimensions = {
        Max(image->Width() >> dstOffset.level, 1U),
        Max(image->Height() >> dstOffset.level, 1U),
        Max(image->Depth() >> dstOffset.level, 1U)
    };

    if (dstOffset.x + size.width > mipDimensions.width ||
        dstOffset.y + size.height > mipDimensions.height ||
        dstOffset.z + size.depth > mipDimensions.depth)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromBufferToTexture failed: Invalid texture region");
        return;
    }
    if (size.width > srcOffset.imageWidth || size.height > srcOffset.imageHeight)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromBufferToTexture failed: Invalid buffer region");
        return;
    }

    DKPixelFormat pixelFormat = image->PixelFormat();
    size_t bufferLength = buffer->Length();
    size_t bytesPerPixel = DKPixelFormatBytesPerPixel(pixelFormat);
    DKASSERT_DEBUG(bytesPerPixel > 0);      // Unsupported texture format!

    size_t requiredBufferLengthForCopy = size_t(srcOffset.imageWidth) * size_t(srcOffset.imageHeight) * size.depth * bytesPerPixel + srcOffset.bufferOffset;
    if (requiredBufferLengthForCopy > bufferLength)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromBufferToTexture failed: buffer is too small!");
        return;
    }

    VkBufferImageCopy region = {};
    region.bufferOffset = srcOffset.bufferOffset;
    region.bufferRowLength = srcOffset.imageWidth;
    region.bufferImageHeight = srcOffset.imageHeight;
    region.imageOffset = { (int32_t)dstOffset.x, (int32_t)dstOffset.y,(int32_t)dstOffset.z };
    region.imageExtent = { size.width, size.height, size.depth };
    SetupSubresource(dstOffset, 1, pixelFormat, region.imageSubresource);

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        image->SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         state.encoder->commandBuffer->QueueFamily()->familyIndex,
                         commandBuffer);

        vkCmdCopyBufferToImage(commandBuffer,
                               buffer->buffer,
                               image->image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &region);
    });
    encoder->commands.Add(command);
    encoder->buffers.Add(src);
    encoder->textures.Add(dst);
}

void CopyCommandEncoder::CopyFromTextureToBuffer(DKTexture* src, const TextureOrigin& srcOffset,
                                                 DKGpuBuffer* dst, const BufferImageOrigin& dstOffset,
                                                 const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<ImageView*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<BufferView*>(dst) != nullptr);

    DKASSERT_DEBUG((dstOffset.bufferOffset % 4) == 0);

    Image* image = static_cast<ImageView*>(src)->image;
    Buffer* buffer = static_cast<BufferView*>(dst)->buffer;

    DKASSERT_DEBUG(buffer && buffer->buffer);
    DKASSERT_DEBUG(image && image->image);

    const Size mipDimensions = {
        Max(image->Width() >> srcOffset.level, 1U),
        Max(image->Height() >> srcOffset.level, 1U),
        Max(image->Depth() >> srcOffset.level, 1U)
    };

    if (srcOffset.x + size.width > mipDimensions.width ||
        srcOffset.y + size.height > mipDimensions.height ||
        srcOffset.z + size.depth > mipDimensions.depth)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromTextureToBuffer failed: Invalid texture region");
        return;
    }
    if (size.width > dstOffset.imageWidth || size.height > dstOffset.imageHeight)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromTextureToBuffer failed: Invalid buffer region");
        return;
    }

    DKPixelFormat pixelFormat = image->PixelFormat();
    size_t bufferLength = buffer->Length();
    size_t bytesPerPixel = DKPixelFormatBytesPerPixel(pixelFormat);
    DKASSERT_DEBUG(bytesPerPixel > 0);      // Unsupported texture format!

    size_t requiredBufferLengthForCopy = size_t(dstOffset.imageWidth) * size_t(dstOffset.imageHeight) * size.depth * bytesPerPixel + dstOffset.bufferOffset;
    if (requiredBufferLengthForCopy > bufferLength)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromTextureToBuffer failed: buffer is too small!");
        return;
    }

    VkBufferImageCopy region = {};
    region.bufferOffset = dstOffset.bufferOffset;
    region.bufferRowLength = dstOffset.imageWidth;
    region.bufferImageHeight = dstOffset.imageHeight;
    region.imageOffset = { (int32_t)srcOffset.x,(int32_t)srcOffset.y, (int32_t)srcOffset.z };
    region.imageExtent = { size.width, size.height,size.depth };
    SetupSubresource(srcOffset, 1, pixelFormat, region.imageSubresource);

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        image->SetLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_ACCESS_TRANSFER_READ_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         state.encoder->commandBuffer->QueueFamily()->familyIndex,
                         commandBuffer);

        vkCmdCopyImageToBuffer(commandBuffer,
                               image->image,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               buffer->buffer,
                               1, &region);
    });
    encoder->commands.Add(command);
    encoder->textures.Add(src);
    encoder->buffers.Add(dst);
}

void CopyCommandEncoder::CopyFromTextureToTexture(DKTexture* src, const TextureOrigin& srcOffset,
                                                  DKTexture* dst, const TextureOrigin& dstOffset,
                                                  const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<ImageView*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<ImageView*>(dst) != nullptr);

    Image* srcImage = static_cast<ImageView*>(src)->image;
    Image* dstImage = static_cast<ImageView*>(dst)->image;

    DKASSERT_DEBUG(srcImage && srcImage->image);
    DKASSERT_DEBUG(dstImage && dstImage->image);

    const Size srcMipDimensions = {
        Max(srcImage->Width() >> srcOffset.level, 1U),
        Max(srcImage->Height() >> srcOffset.level, 1U),
        Max(srcImage->Depth() >> srcOffset.level, 1U)
    };
    const Size dstMipDimensions = {
        Max(dstImage->Width() >> dstOffset.level, 1U),
        Max(dstImage->Height() >> dstOffset.level, 1U),
        Max(dstImage->Depth() >> dstOffset.level, 1U)
    };

    if (srcOffset.x + size.width > srcMipDimensions.width ||
        srcOffset.y + size.height > srcMipDimensions.height ||
        srcOffset.z + size.depth > srcMipDimensions.depth)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromTextureToTexture failed: Invalid source texture region");
        return;
    }
    if (dstOffset.x + size.width > dstMipDimensions.width ||
        dstOffset.y + size.height > dstMipDimensions.height ||
        dstOffset.z + size.depth > dstMipDimensions.depth)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromTextureToTexture failed: Invalid destination texture region");
        return;
    }

    DKPixelFormat srcPixelFormat = srcImage->PixelFormat();
    DKPixelFormat dstPixelFormat = dstImage->PixelFormat();
    size_t srcBytesPerPixel = DKPixelFormatBytesPerPixel(srcPixelFormat);
    size_t dstBytesPerPixel = DKPixelFormatBytesPerPixel(dstPixelFormat);

    DKASSERT_DEBUG(srcBytesPerPixel > 0);      // Unsupported texture format!
    DKASSERT_DEBUG(dstBytesPerPixel > 0);      // Unsupported texture format!

    if (srcBytesPerPixel != dstBytesPerPixel)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromTextureToTexture failed: Incompatible pixel formats");
        return;
    }

    VkImageCopy region = {};
    SetupSubresource(srcOffset, 1, srcPixelFormat, region.srcSubresource);
    SetupSubresource(dstOffset, 1, dstPixelFormat, region.dstSubresource);

    DKASSERT_DEBUG(region.srcSubresource.aspectMask);
    DKASSERT_DEBUG(region.dstSubresource.aspectMask);

    region.srcOffset = { (int32_t)srcOffset.x, (int32_t)srcOffset.y,(int32_t)srcOffset.z };
    region.dstOffset = { (int32_t)dstOffset.x, (int32_t)dstOffset.y,(int32_t)dstOffset.z };
    region.extent = { size.width, size.height, size.depth };

    VkImageMemoryBarrier imageMemoryBarriers[2] = { { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER }, { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER } };
    imageMemoryBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarriers[0].image = srcImage->image;
    SetupSubresource(srcOffset, 1, 1, srcPixelFormat, imageMemoryBarriers[0].subresourceRange);

    imageMemoryBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarriers[1].image = srcImage->image;
    SetupSubresource(dstOffset, 1, 1, dstPixelFormat, imageMemoryBarriers[1].subresourceRange);

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        srcImage->SetLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            VK_ACCESS_TRANSFER_READ_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            state.encoder->commandBuffer->QueueFamily()->familyIndex,
                            commandBuffer);

        dstImage->SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            state.encoder->commandBuffer->QueueFamily()->familyIndex,
                            commandBuffer);

        vkCmdCopyImage(commandBuffer,
                       srcImage->image,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       dstImage->image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &region);
    });
    encoder->commands.Add(command);
    encoder->textures.Add(src);
    encoder->textures.Add(dst);
}

void CopyCommandEncoder::FillBuffer(DKGpuBuffer* buffer, size_t offset, size_t length, uint8_t value)
{
    DKASSERT_DEBUG(dynamic_cast<BufferView*>(buffer) != nullptr);
    Buffer* buf = static_cast<BufferView*>(buffer)->buffer;
    DKASSERT_DEBUG(buf && buf->buffer);

    size_t bufferLength = buf->Length();
    if (offset + length > bufferLength)
    {
        DKLogE("DKCopyCommandEncoder::FillBuffer failed: Invalid buffer region");
        return;
    }

    uint32_t data = (uint32_t(value) << 24 |
                     uint32_t(value) << 16 |
                     uint32_t(value) << 8 |
                     uint32_t(value));

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        vkCmdFillBuffer(commandBuffer,
                        static_cast<Buffer*>(buf)->buffer,
                        static_cast<VkDeviceSize>(offset),
                        static_cast<VkDeviceSize>(length),
                        data);
    });
    encoder->commands.Add(command);
    encoder->buffers.Add(buffer);
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

void CopyCommandEncoder::SetupSubresource(const TextureOrigin& origin,
                                          uint32_t layerCount,
                                          uint32_t levelCount,
                                          DKPixelFormat pixelFormat,
                                          VkImageSubresourceRange& subresource)
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
    subresource.baseMipLevel = origin.level;
    subresource.baseArrayLayer = origin.layer;
    subresource.layerCount = layerCount;
    subresource.levelCount = levelCount;
}

#endif //#if DKGL_ENABLE_VULKAN
