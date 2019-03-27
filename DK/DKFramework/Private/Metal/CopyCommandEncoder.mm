//
//  File: CopyCommandEncoder.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "CopyCommandEncoder.h"
#include "Buffer.h"
#include "Texture.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

#pragma mark - CopyCommandEncoder::Encoder
bool CopyCommandEncoder::Encoder::Encode(id<MTLCommandBuffer> buffer)
{
    id<MTLBlitCommandEncoder> encoder = [buffer blitCommandEncoder];
    EncodingState state = {};
    for (EncoderCommand* command : commands )
    {
        command->Invoke(encoder, state);
    }
    [encoder endEncoding];
    return true;
}

#pragma mark - CopyCommandEncoder
CopyCommandEncoder::CopyCommandEncoder(class CommandBuffer* b)
: commandBuffer(b)
{
	encoder = DKOBJECT_NEW Encoder();
	encoder->commands.Reserve(CommandEncoder::InitialNumberOfCommands);
}

CopyCommandEncoder::~CopyCommandEncoder()
{
}

void CopyCommandEncoder::EndEncoding()
{
	DKASSERT_DEBUG(!IsCompleted());
	encoder->commands.ShrinkToFit();
	commandBuffer->EndEncoder(this, encoder);
	encoder = NULL;
}

void CopyCommandEncoder::CopyFromBufferToBuffer(DKGpuBuffer* src, size_t srcOffset,
                                                DKGpuBuffer* dst, size_t dstOffset,
                                                size_t size)
{
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(src));
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(dst));

    DKObject<Buffer> srcBuffer = static_cast<Buffer*>(src);
    DKObject<Buffer> dstBuffer = static_cast<Buffer*>(dst);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLBlitCommandEncoder> encoder, EncodingState& state)
    {
        id<MTLBuffer> src = srcBuffer->buffer;
        id<MTLBuffer> dst = dstBuffer->buffer;

        [encoder copyFromBuffer:src
                   sourceOffset:srcOffset
                       toBuffer:dst
              destinationOffset:dstOffset
                           size:size];
    });
    encoder->commands.Add(command);
}

void CopyCommandEncoder::CopyFromBufferToTexture(DKGpuBuffer* src, const BufferImageOrigin& srcOffset,
                                                 DKTexture* dst, const TextureOrigin& dstOffset,
                                                 const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(src));
    DKASSERT_DEBUG(dynamic_cast<Texture*>(dst));

    DKObject<Buffer> buffer = static_cast<Buffer*>(src);
    DKObject<Texture> texture = static_cast<Texture*>(dst);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLBlitCommandEncoder> encoder, EncodingState& state)
    {
        id<MTLBuffer> src = buffer->buffer;
        id<MTLTexture> dst = texture->texture;

        DKPixelFormat pixelFormat = texture->PixelFormat();
        size_t bytesPerPixel = DKPixelFormatBytesPerPixel(pixelFormat);
        DKASSERT_DEBUG(bytesPerPixel);
        NSUInteger bytesPerRow = srcOffset.imageWidth * bytesPerPixel;
        DKASSERT_DEBUG(bytesPerRow);
        NSUInteger bytesPerImage = bytesPerRow * srcOffset.imageHeight;
        DKASSERT_DEBUG(bytesPerImage);

        [encoder copyFromBuffer:src
                   sourceOffset:srcOffset.bufferOffset
              sourceBytesPerRow:bytesPerRow
            sourceBytesPerImage:bytesPerImage
                     sourceSize:MTLSizeMake(size.width, size.height, size.depth)
                      toTexture:dst
               destinationSlice:dstOffset.layer
               destinationLevel:dstOffset.level
              destinationOrigin:MTLOriginMake(dstOffset.x, dstOffset.y, dstOffset.z)
                        options:MTLBlitOptionNone];
    });
    encoder->commands.Add(command);
}

void CopyCommandEncoder::CopyFromTextureToBuffer(DKTexture* src, const TextureOrigin& srcOffset,
                                                 DKGpuBuffer* dst, const BufferImageOrigin& dstOffset,
                                                 const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<Texture*>(src));
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(dst));

    DKObject<Texture> texture = static_cast<Texture*>(src);
    DKObject<Buffer> buffer = static_cast<Buffer*>(dst);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLBlitCommandEncoder> encoder, EncodingState& state)
    {
        id<MTLTexture> src = texture->texture;
        id<MTLBuffer> dst = buffer->buffer;

        DKPixelFormat pixelFormat = texture->PixelFormat();
        size_t bytesPerPixel = DKPixelFormatBytesPerPixel(pixelFormat);
        DKASSERT_DEBUG(bytesPerPixel);
        NSUInteger bytesPerRow = dstOffset.imageWidth * bytesPerPixel;
        DKASSERT_DEBUG(bytesPerRow);
        NSUInteger bytesPerImage = bytesPerRow * dstOffset.imageHeight;
        DKASSERT_DEBUG(bytesPerImage);

        [encoder copyFromTexture:src
                     sourceSlice:srcOffset.layer
                     sourceLevel:srcOffset.level
                    sourceOrigin:MTLOriginMake(srcOffset.x, srcOffset.y, srcOffset.z)
                      sourceSize:MTLSizeMake(size.width, size.height, size.depth)
                        toBuffer:dst
               destinationOffset:dstOffset.bufferOffset
          destinationBytesPerRow:bytesPerRow
        destinationBytesPerImage:bytesPerImage
                         options:MTLBlitOptionNone];
    });
    encoder->commands.Add(command);
}

void CopyCommandEncoder::CopyFromTextureToTexture(DKTexture* src, const TextureOrigin& srcOffset,
                                                  DKTexture* dst, const TextureOrigin& dstOffset,
                                                  const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<Texture*>(src));
    DKASSERT_DEBUG(dynamic_cast<Texture*>(dst));

    DKObject<Texture> srcTexture = static_cast<Texture*>(src);
    DKObject<Texture> dstTexture = static_cast<Texture*>(dst);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLBlitCommandEncoder> encoder, EncodingState& state)
    {
        id<MTLTexture> src = srcTexture->texture;
        id<MTLTexture> dst = dstTexture->texture;

        [encoder copyFromTexture:src
                     sourceSlice:srcOffset.layer
                     sourceLevel:srcOffset.level
                    sourceOrigin:MTLOriginMake(srcOffset.x, srcOffset.y, srcOffset.z)
                      sourceSize:MTLSizeMake(size.width, size.height, size.depth)
                       toTexture:dst
                destinationSlice:dstOffset.layer
                destinationLevel:dstOffset.level
               destinationOrigin:MTLOriginMake(dstOffset.x, dstOffset.y, dstOffset.z)];
    });
    encoder->commands.Add(command);
}

void CopyCommandEncoder::FillBuffer(DKGpuBuffer* buffer, size_t offset, size_t length, uint8_t value)
{
    DKASSERT_DEBUG(dynamic_cast<Buffer*>(buffer));
    DKObject<Buffer> buf = static_cast<Buffer*>(buffer);

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLBlitCommandEncoder> encoder, EncodingState& state)
    {
        id<MTLBuffer> buffer = buf->buffer;
        [encoder fillBuffer:buffer range:NSMakeRange(offset, length) value:value];
    });
    encoder->commands.Add(command);
}

#endif //#if DKGL_ENABLE_METAL
