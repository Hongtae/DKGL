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
#include "ShaderModule.h"

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

        bool FindDescriptorBinding(uint32_t binding, DKShaderBinding*) const;

        DKObject<DKGraphicsDevice> device;
        DKArray<DKShaderBinding> layout;

        using BufferObject = struct { DKObject<Buffer> buffer; uint64_t offset; };
        using TextureObject = DKObject<Texture>;
        using SamplerObject = DKObject<SamplerState>;
        
        // take ownership of bound resources.
        DKMap<uint32_t, DKArray<BufferObject>> buffers;
        DKMap<uint32_t, DKArray<TextureObject>> textures;
        DKMap<uint32_t, DKArray<SamplerObject>> samplers;

        template <typename BindBuffers, typename BindTextures, typename BindSamplers>
        void BindResources(uint32_t set, const DKArray<ResourceBinding>& bindMap,
                           BindBuffers&& bindBuffers,
                           BindTextures&& bindTextures,
                           BindSamplers&& bindSamplers) const
        {
            for (const ResourceBinding& binding : bindMap)
            {
                if (binding.set == set)
                {
                    if (binding.type == DKShaderResource::TypeBuffer)
                    {
                        if (auto p = this->buffers.Find(binding.binding); p)
                        {
                            const ShaderBindingSet::BufferObject* bufferObjects = p->value;
                            size_t numBuffers = p->value.Count();
                            if (numBuffers > 0)
                                bindBuffers(bufferObjects, binding.bufferIndex, numBuffers);
                        }
                    }
                    else
                    {
                        if (binding.type == DKShaderResource::TypeTexture ||
                            binding.type == DKShaderResource::TypeTextureSampler)
                        {
                            if (auto p = this->textures.Find(binding.binding); p)
                            {
                                const ShaderBindingSet::TextureObject* textureObjects = p->value;
                                size_t numTextures = p->value.Count();
                                if (numTextures > 0)
                                    bindTextures(textureObjects, binding.textureIndex, numTextures);
                            }
                        }
                        if (binding.type == DKShaderResource::TypeSampler ||
                            binding.type == DKShaderResource::TypeTextureSampler)
                        {
                            if (auto p = this->samplers.Find(binding.binding); p)
                            {
                                const ShaderBindingSet::SamplerObject* samplerObjects = p->value;
                                size_t numSamplers = p->value.Count();
                                if (numSamplers > 0)
                                    bindSamplers(samplerObjects, binding.samplerIndex, numSamplers);
                            }
                        }
                    }
                }
            }
        }
    };
}
#endif //#if DKGL_ENABLE_METAL
