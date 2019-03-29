//
//  File: CopyCommandEncoder.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKCopyCommandEncoder.h"
#include "CommandBuffer.h"

namespace DKFramework::Private::Metal
{
	class CopyCommandEncoder : public DKCopyCommandEncoder
	{
	public:
        CopyCommandEncoder(class CommandBuffer*);
		~CopyCommandEncoder();

		// DKCommandEncoder overrides
		void EndEncoding() override;
		bool IsCompleted() const override { return encoder == nullptr; }
		DKCommandBuffer* CommandBuffer() override { return commandBuffer; }

        // DKCopyCommandEncoder
        void CopyFromBufferToBuffer(DKGpuBuffer* src, size_t srcOffset,
                                    DKGpuBuffer* dst, size_t dstOffset,
                                    size_t size) override;
        void CopyFromBufferToTexture(DKGpuBuffer* src, const BufferImageOrigin& srcOffset,
                                     DKTexture* dst, const TextureOrigin& dstOffset,
                                     const Size& size) override;
        void CopyFromTextureToBuffer(DKTexture* src, const TextureOrigin& srcOffset,
                                     DKGpuBuffer* dst, const BufferImageOrigin& dstOffset,
                                     const Size& size) override;
        void CopyFromTextureToTexture(DKTexture* src, const TextureOrigin& srcOffset,
                                      DKTexture* dst, const TextureOrigin& dstOffset,
                                      const Size& size) override;
        void FillBuffer(DKGpuBuffer* buffer, size_t offset, size_t length, uint8_t value) override;

	private:
        struct EncodingState
        {
        };
		using EncoderCommand = DKFunctionSignature<void(id<MTLBlitCommandEncoder>, EncodingState&)>;
		class Encoder : public CommandEncoder
		{
        public:
            bool Encode(id<MTLCommandBuffer> buffer) override;
			DKArray<DKObject<EncoderCommand>> commands;
		};
		DKObject<Encoder> encoder;
		DKObject<class CommandBuffer> commandBuffer;
	};
}
#endif //#if DKGL_ENABLE_METAL
