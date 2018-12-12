//
//  File: DKShader.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKShaderResource.h"

namespace DKFramework
{
    struct DKShaderAttribute
    {
        DKString name;
        uint32_t location;
        DKShaderDataType type;
        bool enabled;
    };

	/// @brief shader object
	class DKGL_API DKShader : public DKResource
	{
	public:
        enum DescriptorType
        {
            DescriptorTypeUniformBuffer,
            DescriptorTypeStorageBuffer,
            DescriptorTypeStorageTexture,
            DescriptorTypeUniformTexelBuffer, // readonly texture 'buffer'
            DescriptorTypeStorageTexelBuffer, // writable texture 'buffer'
            DescriptorTypeTextureSampler, // texture, sampler combined
            DescriptorTypeTexture,
            DescriptorTypeSampler,
        };
        struct Descriptor
        {
            uint32_t set;
            uint32_t binding;
            uint32_t count; // array size
            DescriptorType type;
        };

		DKShader();
		DKShader(const DKData*); // share data
		~DKShader();

        DKShaderStage Stage() const     { return stage; }

        const DKData* Data() const  { return data; }
        DKData* Data()              { return data; }

        bool Rebuild(const DKData*);
        bool Validate() override    { return stage != DKShaderStage::Unknown && data; }

        // entry point functions
        const DKArray<DKString>& FunctionNames() const { return functions; }

        // shader reflection 
        const DKArray<DKShaderAttribute>& InputAttributes() const { return inputs; }
        const DKArray<DKShaderAttribute>& OutputAttributes() const { return outputs; }

        const DKArray<DKShaderResource>& Resources() const { return resources; }

        const DKArray<Descriptor>& Descriptors() const { return descriptors; }
        const DKArray<DKShaderPushConstantLayout>& PushConstantBufferLayouts() const { return pushConstantLayouts; }

        // compute-shader threadgroup
        auto ThreadgroupSize() const { return threadgroupSize; }

    private:
		DKShaderStage stage;
		DKObject<DKData> data;

        DKArray<DKString> functions;

        DKArray<DKShaderAttribute> inputs;
        DKArray<DKShaderAttribute> outputs;
        DKArray<DKShaderResource> resources;

        DKArray<DKShaderPushConstantLayout> pushConstantLayouts;

        // for descriptor set layout bindings
        DKArray<Descriptor> descriptors;

        struct { uint32_t x, y, z; } threadgroupSize;
    };
}
