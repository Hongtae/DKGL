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
#include "Event.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

#pragma mark - CopyCommandEncoder::Encoder
bool CopyCommandEncoder::Encoder::Encode(id<MTLCommandBuffer> buffer)
{
    waitEvents.EnumerateForward([&](Event* event) {
        [buffer encodeWaitForEvent:event->event
                             value:event->NextWaitValue()];
    });
    waitSemaphores.EnumerateForward([&](DKMap<Semaphore*, uint64_t>::Pair& pair) {
        [buffer encodeWaitForEvent:pair.key->event
                             value:pair.value];
    });

    id<MTLBlitCommandEncoder> encoder = [buffer blitCommandEncoder];
    EncodingState state = {};
    for (EncoderCommand* command : commands )
    {
        command->Invoke(encoder, state);
    }
    [encoder endEncoding];

    signalEvents.EnumerateForward([&](Event* event) {
        [buffer encodeSignalEvent:event->event
                            value:event->NextSignalValue()];
    });
    signalSemaphores.EnumerateForward([&](DKMap<Semaphore*, uint64_t>::Pair& pair) {
        [buffer encodeSignalEvent:pair.key->event
                            value:pair.value];
    });

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

void CopyCommandEncoder::WaitEvent(const DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<const Event*>(event));
    encoder->events.Add(const_cast<DKGpuEvent*>(event));
    encoder->waitEvents.Insert(const_cast<Event*>(static_cast<const Event*>(event)));
}

void CopyCommandEncoder::SignalEvent(const DKGpuEvent* event)
{
    DKASSERT_DEBUG(dynamic_cast<const Event*>(event));
    encoder->events.Add(const_cast<DKGpuEvent*>(event));
    encoder->signalEvents.Insert(const_cast<Event*>(static_cast<const Event*>(event)));
}

void CopyCommandEncoder::WaitSemaphoreValue(const DKGpuSemaphore* semaphore, uint64_t value)
{
    DKASSERT_DEBUG(dynamic_cast<const Semaphore*>(semaphore));
    Semaphore* s = const_cast<Semaphore*>(static_cast<const Semaphore*>(semaphore));
    if (auto p = encoder->waitSemaphores.Find(s); p)
    {
        if (value > p->value)
            p->value = value;
    }
    else
    {
        encoder->waitSemaphores.Insert(s, value);
        encoder->semaphores.Add(const_cast<DKGpuSemaphore*>(semaphore));
    }
}

void CopyCommandEncoder::SignalSemaphoreValue(const DKGpuSemaphore* semaphore, uint64_t value)
{
    DKASSERT_DEBUG(dynamic_cast<const Semaphore*>(semaphore));
    Semaphore* s = const_cast<Semaphore*>(static_cast<const Semaphore*>(semaphore));
    if (auto p = encoder->signalSemaphores.Find(s); p)
    {
        if (value > p->value)
            p->value = value;
    }
    else
    {
        encoder->signalSemaphores.Insert(s, value);
        encoder->semaphores.Add(const_cast<DKGpuSemaphore*>(semaphore));
    }
}

void CopyCommandEncoder::CopyFromBufferToBuffer(const DKGpuBuffer* src,
                                                size_t srcOffset,
                                                const DKGpuBuffer* dst,
                                                size_t dstOffset,
                                                size_t size)
{
    DKASSERT_DEBUG(dynamic_cast<const Buffer*>(src));
    DKASSERT_DEBUG(dynamic_cast<const Buffer*>(dst));

    DKObject<Buffer> srcBuffer = const_cast<Buffer*>(static_cast<const Buffer*>(src));
    DKObject<Buffer> dstBuffer = const_cast<Buffer*>(static_cast<const Buffer*>(dst));

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

void CopyCommandEncoder::CopyFromBufferToTexture(const DKGpuBuffer* src,
                                                 const BufferImageOrigin& srcOffset,
                                                 const DKTexture* dst,
                                                 const TextureOrigin& dstOffset,
                                                 const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<const Buffer*>(src));
    DKASSERT_DEBUG(dynamic_cast<const Texture*>(dst));

    DKObject<Buffer> buffer = const_cast<Buffer*>(static_cast<const Buffer*>(src));
    DKObject<Texture> texture = const_cast<Texture*>(static_cast<const Texture*>(dst));

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

void CopyCommandEncoder::CopyFromTextureToBuffer(const DKTexture* src,
                                                 const TextureOrigin& srcOffset,
                                                 const DKGpuBuffer* dst,
                                                 const BufferImageOrigin& dstOffset,
                                                 const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<const Texture*>(src));
    DKASSERT_DEBUG(dynamic_cast<const Buffer*>(dst));

    DKObject<Texture> texture = const_cast<Texture*>(static_cast<const Texture*>(src));
    DKObject<Buffer> buffer = const_cast<Buffer*>(static_cast<const Buffer*>(dst));

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

void CopyCommandEncoder::CopyFromTextureToTexture(const DKTexture* src,
                                                  const TextureOrigin& srcOffset,
                                                  const DKTexture* dst,
                                                  const TextureOrigin& dstOffset,
                                                  const Size& size)
{
    DKASSERT_DEBUG(dynamic_cast<const Texture*>(src));
    DKASSERT_DEBUG(dynamic_cast<const Texture*>(dst));

    DKObject<Texture> srcTexture = const_cast<Texture*>(static_cast<const Texture*>(src));
    DKObject<Texture> dstTexture = const_cast<Texture*>(static_cast<const Texture*>(dst));

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

void CopyCommandEncoder::FillBuffer(const DKGpuBuffer* buffer, size_t offset, size_t length, uint8_t value)
{
    DKASSERT_DEBUG(dynamic_cast<const Buffer*>(buffer));
    DKObject<Buffer> buf = const_cast<Buffer*>(static_cast<const Buffer*>(buffer));

    DKObject<EncoderCommand> command = DKFunction([=](id<MTLBlitCommandEncoder> encoder, EncodingState& state)
    {
        id<MTLBuffer> buffer = buf->buffer;
        [encoder fillBuffer:buffer range:NSMakeRange(offset, length) value:value];
    });
    encoder->commands.Add(command);
}

#endif //#if DKGL_ENABLE_METAL
