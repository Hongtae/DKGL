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
#include "DKPipelineReflection.h"

namespace DKFramework
{
    /// vertex stream declarations for single vertex buffer
    struct DKVertexStreamDeclaration
    {
        DKVertexStream streamId;
        DKVertexFormat format;
        size_t offset; // where the data begins, in bytes
        DKString name;
    };
    /// vertex buffer, a buffer can have multiple declarations (multiple stream data)
    struct DKVertexBuffer
    {
        DKArray<DKVertexStreamDeclaration> declarations;
        DKObject<DKGpuBuffer> buffer;
        size_t offset; // first vertex index
        size_t count; // num vertices
        size_t size; // vertex size
    };

    class DKSceneState;
    /// graphical polygon mesh
    class DKGL_API DKMesh : public DKResource
    {
    public:
        DKMesh();
        ~DKMesh();

        DKArray<DKVertexBuffer> vertexBuffers;
        DKObject<DKGpuBuffer> indexBuffer;

        DKObject<DKMaterial> material;

        DKPrimitiveType primitiveType;
        DKCullMode cullMode = DKCullMode::Back;
        DKFrontFace frontFace = DKFrontFace::CCW;

        uint32_t vertexStart;
        uint32_t vertexCount;
        uint32_t indexBufferByteOffset;
        uint32_t indexCount;
        uint32_t indexOffset;
        uint32_t vertexOffset;
        DKIndexType indexType;

        /// @brief resource buffer allocation, binding policy
        enum class ResourceBufferUsagePolicy
        {
            UseExternalBufferManually = 0,  ///< don't alloc buffer, use external resources manually.
            SingleBuffer,       ///< single buffer per mesh
            SingleBufferPerSet, ///< single buffer per descriptor-set
            SingleBufferPerResource,   ///< separated buffers for each resources
        };

        bool InitResources(DKGraphicsDevice* device,
                           ResourceBufferUsagePolicy p = ResourceBufferUsagePolicy::SingleBuffer);

        void UpdateMaterialProperties(DKSceneState* scene);

        bool EncodeRenderCommand(DKRenderCommandEncoder*,
                                 uint32_t numInstances,
                                 uint32_t baseInstance) const;

        DKVertexDescriptor VertexDescriptor() const;

        // material properties (override)
        using TextureArray = DKMaterial::TextureArray;
        using BufferArray = DKMaterial::BufferArray;
        using SamplerArray = DKMaterial::SamplerArray;
        using StructElementProperty = DKMaterial::StructElementProperty;

        DKMap<DKString, BufferArray> bufferProperties;
        DKMap<DKString, TextureArray> textureProperties;
        DKMap<DKString, SamplerArray> samplerProperties;
        DKMap<DKString, StructElementProperty> structElementProperties;

        const DKPipelineReflection* PipelineReflection() const;
    private:
        DKObject<DKRenderPipelineState> renderPipelineState;
        DKPipelineReflection pipelineReflection;

        using ResourceBinding = DKMaterial::ResourceBinding;
        using ResourceBindingSet = DKMaterial::ResourceBindingSet;
        DKArray<ResourceBindingSet> resourceBindings;

        bool BuildPipelineStateObject(DKGraphicsDevice* device);
    };
}
