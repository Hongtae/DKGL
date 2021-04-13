//
//  File: DKMesh.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKMaterial.h"
#include "DKGpuBuffer.h"
#include "DKVertexDescriptor.h"
#include "DKRenderCommandEncoder.h"

namespace DKFramework
{
    /// vertex stream declarations for single vertex buffer
    struct DKVertexStreamDeclaration
    {
        DKVertexStream streamId;
        DKVertexFormat format;
        bool normalize;    ///< normalize integer to float. (0.0~1.0 for unsigned, -1.0~1.0 for signed)
        uint32_t offset;
        DKString name;
    };
    /// vertex buffer, a buffer can have multiple declarations (multiple stream data)
    struct DKVertexBuffer
    {
        DKArray<DKVertexStreamDeclaration> declarations;
        DKObject<DKGpuBuffer> buffer;
        size_t offset; // first vertex index
        size_t count; // num vertices
        uint32_t size; // vertex size
    };

    struct DKSubMesh
    {
        DKObject<DKGpuBuffer> indexBuffer;
        uint32_t indexCount;
        uint32_t indexOffset;
        uint32_t vertexOffset;
        DKIndexType indexType;
        bool visible;
    };
    /// graphical polygon mesh
    class DKGL_API DKMesh : public DKResource
    {
    public:
        DKMesh();
        ~DKMesh();

        DKArray<DKVertexBuffer> vertexBuffers;
        DKArray<DKSubMesh> subMeshes;
        DKObject<DKMaterial> material;

        void UpdateMaterialProperties();

        bool EncodeRenderCommand(DKRenderCommandEncoder*,
                                 uint32_t numInstances,
                                 uint32_t baseInstance) const;
    };
}
