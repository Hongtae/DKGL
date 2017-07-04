//
//  File: ShaderModule.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>

#include "../../DKShaderModule.h"
#include "../../DKGraphicsDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Vulkan
		{
			class ShaderModule : public DKShaderModule
			{
			public:
				ShaderModule(DKGraphicsDevice*, VkShaderModule, const void* data, size_t, DKShader::StageType);
				~ShaderModule(void);

				DKObject<DKShaderFunction> CreateFunction(const DKString& name) const override;
				DKObject<DKShaderFunction> CreateSpecializedFunction(const DKString& name, const DKShaderSpecialization* values, size_t numValues) const override;
				const DKArray<DKString>& FunctionNames(void) const override { return functionNames; }

				DKGraphicsDevice* Device(void) override { return device; }

				DKArray<DKString> functionNames;
				DKObject<DKGraphicsDevice> device;
				VkShaderModule module;
				VkShaderStageFlagBits stage;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_VULKAN
