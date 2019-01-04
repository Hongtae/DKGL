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
            UsageUnknown = 0,
            UsageCopySource = 1,
            UsageCopyDestination = 1 << 1,
            UsageSampled = 1 << 2,
            UsageStorage = 1 << 3,
            UsageShaderRead = 1 << 4,
            UsageShaderWrite = 1 << 5,
            UsageRenderTarget = 1 << 6,
            UsagePixelFormatView = 1 << 7,
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
