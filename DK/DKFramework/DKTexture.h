//
//  File: DKTexture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKPixelFormat.h"

namespace DKFramework
{
    class DKTexture
    {
    public:
        enum Type
        {
            TypeUnknown = 0,
            Type1D,
            Type2D,
            Type3D,
            TypeCube,
        };
        enum Usage : uint32_t
        {
            UsageUnknown            = 0U,
            UsageCopySource         = 1U,
            UsageCopyDestination    = 1U << 1,
            UsageSampled            = 1U << 2,
            UsageStorage            = 1U << 3,
            UsageShaderRead         = 1U << 4,
            UsageShaderWrite        = 1U << 5,
            UsageRenderTarget       = 1U << 6,
            UsagePixelFormatView    = 1U << 7,
        };

        virtual ~DKTexture() {}

        virtual uint32_t Width() const = 0;
        virtual uint32_t Height() const = 0;
        virtual uint32_t Depth() const = 0;
        virtual uint32_t MipmapCount() const = 0;
        virtual uint32_t ArrayLength() const = 0;

        virtual Type TextureType() const = 0;
        virtual DKPixelFormat PixelFormat() const = 0;
    };

    struct DKTextureDescriptor
    {
        DKTexture::Type textureType;
        DKPixelFormat pixelFormat;

        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mipmapLevels;
        uint32_t sampleCount;
        uint32_t arrayLength;
        uint32_t usage;
    };
}
