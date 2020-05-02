//
//  File: DKMesh.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "DKMesh.h"

using namespace DKFramework;

DKMesh::DKMesh()
{
}

DKMesh::~DKMesh()
{
}

void DKMesh::UpdateMaterialProperties()
{
    if (material)
    {
        material->UpdateDescriptorSets(nullptr);
    }
}

bool DKMesh::EncodeRenderCommand(DKRenderCommandEncoder* encoder) const
{
    if (material && material->Bind(encoder))
    {

    }
    return false;
}
