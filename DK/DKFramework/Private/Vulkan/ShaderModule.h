//
//  File: ShaderModule.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKShaderModule.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework::Private::Vulkan
{
	class ShaderModule : public DKShaderModule
	{
	public:
		ShaderModule(DKGraphicsDevice*, VkShaderModule, const DKShader*);
		~ShaderModule();

		DKObject<DKShaderFunction> CreateFunction(const DKString& name) const override;
		DKObject<DKShaderFunction> CreateSpecializedFunction(const DKString& name, const DKShaderSpecialization* values, size_t numValues) const override;
		const DKArray<DKString>& FunctionNames() const override { return functionNames; }

		DKGraphicsDevice* Device() override { return device; }

		DKArray<DKString> functionNames;
		DKObject<DKGraphicsDevice> device;
		VkShaderModule module;
		VkShaderStageFlagBits stage;

        DKArray<DKShaderAttribute> inputAttributes;
        DKArray<DKShaderPushConstantLayout> pushConstantLayouts;
        DKArray<DKShaderResource> resources;
        DKArray<DKShader::Descriptor> descriptors;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
