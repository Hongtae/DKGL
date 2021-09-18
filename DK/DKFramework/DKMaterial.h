//
//  File: DKMaterial.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKRenderPipeline.h"
#include "DKShader.h"
#include "DKShaderModule.h"
#include "DKShaderBindingSet.h"
#include "DKRenderCommandEncoder.h"

namespace DKFramework
{
    /// predefined shader uniform constant identifiers
    enum class DKShaderUniform : uint32_t
    {
        Unknown = 0,                        ///< error
        ModelMatrix,                        ///< float4x4
        ModelMatrixInverse,                 ///< float4x4
        ViewMatrix,                         ///< float4x4
        ViewMatrixInverse,                  ///< float4x4
        ProjectionMatrix,                   ///< float4x4
        ProjectionMatrixInverse,            ///< float4x4
        ViewProjectionMatrix,               ///< float4x4
        ViewProjectionMatrixInverse,        ///< float4x4
        ModelViewMatrix,                    ///< float4x4
        ModelViewMatrixInverse,             ///< float4x4
        ModelViewProjectionMatrix,          ///< float4x4
        ModelViewProjectionMatrixInverse,   ///< float4x4
        LinearTransformArray,               ///< float3x3
        AffineTransformArray,               ///< float4x4
        PositionArray,                      ///< float3
        Texture2D,                          ///< uint1
        Texture3D,                          ///< uint1
        TextureCube,                        ///< uint1
        AmbientColor,                       ///< float3
        CameraPosition,                     ///< float3
        UserDefine,                         ///< user define
    };
    /// predefined vertex stream identifiers
    enum class DKVertexStream : uint32_t
    {
        Unknown = 0,		// regard to error
        Position,
        Normal,
        Color,
        TexCoord,
        Tangent,
        Bitangent,
        BlendIndices,
        BlendWeights,
        UserDefine,		// user-define (you can access by name, at shader)
    };

    class DKSceneState;

    class DKGL_API DKMaterial : public DKResource
    {
    public:
        DKMaterial();
        ~DKMaterial();

        // shader parameter info, with pre-defined constants
        struct ShaderTemplate
        {
            DKObject<DKShader> shader;
            DKObject<DKShaderFunction> shaderFunction;

            // predefined resources, attributes.
            DKMap<DKString, DKShaderUniform> resourceTypes;
            DKMap<DKString, DKVertexStream> inputAttributeTypes;
        };
        DKMap<DKShaderStage, ShaderTemplate> shaderTemplates;

        union ResourceIndex
        {
            struct
            {
                uint32_t set;
                uint32_t binding;
            };
            uint64_t value;
            operator uint64_t () const noexcept {return value;} 
        };
        DKMap<ResourceIndex, DKString> resourceIndexNameMap;

        struct BufferInfo
        {
            DKObject<DKGpuBuffer> buffer;
            uint64_t offset;
            uint64_t length;
        };
        // default shader binding objects (parameters)
        using TextureArray = DKArray<DKObject<DKTexture>>;
        using BufferArray = DKArray<BufferInfo>;
        using SamplerArray = DKArray<DKObject<DKSamplerState>>;

        // material resource properties
        DKMap<ResourceIndex, BufferArray> bufferResourceProperties;
        DKMap<ResourceIndex, TextureArray> textureResourceProperties;
        DKMap<ResourceIndex, SamplerArray> samplerResourceProperties;

        // named material properties
        DKMap<DKString, BufferArray> bufferProperties;
        DKMap<DKString, TextureArray> textureProperties;
        DKMap<DKString, SamplerArray> samplerProperties;

        struct StructProperty
        {
            DKArray<uint8_t> data; // typeSize.Bytes() * arrayLength
            template <typename Scalar, int Num>
            void Set(const Scalar(&val)[Num])
            {
                data.Clear();
                data.Add(reinterpret_cast<const uint8_t*>(&val[0]),
                         sizeof(Scalar[Num]));
            }
            template <typename Scalar, int Num>
            void Add(const Scalar(&val)[Num])
            {
                data.Add(reinterpret_cast<const uint8_t*>(&val[0]),
                         sizeof(Scalar[Num]));
            }
            void Set(const void* value, size_t size)
            {
                data.Clear();
                data.Add(reinterpret_cast<const uint8_t*>(value), size);
            }
            void Add(const void* value, size_t size)
            {
                data.Add(reinterpret_cast<const uint8_t*>(value), size);
            }
        };
        DKMap<DKString, StructProperty> structProperties;

        DKRenderPipelineDescriptor RenderPipelineDescriptor() const;
        DKShaderUniform ShaderUniformForResource(ResourceIndex, uint32_t) const;
        DKShaderUniform ShaderUniformForResource(const DKString&, uint32_t) const;

        bool FindShaderDescriptorForResource(ResourceIndex, uint32_t, DKShader::Descriptor&) const;

        bool FindStageInputAttribute(DKShaderStage, DKVertexStream, const DKString&, DKShaderAttribute&) const;
        bool FindStageInputAttribute(DKShaderStage, DKVertexStream, DKShaderAttribute&) const;
        bool FindStageInputAttribute(DKShaderStage, const DKString&, DKShaderAttribute&) const;


        struct ResourceBinder
        {
            using BufferWriter = DKFunctionSignature<size_t (const void* data, size_t length)>;

            virtual ~ResourceBinder() {}

            // bind resource directly.
            virtual BufferArray BufferResource(const DKShaderResource&) = 0;
            virtual TextureArray TextureResource(const DKShaderResource&) = 0;
            virtual SamplerArray SamplerResource(const DKShaderResource&) = 0;

            // bind struct element separately.
            virtual bool WriteStructElement(const DKString& keyPath,
                                            const DKShaderResourceStructMember& element,
                                            uint32_t arrayIndex,
                                            BufferWriter*) = 0;
            virtual bool WriteStruct(const DKString& keyPath,
                                     uint32_t structSize, // (offset + size)
                                     uint32_t arrayIndex,
                                     BufferWriter*) = 0;
        };

        struct ResourceBinding
        {
            DKShaderResource resource;  // from spirv-data
            DKShaderBinding binding;    // from descriptor-set layout
        };
        struct ResourceBindingSet
        {
            uint32_t resourceIndex; // BindingSet(descriptorSet) index
            DKObject<DKShaderBindingSet> bindings;
            DKArray<ResourceBinding> resources;
        };
        struct PushConstantData
        {
            DKShaderPushConstantLayout layout;
            DKArray<uint8_t> data;
        };

        bool BindResource(ResourceBindingSet&, DKSceneState*, ResourceBinder*);
        bool BindResource(PushConstantData&, DKSceneState*,ResourceBinder*);

        // rasterazation properties...
        DKArray<DKRenderPipelineColorAttachmentDescriptor> colorAttachments;
        DKPixelFormat depthStencilAttachmentPixelFormat;
        DKDepthStencilDescriptor depthStencilDescriptor;

        DKTriangleFillMode triangleFillMode = DKTriangleFillMode::Fill;
        DKDepthClipMode depthClipMode = DKDepthClipMode::Clip;

    private:
        DKObject<ResourceBinder> SceneResourceBinder(DKSceneState*, ResourceBinder*);

    };
}
