//
//  File: ShaderBindingSet.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKShaderBindingSet.h"
#include "../../DKGraphicsDevice.h"
#include "Buffer.h"
#include "Texture.h"
#include "SamplerState.h"

namespace DKFramework::Private::Metal
{
    class ShaderBindingSet : public DKShaderBindingSet
    {
    public:
        ShaderBindingSet(DKGraphicsDevice*, const DKShaderBinding*, size_t);
        ~ShaderBindingSet();

        void SetBuffer(uint32_t binding, DKGpuBuffer*, uint64_t offset, uint64_t length) override;
        void SetBufferArray(uint32_t binding, uint32_t numBuffers, BufferInfo*) override;

        void SetTexture(uint32_t binding, DKTexture*) override;
        void SetTextureArray(uint32_t binding, uint32_t numTextures, DKTexture**) override;

        void SetSamplerState(uint32_t binding, DKSamplerState*) override;
        void SetSamplerStateArray(uint32_t binding, uint32_t numSamplers, DKSamplerState**) override;

        DKObject<DKGraphicsDevice> device;
        DKArray<DKShaderBinding> layout;

        using BufferObject = DKObject<Buffer>;
        using TextureObject = DKObject<Texture>;
        using SamplerObject = DKObject<SamplerState>;
        
        // take ownership of bound resources.
        DKMap<uint32_t, DKArray<BufferObject>> buffers;
        DKMap<uint32_t, DKArray<TextureObject>> textures;
        DKMap<uint32_t, DKArray<SamplerObject>> samplers;
    };
}
#endif //#if DKGL_ENABLE_METAL
