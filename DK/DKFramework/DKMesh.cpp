//
//  File: DKMesh.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "DKMesh.h"
#include "DKGraphicsDevice.h"

using namespace DKFramework;

DKMesh::DKMesh()
    : primitiveType(DKPrimitiveType::Triangle)
    , vertexStart(0)
    , vertexCount(0)
    , indexBufferByteOffset(0)
    , indexCount(0)
    , indexOffset(0)
    , vertexOffset(0)
    , indexType(DKIndexType::UInt16)
{
}

DKMesh::~DKMesh()
{
}

DKVertexDescriptor DKMesh::VertexDescriptor() const
{
    DKVertexDescriptor descriptor = {};
    descriptor.layouts.Reserve(vertexBuffers.Count());

    size_t numVertexStreams = 0;
    for (const DKVertexBuffer& vb : vertexBuffers)
    {
        numVertexStreams += vb.declarations.Count();
    }
    descriptor.attributes.Reserve(numVertexStreams);
    for (uint32_t index = 0; index < vertexBuffers.Count(); index++)
    {
        const DKVertexBuffer& buffer = vertexBuffers.Value(index);

        size_t numEnabledAttributes = 0;

        for (const DKVertexStreamDeclaration& decl : buffer.declarations)
        {
            DKShaderAttribute attr = {};
            bool found = false;

            if (decl.streamId != DKVertexStream::Unknown)
            {
                if (decl.streamId == DKVertexStream::UserDefine)
                {
                    found = this->material->FindStageInputAttribute(DKShaderStage::Vertex,
                                                                    decl.name,
                                                                    attr);

                }
                else
                {
                    found = this->material->FindStageInputAttribute(DKShaderStage::Vertex,
                                                                    decl.streamId,
                                                                    attr);
                }
            }
            if (found)
            {
                if (attr.enabled)
                {
                    DKVertexAttributeDescriptor desc = {};
                    desc.format = decl.format;
                    desc.offset = (uint32_t)decl.offset;
                    desc.bufferIndex = index;
                    desc.location = attr.location;
                    descriptor.attributes.Add(desc);
                    numEnabledAttributes++;
                }
            }
            else
            {
                DKLogE("Error: Cannot find shader input attributes: %ls (type:%d)",
                       (const wchar_t*)decl.name, decl.streamId);
            }
        }
        if (numEnabledAttributes > 0)
        {
            DKVertexBufferLayoutDescriptor layout = {};
            layout.step = DKVertexStepRate::Vertex;
            layout.stride = (uint32_t)buffer.size;
            layout.bufferIndex = index;
            descriptor.layouts.Add(layout);
        }
    }
    return descriptor;
}

bool DKMesh::BuildPipelineStateObject(DKGraphicsDevice* device)
{
    DKRenderPipelineDescriptor renderPipelineDescriptor = this->material->RenderPipelineDescriptor();
    // setup vertex descriptor..
    renderPipelineDescriptor.vertexDescriptor = this->VertexDescriptor();

    // setup rest of pso descriptor.
    renderPipelineDescriptor.primitiveTopology = this->primitiveType;
    renderPipelineDescriptor.frontFace = this->frontFace;
    renderPipelineDescriptor.cullMode = this->cullMode;

    DKPipelineReflection reflection = {};
    DKObject<DKRenderPipelineState> pso = device->CreateRenderPipeline(renderPipelineDescriptor, &reflection);
    if (!pso)
    {
        return false;
    }

    auto descriptorTypeToResourceType = [](DKShader::DescriptorType type)->DKShaderResource::Type
    {
        switch (type)
        {
        case DKShader::DescriptorTypeUniformBuffer:
        case DKShader::DescriptorTypeStorageBuffer:
        case DKShader::DescriptorTypeUniformTexelBuffer:
        case DKShader::DescriptorTypeStorageTexelBuffer:
            return DKShaderResource::TypeBuffer;
        case DKShader::DescriptorTypeStorageTexture:
        case DKShader::DescriptorTypeTexture:
            return DKShaderResource::TypeTexture;
        case DKShader::DescriptorTypeSampler:
            return DKShaderResource::TypeSampler;
        case DKShader::DescriptorTypeTextureSampler:
            return DKShaderResource::TypeTextureSampler;
        }
        DKASSERT_DEBUG("Invalid type!");
        return {};
    };

    // setup binding table..
    DKArray<ResourceBindingSet> resourceBindings;
    for (const DKShaderResource& res : reflection.resources)
    {
        DKShader::Descriptor shaderDescriptor = {};
        if (!this->material->FindShaderDescriptorForResource({ res.set, res.binding },
                                                             res.stages,
                                                             shaderDescriptor))
        {
            DKLogE("Cannot find shader resource(%ls) descriptor!",
                   (const wchar_t*)res.name);
            return false;
        }

        const DKShaderResource::Type type = descriptorTypeToResourceType(shaderDescriptor.type);
        if (type != res.type)
        {
            DKLogE("Invalid shader resource(%ls) type!",
                   (const wchar_t*)res.name);
            return false;
        }

        //
        ResourceBindingSet* rbset = nullptr;
        for (ResourceBindingSet& rbs : resourceBindings)
        {
            if (rbs.resourceIndex == res.set)
            {
                rbset = &rbs;
                break;
            }
        }
        if (rbset == nullptr)
        {
            ResourceBindingSet rbs = { res.set };
            auto idx = resourceBindings.Add(rbs);
            rbset = &resourceBindings.Value(idx);
        }

        ResourceBinding resource = {};
        resource.resource = res;
        resource.binding = { res.binding };
        resource.binding.type = shaderDescriptor.type;
        resource.binding.arrayLength = shaderDescriptor.count;
        rbset->resources.Add(resource);
    }
    resourceBindings.Sort([](const ResourceBindingSet& lhs, const ResourceBindingSet& rhs)
    {
        return lhs.resourceIndex < rhs.resourceIndex;
    });
    DKShaderBindingSetLayout layout;
    for (ResourceBindingSet& rbs : resourceBindings)
    {
        rbs.resources.Sort([](const ResourceBinding& lhs, const ResourceBinding& rhs)
        {
            return lhs.binding.binding < rhs.binding.binding;
        });

        // Create ResourceBindingSet
        layout.bindings.Clear();
        layout.bindings.Reserve(rbs.resources.Count());
        for (const ResourceBinding& b : rbs.resources)
            layout.bindings.Add(b.binding);

        rbs.bindings = device->CreateShaderBindingSet(layout);
        if (rbs.bindings == nullptr)
        {
            DKLogE("Cannot create ShaderBindingSet(set:%d)", rbs.resourceIndex);
            return false;
        }
    }

    this->renderPipelineState = pso;
    this->pipelineReflection = reflection;
    this->resourceBindings = std::move(resourceBindings);
    return true;
}

const DKPipelineReflection* DKMesh::PipelineReflection() const
{
    if (renderPipelineState)
        return &this->pipelineReflection;
    return nullptr;
}

bool DKMesh::InitResources(DKGraphicsDevice* device, ResourceBufferUsagePolicy bufferUsagePolicy)
{
    if (material == nullptr)
        return false;
    if (renderPipelineState == nullptr)
        BuildPipelineStateObject(device);
    if (renderPipelineState == nullptr)
        return false;

    auto alignAddressNPOT = [](uintptr_t ptr, uintptr_t alignment)->uintptr_t
    {
        if (ptr % alignment)
            ptr += alignment - (ptr % alignment);
        return ptr;
    };
    auto alignAddress = [](uintptr_t ptr, uintptr_t alignment)->uintptr_t
    {
        return (ptr + alignment -1) & ~(alignment - 1);
    };

    size_t numBuffersGenerated = 0;
    size_t totalBytesAllocated = 0;

    if (bufferUsagePolicy == ResourceBufferUsagePolicy::SingleBuffer)
    {
        DKMap<DKString, BufferArray> bufferResourceMap;

        uint64_t bufferOffset = 0;
        uint64_t bufferLength = 0;
        for (const ResourceBindingSet& bset : resourceBindings)
        {
            for (const ResourceBinding& rb : bset.resources)
            {
                if (rb.resource.type == DKShaderResource::TypeBuffer)
                {
                    BufferArray buffers = {};

                    for (uint32_t i = 0; i < rb.resource.count; ++i)
                    {
                        DKMaterial::BufferInfo bufferInfo = { nullptr };
                        bufferInfo.length = rb.resource.typeInfo.buffer.size;
                        bufferInfo.offset = bufferOffset;
                        buffers.Add(bufferInfo);

                        bufferOffset += rb.resource.stride;
                    }
                    bufferLength = bufferOffset + rb.resource.typeInfo.buffer.size;
                    bufferLength = alignAddress(bufferLength, 16);
                    bufferOffset = alignAddress(bufferOffset, 16);

                    bufferResourceMap.Insert(rb.resource.name, buffers);
                }
            }
        }
        if (bufferLength > 0)
        {
            DKObject<DKGpuBuffer> buffer = device->CreateBuffer(bufferLength,
                                                                DKGpuBuffer::StorageModeShared,
                                                                DKCpuCacheMode::DKCpuCacheModeWriteOnly);
            if (!buffer)
            {
                DKLogE("ERROR: Cannot create GPU buffer! (length:%llu)", bufferLength);
                return false;
            }
            numBuffersGenerated++;
            totalBytesAllocated += bufferLength;

            bufferResourceMap.EnumerateForward([&](decltype(bufferResourceMap)::Pair& pair)
            {
                for (DKMaterial::BufferInfo& bufferInfo : pair.value)
                {
                    bufferInfo.buffer = buffer;
                }
                this->bufferProperties.Update(pair);
            });
        }
        //return true;
    }
    else if (bufferUsagePolicy == ResourceBufferUsagePolicy::SingleBufferPerSet)
    {
        DKMap<DKString, BufferArray> bufferResourceMap;

        for (const ResourceBindingSet& bset : resourceBindings)
        {
            uint64_t bufferOffset = 0;
            uint64_t bufferLength = 0;

            for (const ResourceBinding& rb : bset.resources)
            {
                if (rb.resource.type == DKShaderResource::TypeBuffer)
                {
                    BufferArray buffers = {};

                    for (uint32_t i = 0; i < rb.resource.count; ++i)
                    {
                        DKMaterial::BufferInfo bufferInfo = { nullptr };
                        bufferInfo.length = rb.resource.typeInfo.buffer.size;
                        bufferInfo.offset = bufferOffset;
                        buffers.Add(bufferInfo);

                        bufferOffset += rb.resource.stride;
                    }
                    bufferLength = bufferOffset + rb.resource.typeInfo.buffer.size;
                    bufferLength = alignAddress(bufferLength, 16);
                    bufferOffset = alignAddress(bufferOffset, 16);

                    bufferResourceMap.Insert(rb.resource.name, buffers);
                }
            }

            if (bufferLength > 0)
            {
                DKObject<DKGpuBuffer> buffer = device->CreateBuffer(bufferLength,
                                                                    DKGpuBuffer::StorageModeShared,
                                                                    DKCpuCacheMode::DKCpuCacheModeWriteOnly);
                if (!buffer)
                {
                    DKLogE("ERROR: Cannot create GPU buffer! (length:%llu)", bufferLength);
                    return false;
                }
                numBuffersGenerated++;
                totalBytesAllocated += bufferLength;

                bufferResourceMap.EnumerateForward([&](decltype(bufferResourceMap)::Pair& pair)
                {
                    for (DKMaterial::BufferInfo& bufferInfo : pair.value)
                    {
                        if (bufferInfo.buffer == nullptr)
                            bufferInfo.buffer = buffer;
                    }
                });
            }
        }
        bufferResourceMap.EnumerateForward([&](decltype(bufferResourceMap)::Pair& pair)
        {
            this->bufferProperties.Update(pair);
        });
        //return true;
    }
    else if (bufferUsagePolicy == ResourceBufferUsagePolicy::SingleBufferPerResource)
    {
        DKMap<DKString, BufferArray> bufferResourceMap;

        for (const ResourceBindingSet& bset : resourceBindings)
        {
            for (const ResourceBinding& rb : bset.resources)
            {
                if (rb.resource.type == DKShaderResource::TypeBuffer)
                {
                    uint64_t bufferOffset = 0;
                    uint64_t bufferLength = 0;

                    BufferArray buffers = {};

                    for (uint32_t i = 0; i < rb.resource.count; ++i)
                    {
                        DKMaterial::BufferInfo bufferInfo = { nullptr };
                        bufferInfo.length = rb.resource.typeInfo.buffer.size;
                        bufferInfo.offset = bufferOffset;
                        buffers.Add(bufferInfo);

                        bufferOffset += rb.resource.stride;
                    }
                    bufferLength = bufferOffset + rb.resource.typeInfo.buffer.size;
                    bufferLength = alignAddress(bufferLength, 16);
                    bufferOffset = alignAddress(bufferOffset, 16);

                    if (bufferLength > 0)
                    {
                        DKObject<DKGpuBuffer> buffer = device->CreateBuffer(bufferLength,
                                                                            DKGpuBuffer::StorageModeShared,
                                                                            DKCpuCacheMode::DKCpuCacheModeWriteOnly);
                        if (!buffer)
                        {
                            DKLogE("ERROR: Cannot create GPU buffer! (length:%llu)", bufferLength);
                            return false;
                        }
                        numBuffersGenerated++;
                        totalBytesAllocated += bufferLength;

                        for (DKMaterial::BufferInfo& bufferInfo : buffers)
                        {
                            bufferInfo.buffer = buffer;
                        }
                        bufferResourceMap.Insert(rb.resource.name, buffers);
                    }
                }
            }
        }
        bufferResourceMap.EnumerateForward([&](decltype(bufferResourceMap)::Pair& pair)
        {
            this->bufferProperties.Update(pair);
        });
        //return true;
    }
    else
    {
    }
    DKLog("DKMesh::InitResources() generated %llu buffers, %llu bytes",
          numBuffersGenerated, totalBytesAllocated);
    return true;
}

void DKMesh::UpdateMaterialProperties(DKSceneState* scene)
{
    if (material)
    {
        struct Binder : public DKMaterial::ResourceBinder
        {
            DKMesh* mesh;
            Binder(DKMesh* m) : mesh(m) {}
            BufferArray BufferResource(const DKShaderResource& res)
            {
                if (auto p = mesh->bufferProperties.Find(res.name);
                    p && p->value.Count() > 0)
                {
                    return p->value;
                }
                return {};
            }
            TextureArray TextureResource(const DKShaderResource& res)
            {
                if (auto p = mesh->textureProperties.Find(res.name);
                    p && p->value.Count() > 0)
                {
                    return p->value;
                }
                return {};
            }
            SamplerArray SamplerResource(const DKShaderResource& res)
            {
                if (auto p = mesh->samplerProperties.Find(res.name);
                    p && p->value.Count() > 0)
                {
                    return p->value;
                }
                return {};
            }

            // bind struct element separately.
             bool WriteStructElement(const DKString& keyPath,
                                            const DKShaderResourceStructMember& element,
                                            const DKShaderResource& resource,
                                            uint32_t resourceArrayIndex,
                                            BufferWriter* writer)
             {
                 if (auto p = mesh->structElementProperties.Find(keyPath); p)
                 {
                     DKShaderDataTypeSize elementSize = element.dataType;
                     if (elementSize.Bytes() == p->value.typeSize.Bytes())
                     {
                         size_t arraySize = element.count;
                         size_t numItems = p->value.data.Count() / elementSize.Bytes();

                         size_t numBounds = 0;
                         size_t itemsToBind = Min(arraySize, numItems);

                         size_t offset = 0;
                         while (numBounds < itemsToBind)
                         {
                             const uint8_t* ptr = p->value.data;
                             if (!writer->Invoke(&ptr[offset], elementSize.Bytes()))
                                 break;

                             offset += element.stride;
                             numBounds++;
                         }
                         return true;
                     }
                 }
                 return false;
             }
        } binder(this);

        for (ResourceBindingSet& rbs : resourceBindings)
            material->BindResource(rbs, scene, &binder);
    }
}

bool DKMesh::EncodeRenderCommand(DKRenderCommandEncoder* encoder,
                                 uint32_t numInstances,
                                 uint32_t baseInstance) const
{
    if (renderPipelineState && material)
    {
        encoder->SetRenderPipelineState(renderPipelineState);
        for (const ResourceBindingSet& rset : resourceBindings)
        {
            encoder->SetResources(rset.resourceIndex, rset.bindings);
        }

        for (uint32_t index = 0; index < vertexBuffers.Count(); index++)
        {
            const DKVertexBuffer& buffer = vertexBuffers.Value(index);
            encoder->SetVertexBuffer(buffer.buffer, buffer.offset, index);
        }

        if (vertexCount > 0)
        {
            if (indexBuffer)
            {
                encoder->SetIndexBuffer(indexBuffer, indexBufferByteOffset, indexType);
                encoder->DrawIndexed(indexCount, numInstances, indexOffset, vertexOffset, baseInstance);
            }
            else
            {
                encoder->Draw(vertexCount, numInstances, vertexStart, baseInstance);
            }
        }

        return true;
    }
    return false;
}
