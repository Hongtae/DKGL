//
//  File: DKShaderBindingSet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKGpuBuffer.h"
#include "DKTexture.h"
#include "DKSampler.h"
#include "DKShader.h"

namespace DKFramework
{
    struct DKShaderBinding
    {
        uint32_t binding;
        DKShader::DescriptorType type;
        uint32_t length; // array size or bytes of uniform buffer
        DKSamplerState* immutableSamplers;
    };
    struct DKShaderBindingSetLayout
    {
        using Binding = DKShaderBinding;
        DKArray<Binding> bindings;
    };
    /**
    @brief Shader Resource Binding Unit, binding descriptors for single set
    */
    class DKShaderBindingSet
    {
    public:
        virtual ~DKShaderBindingSet() {}

        virtual void SetBuffer(uint32_t binding, DKGpuBuffer*) = 0;
        virtual void SetTexture(uint32_t binding, DKTexture*) = 0;
        virtual void SetSamplerState(uint32_t binding, DKSamplerState*) = 0;
        virtual void SetTextureSampler(uint32_t binding, DKTexture*, DKSamplerState*) = 0;
    };
}
