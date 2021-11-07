//
//  File: DKCopyCommandEncoder.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCommandEncoder.h"
#include "DKTexture.h"
#include "DKGpuBuffer.h"

namespace DKFramework
{
	/// @brief CommandEncoder for GPU copy(blit) operation
    class DKCopyCommandEncoder : public DKCommandEncoder
    {
    public:
        enum Option
        {
            OptionNone                       = 0,
            OptionDepthFromDepthStencil      = 1 << 0,
            OptionStencilFromDepthStencil    = 1 << 1,
        };

        struct Size
        {
            uint32_t width, height, depth;
        };
        struct TextureOrigin
        {
            uint32_t layer;
            uint32_t level;
            uint32_t x, y, z;       ///< pixel offset
        };
        struct BufferImageOrigin
        {
            size_t bufferOffset;    ///< buffer offset (bytes)
            uint32_t imageWidth;    ///< buffer image's width (pixels)
            uint32_t imageHeight;   ///< buffer image's height (pixels)
        };

        virtual ~DKCopyCommandEncoder() {}

        /// Copying Data Between Buffers
        virtual void CopyFromBufferToBuffer(const DKGpuBuffer* src,
                                            size_t srcOffset,
                                            const DKGpuBuffer* dst,
                                            size_t dstOffset,
                                            size_t size) = 0;

        /// Copying Between Textures and Buffers
        virtual void CopyFromBufferToTexture(const DKGpuBuffer* src,
                                             const BufferImageOrigin& srcOffset,
                                             const DKTexture* dst,
                                             const TextureOrigin& dstOffset,
                                             const Size& size) = 0;

        virtual void CopyFromTextureToBuffer(const DKTexture* src,
                                             const TextureOrigin& srcOffset,
                                             const DKGpuBuffer* dst,
                                             const BufferImageOrigin& dstOffset,
                                             const Size& size) = 0;

        /// Copying Data Between Textures
        virtual void CopyFromTextureToTexture(const DKTexture* src,
                                              const TextureOrigin& srcOffset,
                                              const DKTexture* dst,
                                              const TextureOrigin& dstOffset,
                                              const Size& size) = 0;

        /// Filling Buffers
        virtual void FillBuffer(const DKGpuBuffer* buffer,
                                size_t offset,
                                size_t length,
                                uint8_t value) = 0;
    };
}
