//
//  File: ShaderBindingSet.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include "ShaderBindingSet.h"

using namespace DKFramework;
using namespace DKFramework::Private::Metal;

ShaderBindingSet::ShaderBindingSet(DKGraphicsDevice* dev, const DKShaderBinding* bindings, size_t numBindings)
: device(dev)
, layout(bindings, numBindings)
{
}

ShaderBindingSet::~ShaderBindingSet()
{
    
}

bool ShaderBindingSet::FindDescriptorBinding(uint32_t binding, DKShaderBinding* descriptor) const
{
    for (const DKShaderBinding b : layout)
    {
        if (b.binding == binding)
        {
            *descriptor = b;
            return true;
        }
    }
    return false;
}

void ShaderBindingSet::SetBuffer(uint32_t binding, DKGpuBuffer* bufferObject, uint64_t offset, uint64_t length)
{
    BufferInfo bi = { bufferObject, offset, length };
    SetBufferArray(binding, 1, &bi);
}

void ShaderBindingSet::SetBufferArray(uint32_t binding, uint32_t numBuffers, BufferInfo* bufferArray)
{
    DKShaderBinding descriptor;
    if (FindDescriptorBinding(binding, &descriptor))
    {
        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numBuffers, descriptor.arrayLength - startingIndex);
        DKASSERT_DEBUG(availableItems <= numBuffers);

        DKArray<BufferObject>& bufferObjectArray = buffers.Value(binding);
        bufferObjectArray.Clear();
        bufferObjectArray.Reserve(availableItems);

        for (uint32_t i = 0; i < availableItems; ++i)
        {
            DKASSERT_DEBUG(dynamic_cast<Buffer*>(bufferArray[i].buffer));
            Buffer* buffer = static_cast<Buffer*>(bufferArray[i].buffer);

            BufferObject bo = { buffer, bufferArray[i].offset };
            bufferObjectArray.Add(bo);
        }
    }
}

void ShaderBindingSet::SetTexture(uint32_t binding, DKTexture* texture)
{
    if (texture)
        SetTextureArray(binding, 1, &texture);
}

void ShaderBindingSet::SetTextureArray(uint32_t binding, uint32_t numTextures, DKTexture** textureArray)
{
    DKShaderBinding descriptor;
    if (FindDescriptorBinding(binding, &descriptor))
    {
        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numTextures, descriptor.arrayLength - startingIndex);
        DKASSERT_DEBUG(availableItems <= numTextures);

        DKArray<TextureObject>& textureObjectArray = textures.Value(binding);
        textureObjectArray.Clear();
        textureObjectArray.Reserve(availableItems);

        for (uint32_t i = 0; i < availableItems; ++i)
        {
            DKASSERT_DEBUG(dynamic_cast<Texture*>(textureArray[i]) != nullptr);
            Texture* texture = static_cast<Texture*>(textureArray[i]);
            textureObjectArray.Add(texture);
        }
    }
}

void ShaderBindingSet::SetSamplerState(uint32_t binding, DKSamplerState* sampler)
{
    if (sampler)
        SetSamplerStateArray(binding, 1, &sampler);
}

void ShaderBindingSet::SetSamplerStateArray(uint32_t binding, uint32_t numSamplers, DKSamplerState** samplerArray)
{
    DKShaderBinding descriptor;
    if (FindDescriptorBinding(binding, &descriptor))
    {
        uint32_t startingIndex = 0;
        uint32_t availableItems = Min(numSamplers, descriptor.arrayLength - startingIndex);
        DKASSERT_DEBUG(availableItems <= numSamplers);

        DKArray<SamplerObject>& samplerObjectArray = samplers.Value(binding);
        samplerObjectArray.Clear();
        samplerObjectArray.Reserve(availableItems);

        for (uint32_t i = 0; i < availableItems; ++i)
        {
            DKASSERT_DEBUG(dynamic_cast<SamplerState*>(samplerArray[i]) != nullptr);
            SamplerState* sampler = static_cast<SamplerState*>(samplerArray[i]);
            samplerObjectArray.Add(sampler);
        }
    }
}

#endif //#if DKGL_ENABLE_METAL
