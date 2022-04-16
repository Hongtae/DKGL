//
//  File: DKMaterial.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "DKMaterial.h"
#include "DKGraphicsDevice.h"

namespace DKFramework::Private
{
    inline DKMaterial::ResourceBinder::BufferWriter
        CreateBufferWriter(uint8_t* buffer, size_t bufferLength, size_t offset,
                           bool allowPartialWrites = true)
    {
        if (allowPartialWrites)
        {
            // write only as much as the buffer capacity.
            return [=](const void* data, size_t length)->size_t
            {
                if (bufferLength > offset && length > 0)
                {
                    size_t s = Min(bufferLength - offset, length);
                    memcpy(&buffer[offset], data, s);
                    return s;
                }
                return 0;
            };
        }
        // buffer must have enough space!
        return [=](const void* data, size_t length)->size_t
        {
            if (length + offset <= bufferLength && length > 0)
            {
                memcpy(&buffer[offset], data, length);
                return length;
            }
            return 0;
        };
    }
    struct ShaderResourceStructMemberBinder
    {
        const DKShaderResourceStructMember& member;
        DKString parentKeyPath;
        uint32_t arrayIndex;
        uint32_t offset;
        uint8_t* buffer;
        size_t bufferLength;

        size_t Bind(DKMaterial::ResourceBinder* binder, bool recursive=true)
        {
            const DKString keyPath = DKString(parentKeyPath).Append(".").Append(member.name);
            const DKShaderDataType type = member.dataType;
            const uint32_t memberOffset = member.offset + offset;

            if (memberOffset >= bufferLength)
                return 0;   // Insufficient buffer!

            DKMaterial::ResourceBinder::BufferWriter
                bufferWriter = CreateBufferWriter(buffer, bufferLength, memberOffset);

            // find resource with keyPath
            bool bound = binder->WriteStructElement(keyPath,
                                                    member,
                                                    arrayIndex,
                                                    bufferWriter);

            size_t memberBounds = 0;
            if (!bound || recursive)
            {
                for (const DKShaderResourceStructMember& m : member.members)
                {
                    memberBounds += ShaderResourceStructMemberBinder
                    {
                        m,
                        keyPath,
                        arrayIndex,
                        offset + m.offset,
                        buffer,
                        bufferLength
                    }.Bind(binder, recursive);
                }
            }
            if (bound)
                return DKShaderDataTypeSize(type).Bytes();
            return memberBounds;
        }
    };
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKMaterial::DKMaterial()
    : depthStencilAttachmentPixelFormat(DKPixelFormat::Invalid)
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
                if (auto p2 = p->value.inputAttributeTypes.Find(attr.name); p2)
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
            if (auto p2 = p->value.inputAttributeTypes.Find(attr.name); p2)
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

    renderPipelineDescriptor.colorAttachments = this->colorAttachments;
    renderPipelineDescriptor.depthStencilAttachmentPixelFormat = this->depthStencilAttachmentPixelFormat;
    renderPipelineDescriptor.depthStencilDescriptor = this->depthStencilDescriptor;

    renderPipelineDescriptor.primitiveTopology = DKPrimitiveType::Triangle;
    renderPipelineDescriptor.frontFace = DKFrontFace::CCW;
    renderPipelineDescriptor.triangleFillMode = this->triangleFillMode;
    renderPipelineDescriptor.depthClipMode = this->depthClipMode;
    renderPipelineDescriptor.cullMode = DKCullMode::Back;
    renderPipelineDescriptor.rasterizationEnabled = true;

    return renderPipelineDescriptor;
}

bool DKMaterial::BindResource(ResourceBindingSet& rbset, DKSceneState* scene, ResourceBinder* resourceBinder)
{
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
            if (BufferArray buffers = binder->BufferResource(res); buffers.Count() > 0)
            {
                DKASSERT_DEBUG(res.set == rbset.resourceIndex);
                size_t validBufferCount = Min(buffers.Count(), res.count);

                DKArray<DKShaderBindingSet::BufferInfo> bufferInfos;
                bufferInfos.Reserve(validBufferCount);

                for (uint32_t index = 0; index < validBufferCount; index++)
                {
                    BufferInfo& bi = buffers.Value(index);

                    if (bi.buffer &&
                        bi.offset + bi.length <= bi.buffer->Length())
                    {
                        size_t resourceSize = res.typeInfo.buffer.size;

                        if (uint8_t* ptr = reinterpret_cast<uint8_t*>(bi.buffer->Contents());
                            ptr)
                        {
                            bound = binder->WriteStruct(res.name,
                                                             res.typeInfo.buffer.size,
                                                             index,
                                                             CreateBufferWriter(&ptr[bi.offset], bi.length, 0));
                                
                            size_t memberBounds = 0;
                            for (const DKShaderResourceStructMember& member : res.members)
                            {
                                size_t s = ShaderResourceStructMemberBinder
                                {
                                    member,
                                    res.name,
                                    index,
                                    0, // offset
                                    &ptr[bi.offset], // buffer
                                    bi.length
                                }.Bind(binder, true);

                                memberBounds += s;
                                if (s == 0 && !bound)
                                {
                                    const DKString keyPath = DKString(res.name).Append(".").Append(member.name);
                                    DKLogE("ERROR: Cannot bind struct resource:%ls", (const wchar_t*)keyPath);
                                }
                            }
                            if (bound || memberBounds > 0)
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
            if (size_t numBound = bindTextureArray(res, binder, rbset.bindings); numBound > 0)
                bound = true;
            else
            {
                DKLogE("ERROR: Failed to bind texture for resource::%ls",
                       (const wchar_t*)res.name);
            }
        }
        else if (rb.resource.type == DKShaderResource::TypeSampler)
        {
            if (size_t numBound = bindSamplerArray(res, binder, rbset.bindings); numBound > 0)
                bound = true;
            else
            {
                DKLogE("ERROR: Failed to bind sampler for resource::%ls",
                       (const wchar_t*)res.name);
            }
        }
        else if (rb.resource.type == DKShaderResource::TypeTextureSampler)
        {
            size_t numTextureBound = bindTextureArray(res, binder, rbset.bindings);
            size_t numSamplerBound = bindSamplerArray(res, binder, rbset.bindings);

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

bool DKMaterial::BindResource(PushConstantData& pc, DKSceneState* scene, ResourceBinder* resourceBinder)
{
    DKObject<ResourceBinder> binder = this->SceneResourceBinder(scene, resourceBinder);
    if (binder == nullptr)
        binder = resourceBinder;
    if (binder == nullptr)
    {
        DKLogE("Cannot access resource binder for material!");
        return false;
    }

    uint32_t structSize = pc.layout.offset + pc.layout.size;
    for (const DKShaderResourceStructMember& member : pc.layout.members)
    {
        structSize = Max(structSize, member.offset + member.size);
    }
    pc.data.Resize(structSize);

    bool bound = binder->WriteStruct(pc.layout.name,
                                     structSize,
                                     0, // array-index
                                     CreateBufferWriter(pc.data,
                                                        structSize,
                                                        0));

    size_t memberBounds = 0;
    for (const DKShaderResourceStructMember& member : pc.layout.members)
    {
        if (member.offset < pc.layout.offset ||
            member.offset >= pc.layout.offset + pc.layout.size)
            continue;

        size_t s = ShaderResourceStructMemberBinder
        {
            member,
            pc.layout.name,
            0, // array index
            0, // offset
            pc.data,
            structSize
        }.Bind(binder, true);

        memberBounds += s;
        if (s == 0 && !bound)
        {
            const DKString keyPath = DKString(pc.layout.name).Append(".").Append(member.name);
            DKLogE("ERROR: Cannot bind struct resource:%ls", (const wchar_t*)keyPath);
        }
    }
    if (memberBounds == pc.layout.size)
        bound = true;
    return bound;
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
                                uint32_t resourceArrayIndex,
                                const BufferWriter& writer) override
        {
            if (customBinder)
            {
                if (customBinder->WriteStructElement(keyPath,
                                                     element,
                                                     resourceArrayIndex,
                                                     writer))
                    return true;

            }
            if (auto p = material->structProperties.Find(keyPath); p)
            {
                DKASSERT_DEBUG(element.size > 0);

                size_t elementSize = size_t(element.size);
                size_t offset = elementSize * resourceArrayIndex;

                StructProperty& prop = p->value;

                if (prop.data.Count() > offset)
                {
                    const uint8_t* ptr = prop.data;
                    size_t size = Min(prop.data.Count() - offset, elementSize);
                    if (writer->Invoke(&ptr[offset], size) == size)
                        return true;
                }
            }
            return false;
        }
        bool WriteStruct(const DKString& keyPath,
                         uint32_t structSize,
                         uint32_t arrayIndex,
                         const BufferWriter& writer) override
        {
            if (customBinder)
            {
                if (customBinder->WriteStruct(keyPath,
                                              structSize,
                                              arrayIndex,
                                              writer))
                    return true;

            }
            if (auto p = material->structProperties.Find(keyPath); p)
            {
                DKASSERT_DEBUG(structSize > 0);

                size_t offset = size_t(structSize) * arrayIndex;

                StructProperty& prop = p->value;

                if (prop.data.Count() > offset)
                {
                    const uint8_t* ptr = prop.data;
                    size_t size = Min(prop.data.Count() - offset, structSize);
                    if (writer->Invoke(&ptr[offset], size) == size)
                        return true;
                }
            }
            return false;
        }
    };
    DKObject<ProxyBinder> bp = DKOBJECT_NEW ProxyBinder(this, binder);
    return bp.SafeCast<ResourceBinder>();
}
