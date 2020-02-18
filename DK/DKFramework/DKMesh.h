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

namespace DKFramework
{
    /// vertex stream declarations for single vertex buffer
    struct DKVertexStreamDeclaration
    {
        DKVertexStream streamId;
        DKVertexFormat format;
        bool normalize;    ///< normalize integer to float. (0.0~1.0 for unsigned, -1.0~1.0 for signed)
        size_t offset;
        DKString name;
    };
    /// vertex buffer, a buffer can have multiple declarations (multiple stream data)
    struct DKVertexBuffer
    {
        DKArray<DKVertexStreamDeclaration> declarations;
        DKObject<DKGpuBuffer> buffer;
        size_t count;
        size_t size;
    };

    struct DKSubMesh
    {
        DKObject<DKGpuBuffer> indexBuffer;
        bool visible;
    };
    /// graphical polygon mesh
    class DKMesh : public DKResource
    {
    public:
        DKMesh();
        ~DKMesh();

        DKArray<DKVertexBuffer> vertexBuffers;
        DKArray<DKSubMesh> subMeshes;
        DKObject<DKMaterial> material;
    };
}
