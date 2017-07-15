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
			struct DescriptorSetLayout
			{
				struct Binding
				{
					DKStringU8 name;
					uint32_t index;
					uint32_t count;
				};
				DKArray<Binding> bindings; // bindings with descriptor-set (starting with set=0)
			};
			struct PushConstantLayout
			{
				DKStringU8 name;
				struct Member
				{
					DKStringU8 name;
					uint32_t offset;
					uint32_t size;
				};
				DKArray<Member> members;
			};

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
				// descriptor set bindings
				// descriptor set index starting with zero and should be continuous even if set is empty.
				DKArray<DescriptorSetLayout> layouts[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
				PushConstantLayout pushConstantLayout;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_VULKAN
