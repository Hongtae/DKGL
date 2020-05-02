//
//  File: DKMaterial.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "DKMaterial.h"

using namespace DKFramework;

DKMaterial::DKMaterial()
{
}

DKMaterial::~DKMaterial()
{
}

bool DKMaterial::Build()
{
    return false;
}

bool DKMaterial::UpdateDescriptorSets(const ShaderPropertyCallback*)
{
    return false;
}

bool DKMaterial::Bind(DKRenderCommandEncoder* encoder) const
{
    return false;
}
