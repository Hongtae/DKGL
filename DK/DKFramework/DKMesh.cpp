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

bool DKMesh::EncodeRenderCommand(DKRenderCommandEncoder* encoder,
                                 uint32_t numInstances,
                                 uint32_t baseInstance) const
{
    if (material && material->EncodeRenderCommand(encoder))
    {
        for (uint32_t index = 0; index < vertexBuffers.Count(); index++)
        {
            const DKVertexBuffer& buffer = vertexBuffers.Value(index);
            encoder->SetVertexBuffer(buffer.buffer, buffer.offset, index);
        }

        for (const DKSubMesh& subMesh : subMeshes)
        {
            if (subMesh.visible && subMesh.indexCount > 0)
            {
                const DKGpuBuffer* indexBuffer = subMesh.indexBuffer;
                encoder->SetIndexBuffer(indexBuffer, 0, subMesh.indexType);
                encoder->DrawIndexed(subMesh.indexCount, numInstances, subMesh.indexOffset, subMesh.vertexOffset, baseInstance);
            }
        }
    }
    return false;
}
