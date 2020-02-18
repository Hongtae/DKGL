//
//  File: DKMaterial.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKShader.h"
#include "DKShaderModule.h"
#include "DKShaderBindingSet.h"

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

    class DKMaterial : public DKResource
    {
    public:
        DKMaterial();
        ~DKMaterial();

        struct ShaderTemplate
        {
            DKObject<DKShader> shader;
            DKObject<DKShaderModule> shaderModule;

            struct FunctionProperty
            {
                DKArray<DKShaderBindingSetLayout> bindingSetLayouts;
                DKArray<DKShaderAttribute> inputAttributes;
            };

            DKMap<DKString, FunctionProperty> functionProperties;
            DKMap<DKString, DKShaderUniform> predefinedUniformValues;
        };

        DKObject<ShaderTemplate> shaderTemplate;
        DKObject<DKShaderFunction> shaderFunction;

        using TextureArray = DKArray<DKObject<DKTexture>>;
        using BufferArray = DKArray<DKObject<DKGpuBuffer>>;
        using SamplerArray = DKArray<DKObject<DKSamplerState>>;

        DKMap<uint32_t, TextureArray> textureParameter;
        DKMap<uint32_t, BufferArray> bufferParameter;
        DKMap<uint32_t, SamplerArray> samplerParameter;
        DKMap<DKString, uint32_t> parameterNameIndex;

    private:

    };
}
