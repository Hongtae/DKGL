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

void ShaderBindingSet::SetBuffer(uint32_t binding, DKGpuBuffer*, uint64_t offset, uint64_t length)
{
}

void ShaderBindingSet::SetBufferArray(uint32_t binding, uint32_t numBuffers, BufferInfo*)
{
}

void ShaderBindingSet::SetTexture(uint32_t binding, DKTexture*)
{
}

void ShaderBindingSet::SetTextureArray(uint32_t binding, uint32_t numTextures, DKTexture**)
{
}

void ShaderBindingSet::SetSamplerState(uint32_t binding, DKSamplerState*)
{
}

void ShaderBindingSet::SetSamplerStateArray(uint32_t binding, uint32_t numSamplers, DKSamplerState**)
{
}

#endif //#if DKGL_ENABLE_METAL
