//
//  File: DKMaterial.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKShader.h"
#include "DKShaderModule.h"
#include "DKShaderBindingSet.h"

namespace DKFramework
{
    class DKMaterial : public DKResource
    {
    public:
        DKMaterial();
        ~DKMaterial();

        DKObject<DKShader> shader;
        DKObject<DKShaderModule> shaderModule;

        struct FunctionProperty
        {
            DKArray<DKShaderBindingSetLayout> bindingSetLayouts;
            DKArray<DKShaderAttribute> inputAttributes;
        };

        DKMap<DKString, FunctionProperty> functionProperties;
    };
}
