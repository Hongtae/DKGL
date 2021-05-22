//
//  File: ShaderFunction.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>
#include "../../DKShaderModule.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Metal
{
    struct ResourceBinding
    {
        uint32_t set;
        uint32_t binding;

        uint32_t bufferIndex;
        uint32_t textureIndex;
        uint32_t samplerIndex;

        DKShaderResource::Type type;
    };

    struct StageResourceBindingMap
    {
        DKArray<ResourceBinding> resourceBindings; // spir-v to msl binding (mapping)
        uint32_t inputAttributeIndexOffset;
        uint32_t pushConstantIndex;
        uint32_t pushConstantOffset;
    };

	class ShaderModule : public DKShaderModule
	{
	public:
        struct NameConversion
        {
            DKString originalName;
            DKString cleansedName;
        };
		ShaderModule(DKGraphicsDevice*, id<MTLLibrary>, const DKArray<NameConversion>& functionNames);
		~ShaderModule();

		const DKArray<DKString>& FunctionNames() const override { return functionNames; }

		DKObject<DKShaderFunction> CreateFunction(const DKString& name) const override;
		DKObject<DKShaderFunction> CreateSpecializedFunction(const DKString& name, const DKShaderSpecialization* values, size_t numValues) const override;

		DKGraphicsDevice* Device() override { return device; }

		id<MTLLibrary> library;
		DKObject<DKGraphicsDevice> device;
		DKArray<DKString> functionNames; // name from spirv
        DKMap<DKString, DKString> functionNameMap; // spirv to msl table

        MTLSize workgroupSize;

        StageResourceBindingMap bindings;
	};
}
#endif //#if DKGL_ENABLE_METAL
