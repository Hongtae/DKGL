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

CopyCommandEncoder::Encoder::Encoder(class CommandBuffer* cb)
    : commandBuffer(cb)
{
}

CopyCommandEncoder::Encoder::~Encoder()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(commandBuffer->Queue()->Device());
    VkDevice device = dev->device;
}

bool CopyCommandEncoder::Encoder::Encode(VkCommandBuffer commandBuffer)
{
    // recording commands
    EncodingState state = {};
    for (EncoderCommand* c : commands)
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

void CopyCommandEncoder::CopyFromBufferToBuffer(DKGpuBuffer* src, size_t srcOffset,
                                                DKGpuBuffer* dst, size_t dstOffset,
                                                size_t size)
{
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(dst) != nullptr);

    size_t srcLength = src->Length();
    size_t dstLength = dst->Length();

    if (srcOffset + size > srcLength || dstOffset + size > dstLength)
    {
        DKLogE("DKCopyCommandEncoder::CopyFromBufferToBuffer failed: Invalid buffer region");
        return;
    }

    VkBufferCopy region = {srcOffset, dstOffset, size};

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        vkCmdCopyBuffer(commandBuffer,
                        static_cast<Buffer*>(src)->buffer,
                        static_cast<Buffer*>(dst)->buffer,
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
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<Texture*>(dst) != nullptr);

    DKASSERT_DEBUG((srcOffset.bufferOffset % 4) == 0);

    Buffer* buffer = static_cast<Buffer*>(src);
    Texture* texture = static_cast<Texture*>(dst);

    const Size mipDimensions = {
        Max(texture->Width() >> dstOffset.level, 1U),
        Max(texture->Height() >> dstOffset.level, 1U),
        Max(texture->Depth() >> dstOffset.level, 1U)
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

    DKPixelFormat pixelFormat = texture->PixelFormat();
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
        for (int i = 0; i < region.imageSubresource.layerCount; ++i)
            texture->ChangeLayerLayout(region.imageSubresource.baseArrayLayer + i,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       commandBuffer,
                                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                       VK_PIPELINE_STAGE_TRANSFER_BIT);

        vkCmdCopyBufferToImage(commandBuffer,
                               buffer->buffer,
                               texture->image,
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
    DKASSERT_DEBUG(dynamic_cast<Texture*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(dst) != nullptr);

    DKASSERT_DEBUG((dstOffset.bufferOffset % 4) == 0);

    Texture* texture = static_cast<Texture*>(src);
    Buffer* buffer = static_cast<Buffer*>(dst);

    const Size mipDimensions = {
        Max(texture->Width() >> srcOffset.level, 1U),
        Max(texture->Height() >> srcOffset.level, 1U),
        Max(texture->Depth() >> srcOffset.level, 1U)
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

    DKPixelFormat pixelFormat = texture->PixelFormat();
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
        for (int i = 0; i < region.imageSubresource.layerCount; ++i)
            texture->ChangeLayerLayout(region.imageSubresource.baseArrayLayer + i,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       commandBuffer);

        vkCmdCopyImageToBuffer(commandBuffer,
                               texture->image,
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
    DKASSERT_DEBUG(dynamic_cast<Texture*>(src) != nullptr);
    DKASSERT_DEBUG(dynamic_cast<Texture*>(dst) != nullptr);

    Texture* srcTexture = static_cast<Texture*>(src);
    Texture* dstTexture = static_cast<Texture*>(dst);

    const Size srcMipDimensions = {
        Max(srcTexture->Width() >> srcOffset.level, 1U),
        Max(srcTexture->Height() >> srcOffset.level, 1U),
        Max(srcTexture->Depth() >> srcOffset.level, 1U)
    };
    const Size dstMipDimensions = {
        Max(dstTexture->Width() >> dstOffset.level, 1U),
        Max(dstTexture->Height() >> dstOffset.level, 1U),
        Max(dstTexture->Depth() >> dstOffset.level, 1U)
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

    DKPixelFormat srcPixelFormat = srcTexture->PixelFormat();
    DKPixelFormat dstPixelFormat = dstTexture->PixelFormat();
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

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        for (int i = 0; i < region.srcSubresource.layerCount; ++i)
            srcTexture->ChangeLayerLayout(region.srcSubresource.baseArrayLayer + i,
                                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                          commandBuffer);
        for (int i = 0; i < region.dstSubresource.layerCount; ++i)
            dstTexture->ChangeLayerLayout(region.dstSubresource.baseArrayLayer + i,
                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          commandBuffer,
                                          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                          VK_PIPELINE_STAGE_TRANSFER_BIT);

        vkCmdCopyImage(commandBuffer,
                       srcTexture->image,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       dstTexture->image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &region);
    });
    encoder->commands.Add(command);
    encoder->textures.Add(src);
    encoder->textures.Add(dst);
}

void CopyCommandEncoder::FillBuffer(DKGpuBuffer* buffer, size_t offset, size_t length, uint8_t value)
{
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(buffer) != nullptr);

    size_t bufferLength = buffer->Length();
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
                        static_cast<Buffer*>(buffer)->buffer,
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

#endif //#if DKGL_ENABLE_VULKAN
