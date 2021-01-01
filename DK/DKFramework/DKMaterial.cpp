//
//  File: DKMaterial.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "DKMaterial.h"
#include "DKGraphicsDevice.h"

using namespace DKFramework;

DKMaterial::DKMaterial()
{
}

DKMaterial::~DKMaterial()
{
}

bool DKMaterial::FindShaderDescriptorForResource(ResourceIndex index,
                                                 uint32_t stages,
                                                 DKShader::Descriptor& descriptor) const
{
    bool found = false;
    shaderTemplates.EnumerateForward([&](const decltype(shaderTemplates)::Pair& pair,
                                         bool* stop)
    {
        if (stages & static_cast<uint32_t>(pair.key))
        {
            for (const DKShader::Descriptor& desc : pair.value.shader->Descriptors())
            {
                if (desc.set == index.set && desc.binding == index.binding)
                {
                    descriptor = desc;
                    found = true;
                    *stop = true;
                }
            }
        }
    });
    return found;
}

bool DKMaterial::FindStageInputAttribute(DKShaderStage stage, DKVertexStream stream,  const DKString& name, DKShaderAttribute& outAttr) const
{
    auto p = shaderTemplates.Find(stage);
    if (p && p->value.shader && p->value.shader->Stage() == stage && p->value.shaderFunction)
    {
        for (const DKShaderAttribute& attr : p->value.shader->InputAttributes())
        {
            if (attr.name == name)
            {
                DKVertexStream st = DKVertexStream::UserDefine;
                auto p2 = p->value.inputAttributeTypes.Find(attr.name);
                if (p2)
                    st = p2->value;

                if (st == stream)
                {
                    outAttr = attr;
                    return true;
                }
            }
        }
    }
    return false;
}

bool DKMaterial::FindStageInputAttribute(DKShaderStage stage, DKVertexStream stream, DKShaderAttribute& outAttr) const
{
    auto p = shaderTemplates.Find(stage);
    if (p && p->value.shader && p->value.shader->Stage() == stage && p->value.shaderFunction)
    {
        for (const DKShaderAttribute& attr : p->value.shader->InputAttributes())
        {
            DKVertexStream st = DKVertexStream::UserDefine;
            auto p2 = p->value.inputAttributeTypes.Find(attr.name);
            if (p2)
                st = p2->value;

            if (st == stream)
            {
                outAttr = attr;
                return true;
            }
        }
    }
    return false;
}

bool DKMaterial::FindStageInputAttribute(DKShaderStage stage, const DKString& name, DKShaderAttribute& outAttr) const
{
    auto p = shaderTemplates.Find(stage);
    if (p && p->value.shader && p->value.shader->Stage() == stage && p->value.shaderFunction)
    {
        for (const DKShaderAttribute& attr : p->value.shader->InputAttributes())
        {
            if (attr.name == name)
            {
                outAttr = attr;
                return true;
            }
        }
    }
    return false;
}

DKShaderUniform DKMaterial::ShaderUniformForResource(ResourceIndex index, uint32_t stages) const
{
    DKShaderUniform uniform = DKShaderUniform::Unknown;

    shaderTemplates.EnumerateForward([&](const decltype(shaderTemplates)::Pair& pair,
                                         bool* stop)
    {
        if ((static_cast<uint32_t>(pair.key) & stages) &&
            pair.value.shader &&
            pair.key == pair.value.shader->Stage() &&
            pair.value.shaderFunction)
        {
            for (const DKShaderResource& res : pair.value.shader->Resources())
            {
                if (res.set == index.set && res.binding == index.binding)
                {
                    auto p = pair.value.resourceTypes.Find(res.name);
                    if (p)
                    {
                        uniform = p->value;
                        if (uniform == DKShaderUniform::Unknown)
                        {
                            DKLogE("ShaderUniform(%ls) was found, but type is Unknown!",
                                   (const wchar_t*)res.name);
                        }
                    }
                    break;
                }
            }

            if (uniform != DKShaderUniform::Unknown)
                *stop = true; // stop iterating
        }
    });
    return uniform;
}

DKShaderUniform DKMaterial::ShaderUniformForResource(const DKString& keyPath, uint32_t stages) const
{
    DKShaderUniform uniform = DKShaderUniform::Unknown;

    shaderTemplates.EnumerateForward([&](const decltype(shaderTemplates)::Pair& pair,
                                         bool* stop)
    {
        if ((static_cast<uint32_t>(pair.key) & stages) &&
            pair.value.shader &&
            pair.key == pair.value.shader->Stage() &&
            pair.value.shaderFunction)
        {
            auto p = pair.value.resourceTypes.Find(keyPath);
            if (p)
            {
                uniform = p->value;
                if (uniform == DKShaderUniform::Unknown)
                {
                    DKLogE("ShaderUniform(%ls) was found, but type is Unknown!",
                           (const wchar_t*)keyPath);
                }
            }
            if (uniform != DKShaderUniform::Unknown)
                *stop = true; // stop iterating
        }
    });
    return uniform;
}

DKRenderPipelineDescriptor DKMaterial::RenderPipelineDescriptor() const
{
    auto findShader = [this](DKShaderStage stage) -> DKObject<DKShaderFunction>
    {
        if (auto p = this->shaderTemplates.Find(stage);
            p && p->value.shader && p->value.shader->Stage() == stage && p->value.shaderFunction)
        {
            //DKString name = p->value.shaderFunction->FunctionName();
            for (const DKString& s : p->value.shader->FunctionNames())
            {
                if (s == p->value.shaderFunction->FunctionName())
                {
                    return p->value.shaderFunction;
                }
            }
        }
        return nullptr;
    };

    DKRenderPipelineDescriptor renderPipelineDescriptor = {};
    renderPipelineDescriptor.vertexFunction = findShader(DKShaderStage::Vertex);
    renderPipelineDescriptor.fragmentFunction = findShader(DKShaderStage::Fragment);

    renderPipelineDescriptor.colorAttachments.Resize(1);
    renderPipelineDescriptor.colorAttachments.Value(0).pixelFormat = DKPixelFormat::BGRA8Unorm;
    renderPipelineDescriptor.colorAttachments.Value(0).blendState.enabled = true;
    renderPipelineDescriptor.colorAttachments.Value(0).blendState.sourceRGBBlendFactor = DKBlendFactor::SourceAlpha;
    renderPipelineDescriptor.colorAttachments.Value(0).blendState.destinationRGBBlendFactor = DKBlendFactor::OneMinusSourceAlpha;
    renderPipelineDescriptor.depthStencilAttachmentPixelFormat = DKPixelFormat::Invalid; // no depth buffer
    renderPipelineDescriptor.primitiveTopology = DKPrimitiveType::Triangle;
    renderPipelineDescriptor.frontFace = DKFrontFace::CCW;
    renderPipelineDescriptor.triangleFillMode = DKTriangleFillMode::Fill;
    renderPipelineDescriptor.depthClipMode = DKDepthClipMode::Clip;
    renderPipelineDescriptor.cullMode = DKCullMode::None;
    renderPipelineDescriptor.rasterizationEnabled = true;

    return renderPipelineDescriptor;
}

bool DKMaterial::BindResource(ResourceBindingSet& rbset, DKSceneState* scene, ResourceBinder* resourceBinder)
{
    struct StructElementEnumerator
    {
        ResourceBinder* binder;
        const DKShaderResource& base;
        DKString parentKeyPath;
        uint32_t arrayIndex;
        uint32_t offset;
        uint8_t* buffer;
        size_t bufferLength;

        bool Bind(const DKShaderResourceStruct& str)
        {
            DKASSERT_DEBUG(base.type == DKShaderResource::TypeBuffer);

            bool result = true;
            for (const DKShaderResourceStructMember& mem : str.members)
            {
                const DKString keyPath = DKString(parentKeyPath).Append(".").Append(mem.name);

                const DKShaderDataType type = mem.dataType;
                const uint32_t memberOffset = mem.offset + offset;

                DKObject<ResourceBinder::BufferWriter> bufferWriter = 
                    DKFunction([&](const void* data, size_t length)
                {
                    if (length + memberOffset <= bufferLength)
                    {
                        memcpy(&buffer[memberOffset], data, length);
                        return true;
                    }
                    return false;
                });

                // find resource with keyPath
                bool bound = binder->WriteStructElement(keyPath,
                                                        mem,
                                                        base,
                                                        arrayIndex,
                                                        bufferWriter);

                if (!bound)
                {
                    if (type == DKShaderDataType::Struct)
                    {
                        if (auto p = base.structTypeMemberMap.Find(mem.typeInfoKey); p)
                        {
                            bound = StructElementEnumerator{
                                binder,
                                base,
                                keyPath,
                                arrayIndex,
                                memberOffset,
                                buffer,
                                bufferLength,
                            }.Bind(p->value);
                        }
                    }
                }
                if (!bound)
                {
                    result = false;
                    DKLogW("ERROR: Resource:(%ls, set:%d, bind:%d, offset:%d) failed to bind.",
                           (const wchar_t*)keyPath,
                           base.set, base.binding, memberOffset);
                }
            }
            return result;
        }
    };

    auto bindTextureArray = [](const DKShaderResource& res,
                               ResourceBinder* binder,
                               DKShaderBindingSet* bindingSet)
    {
        DKASSERT_DEBUG(res.type == DKShaderResource::TypeTexture ||
                       res.type == DKShaderResource::TypeTextureSampler);

        TextureArray textures = binder->TextureResource(res);
        DKArray<DKTexture*> textureArray;
        textureArray.Reserve(textures.Count());
        for (DKTexture* tex : textures)
        {
            if (tex)
                textureArray.Add(tex);
            else
                return size_t(0);
        }
        if (textureArray.Count() > 0)
        {
            bindingSet->SetTextureArray(res.binding, textureArray.Count(), textureArray);
        }
        return textureArray.Count();
    };

    auto bindSamplerArray = [](const DKShaderResource& res,
                               ResourceBinder* binder,
                               DKShaderBindingSet* bindingSet)
    {
        DKASSERT_DEBUG(res.type == DKShaderResource::TypeSampler ||
                       res.type == DKShaderResource::TypeTextureSampler);

        SamplerArray samplers = binder->SamplerResource(res);
        DKArray<DKSamplerState*> samplerArray;
        samplerArray.Reserve(samplers.Count());
        for (DKSamplerState* sam : samplers)
        {
            if (sam)
                samplerArray.Add(sam);
            else
                return size_t(0);
        }
        if (samplerArray.Count() > 0)
        {
            bindingSet->SetSamplerStateArray(res.binding, samplerArray.Count(), samplerArray);
        }
        return samplerArray.Count();
    };

    // Create forward binder
    // 1. bind pre-defined for pre-defined constant
    // 2. bind user data for UserDefined.
    // 3. bind default value (if no.2 has been failed)

    DKObject<ResourceBinder> binder = this->SceneResourceBinder(scene, resourceBinder);
    if (binder == nullptr)
        binder = resourceBinder;
    if (binder == nullptr)
    {
        DKLogE("Cannot access resource binder for material!");
        return false;
    }

    size_t numBounds = 0;
    size_t numErrors = 0;

    for (const ResourceBinding& rb : rbset.resources)
    {
        const DKShaderResource& res = rb.resource;

        bool bound = false;
        if (res.type == DKShaderResource::TypeBuffer)
        {
            // bind buffer.
            if (BufferArray buffers = resourceBinder->BufferResource(res); buffers.Count() > 0)
            {
                DKASSERT_DEBUG(res.set == rbset.resourceIndex);

                DKArray<DKShaderBindingSet::BufferInfo> bufferInfos;
                bufferInfos.Reserve(buffers.Count());

                for (uint32_t index = 0; index < buffers.Count(); index++)
                {
                    BufferInfo& bi = buffers.Value(index);

                    if (bi.buffer &&
                        bi.offset + bi.length <= bi.buffer->Length())
                    {
                        size_t resourceSize = res.typeInfo.buffer.size;

                        if (auto p = res.structTypeMemberMap.Find(res.typeInfoKey); p)
                        {
                            if (uint8_t* ptr = reinterpret_cast<uint8_t*>(bi.buffer->Contents());
                                ptr)
                            {
                                if (StructElementEnumerator{
                                    resourceBinder,
                                    res,
                                    res.name,
                                    index,  // buffer array index
                                    0,
                                    &ptr[bi.offset],
                                    bi.length
                                    }.Bind(p->value))
                                {
                                    bi.buffer->Flush();
                                }
                            }
                            else
                            {
                                DKLogE("ERROR: Cannot map buffer for resource:%ls, check StorageMode!",
                                       (const wchar_t*)res.name);
                                // TODO: Create host buffer (StorageModeShared) and copy
                            }
                        }
                        bufferInfos.Add({bi.buffer, bi.offset, bi.length});
                    }
                    else // error!
                    {
                        DKLogE("ERROR: Buffer is null or too small (resource:%ls)",
                               (const wchar_t*)res.name);
                        bufferInfos.Clear();
                        break;
                    }
                }
                if (bufferInfos.Count() > 0)
                {
                    rbset.bindings->SetBufferArray(res.binding, bufferInfos.Count(), bufferInfos);
                    bound = true;
                }
                else
                {
                    DKLogE("ERROR: Failed to bind buffer for resource::%ls",
                           (const wchar_t*)res.name);
                }         
            }
        }
        else if (rb.resource.type == DKShaderResource::TypeTexture)
        {
            if (size_t numBound = bindTextureArray(res, resourceBinder, rbset.bindings); numBound > 0)
                bound = true;
            else
            {
                DKLogE("ERROR: Failed to bind texture for resource::%ls",
                       (const wchar_t*)res.name);
            }
        }
        else if (rb.resource.type == DKShaderResource::TypeSampler)
        {
            if (size_t numBound = bindSamplerArray(res, resourceBinder, rbset.bindings); numBound > 0)
                bound = true;
            else
            {
                DKLogE("ERROR: Failed to bind sampler for resource::%ls",
                       (const wchar_t*)res.name);
            }
        }
        else if (rb.resource.type == DKShaderResource::TypeTextureSampler)
        {
            size_t numTextureBound = bindTextureArray(res, resourceBinder, rbset.bindings);
            size_t numSamplerBound = bindSamplerArray(res, resourceBinder, rbset.bindings);

            if (numTextureBound > 0 && numSamplerBound > 0)
                bound = true;
            else
            {
                DKLogE("ERROR: Failed to bind texture/sampler for resource::%ls",
                       (const wchar_t*)res.name);
            }
        }

        if (bound)
            numBounds++;
        else
            numErrors++;
    }
    return numErrors == 0;
}

DKObject<DKMaterial::ResourceBinder> DKMaterial::SceneResourceBinder(DKSceneState* scene, ResourceBinder* binder)
{
    struct ProxyBinder : public ResourceBinder
    {
        DKMaterial* material;
        ResourceBinder* customBinder;

        ProxyBinder(DKMaterial* m, ResourceBinder* b) :
            material(m), customBinder(b)
        {
            DKASSERT_DEBUG(material);
        }
        BufferArray BufferResource(const DKShaderResource& res) override
        {
            if (customBinder)
            {
                if (auto v = customBinder->BufferResource(res); v.Count() > 0)
                    return v;
            }
            if (auto p = material->bufferResourceProperties.Find({ res.set, res.binding });
                p && p->value.Count() > 0)
            {
                return p->value;
            }
            if (auto p = material->bufferProperties.Find(res.name);
                p && p->value.Count() > 0)
            {
                return p->value;
            }
            return {};
        }
        TextureArray TextureResource(const DKShaderResource& res) override
        {
            if (customBinder)
            {
                if (auto v = customBinder->TextureResource(res); v.Count() > 0)
                    return v;
            }
            if (auto p = material->textureResourceProperties.Find({ res.set, res.binding });
                p && p->value.Count() > 0)
            {
                return p->value;
            }
            if (auto p = material->textureProperties.Find(res.name);
                p && p->value.Count() > 0)
            {
                return p->value;
            }
            return {};
        }
        SamplerArray SamplerResource(const DKShaderResource& res) override
        {
            if (customBinder)
            {
                if (auto v = customBinder->SamplerResource(res); v.Count() > 0)
                    return v;
            }
            if (auto p = material->samplerResourceProperties.Find({ res.set, res.binding });
                p && p->value.Count() > 0)
            {
                return p->value;
            }
            if (auto p = material->samplerProperties.Find(res.name);
                p && p->value.Count() > 0)
            {
                return p->value;
            }
            return {};
        }
        bool WriteStructElement(const DKString& keyPath,
                                const DKShaderResourceStructMember& element,
                                const DKShaderResource& resource,
                                uint32_t resourceArrayIndex,
                                BufferWriter* writer) override
        {
            if (customBinder)
            {
                if (customBinder->WriteStructElement(keyPath,
                                                     element,
                                                     resource,
                                                     resourceArrayIndex,
                                                     writer))
                    return true;

            }
            if (auto p = material->structElementProperties.Find(keyPath); p)
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
    };
    DKObject<ProxyBinder> bp = DKOBJECT_NEW ProxyBinder(this, binder);
    return bp.SafeCast<ResourceBinder>();
}
